//
//  MelissaStemProvider.cpp
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#include "MelissaStemProvider.h"
#include "MelissaDataSource.h"

#ifdef MELISSA_USE_SPLEETER
#include "spleeter/spleeter.h"
#include "input_file.h"
#include "output_folder.h"
#include "utils.h"
#include "split.h"
#include "nlohmann/json.hpp"
#endif

MelissaStemProvider MelissaStemProvider::instance_;

static const String stemFileName = "stem_info.json";

MelissaStemProvider::MelissaStemProvider() : Thread("MelissaSpleeterProcessThread"),
status_(kStemProviderStatus_Ready),
result_(kStemProviderResult_UnknownError)
{
    
}

MelissaStemProvider::~MelissaStemProvider()
{
    
}

bool MelissaStemProvider::requestStems(const File& file)
{
    if (status_ == kStemProviderStatus_Processing || isThreadRunning()) return false;
    
    songFile_ = file;
    startThread();
    
    return true;
}

void MelissaStemProvider::getStemFiles(const File& fileToOpen, File& originalFile, std::map<std::string, File>& stemFiles)
{

    if (!fileToOpen.existsAsFile()) return;
    originalFile = fileToOpen;
    
#ifdef MELISSA_USE_SPLEETER
    // Result 1. fileToOpen has stems
    // Result 2. fileToOpen doesn't have stems
    // Result 3. fileToOpen is a stem

    using json = nlohmann::json;
    
    auto fileDir = fileToOpen.getParentDirectory();
    auto fileName = File::createLegalFileName(fileToOpen.getFileName());
    File stemDir = fileDir.getChildFile(fileName + "_stems");
    
    if (stemDir.isDirectory() && stemDir.getChildFile(stemFileName).existsAsFile())
    {
        // check "stem_info.json"
        auto stemInfoFile = stemDir.getChildFile(stemFileName);
        auto stemInfoJson = stemInfoFile.loadFileAsString().toStdString();
        
        try
        {
            auto j = json::parse(stemInfoJson);
            if (!(j["original"].get<std::string>() == fileToOpen.getFileName().toStdString()))
            {
                // invalid json file
                // Result 2
                return;
            }
            
            for (auto& partName : partNames_)
            {
                auto& partInfo = j[partName];
                File stemFile = stemDir.getChildFile(partInfo["file_name"].get<std::string>());
                const auto md5 = partInfo["md5"].get<std::string>();
                if (!stemFile.existsAsFile() || MD5(stemFile).toHexString().toStdString() != md5)
                {
                    stemFiles.clear();
                    // Result 2
                    return;
                }
                stemFiles[partName] = stemFile;
            }
            
            status_ = kStemProviderStatus_Available;
            return;
        }
        catch (std::exception& e)
        {
            // Result 2
            return;
        }
    }
    
    if (fileDir.getChildFile(stemFileName).existsAsFile())
    {
        auto stemInfoJson = fileDir.getChildFile(stemFileName).loadFileAsString().toStdString();
        try
        {
            auto j = json::parse(stemInfoJson);
            auto originalFileName = j["original"].get<std::string>();
            getStemFiles(fileDir.getParentDirectory().getChildFile(originalFileName), originalFile, stemFiles);
            // Possibly Result 3?
            return;
        }
        catch (std::exception& e)
        {
            // Result 2
            return;
        }
    }
#else
    return;
#endif
}

void MelissaStemProvider::failedToReadPreparedStems()
{
    stopThread(1000);
    
    status_ = kStemProviderStatus_Ready;
    
    MessageManager::callAsync([&]() {
        for (auto& l : listeners_) l->stemProviderStatusChanged(status_);
    });
}

void MelissaStemProvider::prepareForLoadStems(const File& fileToOpen, File& originalFile, std::map<std::string, File>& stemFiles)
{
    stemFiles.clear();
    getStemFiles(fileToOpen, originalFile, stemFiles);
    if (stemFiles.size() == kNumStemFiles)
    {
        status_ = kStemProviderStatus_Available;
    }
    else
    {
        status_ = kStemProviderStatus_Ready;
    }
    
    MessageManager::callAsync([&]() {
        for (auto& l : listeners_) l->stemProviderStatusChanged(status_);
    });
}

void MelissaStemProvider::deleteStems()
{
    if (!songFile_.existsAsFile()) return;
    
    const auto currentSongDirectory = songFile_.getParentDirectory();
    const auto songName = File::createLegalFileName(songFile_.getFileName());
    
    File outputDirName(currentSongDirectory.getChildFile(songName + "_stems"));
    
    if (outputDirName.isDirectory())
    {
        outputDirName.deleteRecursively();
    }
}

void MelissaStemProvider::addListener(MelissaStemProviderListener* listener)
{
    for (auto&& l : listeners_)
    {
        if (l == listener) return;
    }
    
    listeners_.emplace_back(listener);
}

void MelissaStemProvider::removeListener(MelissaStemProviderListener* listener)
{
    for (size_t listener_i = 0; listener_i < listeners_.size(); ++listener_i)
    {
        if (listeners_[listener_i] == listener)
        {
            listeners_.erase(listeners_.begin() + listener_i);
            return;
        }
    }
}

void MelissaStemProvider::run()
{
    status_ = kStemProviderStatus_Processing;
    MessageManager::callAsync([&]() {
        for (auto& l : listeners_) l->stemProviderStatusChanged(status_);
    });
    
    result_ = createStems();
    MessageManager::callAsync([&]() {
        for (auto& l : listeners_) l->stemProviderResultReported(result_);
    });
    
    if (result_ == kStemProviderResult_Success)
    {
        status_ = kStemProviderStatus_Available;
    }
    else if (result_ == kStemProviderResult_Interrupted)
    {
        deleteStems();
        status_ = kStemProviderStatus_Ready;
    }
    else
    {
        status_ = kStemProviderStatus_NotAvailable;
    }
    MessageManager::callAsync([&]() {
        for (auto& l : listeners_) l->stemProviderStatusChanged(status_);
    });

    if (result_ == kStemProviderResult_Success)
    {
        MessageManager::callAsync([&]() {
            // reload file to load stems
            auto dataSource = MelissaDataSource::getInstance();
            dataSource->loadFileAsync(dataSource->getCurrentSongFilePath());
        });
    }
}

StemProviderResult MelissaStemProvider::createStems()
{
#ifdef MELISSA_USE_SPLEETER
    const auto currentSongDirectory = songFile_.getParentDirectory();
    const auto songName = File::createLegalFileName(songFile_.getFileName());
    
    // validate the parameters (output count)
    std::error_code err;
    constexpr int kNumSeparationType = 2;
    spleeter::SeparationType separation_types[kNumSeparationType] = { spleeter::TwoStems, spleeter::FiveStems };
    clock_t startTimes[kNumSeparationType] = { 0, 0 };
    
    // create output directory
    File outputDirName(currentSongDirectory.getChildFile(songName + "_stems"));
    if (outputDirName.createDirectory().failed()) return kStemProviderResult_FailedToReadSourceFile;
    
    // Initialize spleeter
    auto settingsDir = (File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Melissa"));
    auto model_path = settingsDir.getChildFile("models").getFullPathName().toStdString();
    
    for (int separationTypeIndex = 0; separationTypeIndex < kNumSeparationType; ++separationTypeIndex)
    {
        const auto separation_type = separation_types[separationTypeIndex];
        startTimes[separationTypeIndex] = clock();
        
        if (threadShouldExit()) return kStemProviderResult_Interrupted;
        
        spleeter::Initialize(model_path, {separation_type}, err);
        if (err) return kStemProviderResult_FailedToInitialize;
        
        InputFile input(songFile_.getFullPathName().toStdString());
        input.Open(err);
        if (err) return kStemProviderResult_FailedToReadSourceFile;
        {
            auto sampleRate = MelissaDataSource::getInstance()->getSampleRate();
            auto bufferLength = MelissaDataSource::getInstance()->getBufferLength();
            OutputFolder output_folder(outputDirName.getFullPathName().toStdString(), songName.toStdString(), sampleRate, static_cast<int>(bufferLength));
            
            while (true)
            {
                if (threadShouldExit()) return kStemProviderResult_Interrupted;
                
                auto data = input.Read();
                if (data.cols() == 0) break;
                
                auto result = Split(data, separation_type, err);
                if (err) return kStemProviderResult_FailedToSplit;
                
                output_folder.Write(result, err);
                if (err) return kStemProviderResult_FailedToExport;
                
                float estimatedTime = 0.f;
                if (separation_type ==  spleeter::TwoStems)
                {
                    estimatedTime = (clock() - startTimes[0]) / input.getProgress() * 1.15 * 3.5;
                }
                else
                {
                    estimatedTime = (startTimes[1] - startTimes[0]) + (clock() - startTimes[1]) / input.getProgress() * 1.15;
                }
                
                MessageManager::callAsync([&]() {
                    for (auto& l : listeners_) l->stemProviderEstimatedTimeReported(estimatedTime);
                });
            }
            
            output_folder.Flush();
        }
    }
    
    // create melissa_stems.json
    try
    {
        using json = nlohmann::json;
        json stemSettings;
        stemSettings["original"] = songFile_.getFileName().toStdString();
        for (auto& part : partNames_)
        {
            const auto stemFileName = songName + "_" + part + ".ogg";
            stemSettings[part]["file_name"] = stemFileName.toStdString();
            
            File stemFile(outputDirName.getChildFile(stemFileName));
            if (stemFile.existsAsFile())
            {
                stemSettings[part]["md5"] = MD5(stemFile).toHexString().toStdString();
            }
        }
        
        File stemSettingsFile(outputDirName.getChildFile("stem_info.json"));
        if (stemSettingsFile.existsAsFile())
        {
            stemSettingsFile.deleteFile();
        }
        if (!stemSettingsFile.replaceWithText(stemSettings.dump(4)))
        {
            return kStemProviderResult_FailedToExport;
        }
    }
    catch(std::exception& e)
    {
        return kStemProviderResult_FailedToExport;
    }
    
    status_ = kStemProviderStatus_Available;
    return kStemProviderResult_Success;
#endif
    
    return kStemProviderResult_NotSupported;
}
