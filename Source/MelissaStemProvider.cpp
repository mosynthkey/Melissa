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

#ifdef MELISSA_USE_DEMUCS
#include "model.hpp"
#include "Eigen/Dense"
#endif

using namespace juce;

// Define temporary directory name for resampling
static const String kTempDirectoryName = "temp_stems_processing";

MelissaStemProvider MelissaStemProvider::instance_;

static const String stemFileName = "stem_info.json";

MelissaStemProvider::MelissaStemProvider() : Thread("MelissaStemProviderProcessThread"),
status_(kStemProviderStatus_Ready),
result_(kStemProviderResult_UnknownError)
{
    
}

MelissaStemProvider::~MelissaStemProvider()
{
    
}

bool MelissaStemProvider::requestStems(const File& file, SeparatorType type)
{
    if (status_ == kStemProviderStatus_Processing || isThreadRunning()) return false;
    
    songFile_ = file;
    separatorType_ = type;
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
    const auto currentSongDirectory = songFile_.getParentDirectory();
    const auto songName = File::createLegalFileName(songFile_.getFileName());
    
    // Create output directory
    File outputDirName(currentSongDirectory.getChildFile(songName + "_stems"));
    if (outputDirName.createDirectory().failed()) return kStemProviderResult_FailedToReadSourceFile;
    
    if (separatorType_ == kSeparatorType_Spleeter)
    {
#ifdef MELISSA_USE_SPLEETER
        // validate the parameters (output count)
        std::error_code err;
        
        // Initialize spleeter
        auto settingsDir = (File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Melissa"));
        auto model_path = settingsDir.getChildFile("models").getFullPathName().toStdString();

        const auto separation_type = spleeter::FiveStems;
        const clock_t startTime = clock();

        spleeter::Initialize(model_path, {separation_type}, err);
        if (err)
            return kStemProviderResult_FailedToInitialize;

        InputFile input(songFile_.getFullPathName().toStdString());
        input.Open(err);
        if (err)
            return kStemProviderResult_FailedToReadSourceFile;

        auto sampleRate = MelissaDataSource::getInstance()->getSampleRate();
        auto bufferLength = MelissaDataSource::getInstance()->getBufferLength();
        OutputFolder output_folder(outputDirName.getFullPathName().toStdString(), songName.toStdString(), sampleRate, static_cast<int>(bufferLength));

        while (true)
        {
            if (threadShouldExit())
                return kStemProviderResult_Interrupted;

            auto data = input.Read();
            if (data.cols() == 0)
                break;

            auto result = Split(data, separation_type, err);
            if (err)
                return kStemProviderResult_FailedToSplit;

            output_folder.Write(result, err);
            if (err)
                return kStemProviderResult_FailedToExport;

            float estimatedTime = (clock() - startTime) / input.getProgress() * 1.15;

            MessageManager::callAsync([&]()
                                      {
                        for (auto& l : listeners_) l->stemProviderEstimatedTimeReported(estimatedTime); });
        }

        output_folder.Flush();
        return kStemProviderResult_Success;
#else
        return kStemProviderResult_NotSupported;
#endif
    }
    else if (separatorType_ == kSeparatorType_Demucs)
    {
#ifdef MELISSA_USE_DEMUCS
        try
        {
            // Check if we need to resample
            AudioFormatManager formatManager;
            formatManager.registerBasicFormats();
            
            File sourceFile = songFile_;
            File tempFile;
            
            // Create a temporary directory for resampling if needed
            auto tempDir = File::getSpecialLocation(File::tempDirectory).getChildFile(kTempDirectoryName);
            tempDir.createDirectory();
            
            // Read the audio file and check sample rate
            auto reader = std::unique_ptr<AudioFormatReader>(formatManager.createReaderFor(sourceFile));
            if (!reader) return kStemProviderResult_FailedToReadSourceFile;
            
            // Check if resampling is needed (Demucs requires 44.1kHz)
            bool needsResampling = reader->sampleRate != 44100.0;
            
            if (needsResampling)
            {
                // Create a temporary file for resampled audio
                tempFile = tempDir.getChildFile(songName + "_resampled.wav");
                
                // Read the entire audio file
                const int numSamples = static_cast<int>(reader->lengthInSamples);
                const int numChannels = reader->numChannels > 2 ? 2 : static_cast<int>(reader->numChannels); // Use at most 2 channels
                
                AudioBuffer<float> buffer(numChannels, numSamples);
                reader->read(&buffer, 0, numSamples, 0, true, true);
                
                // Create a resampled output file
                WavAudioFormat wavFormat;
                
                std::unique_ptr<AudioFormatWriter> writer(
                    wavFormat.createWriterFor(new FileOutputStream(tempFile), 44100.0, numChannels, 16, {}, 0));
                
                if (writer == nullptr) return kStemProviderResult_FailedToReadSourceFile;
                
                // Handle resampling manually
                writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
                
                // Use the resampled file for processing
                sourceFile = tempFile;
                
                // Re-open the reader for the resampled file
                reader = std::unique_ptr<AudioFormatReader>(formatManager.createReaderFor(sourceFile));
                if (!reader) return kStemProviderResult_FailedToReadSourceFile;
            }
            
            // Load the Demucs model
            auto settingsDir = (File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Melissa"));
            auto modelPath = settingsDir.getChildFile("demucs_models").getChildFile("ggml-model-htdemucs-6s-f16.bin").getFullPathName();
            
            auto model = std::make_unique<demucscpp::demucs_model>();
            if (!demucscpp::load_demucs_model(modelPath.toStdString(), model.get()))
            {
                return kStemProviderResult_FailedToInitialize;
            }
            
            // Process audio with Demucs
            const int numSamples = static_cast<int>(reader->lengthInSamples);
            const int numChannels = reader->numChannels > 2 ? 2 : static_cast<int>(reader->numChannels); // Use at most 2 channels
            
            // Read audio data
            AudioBuffer<float> buffer(numChannels, numSamples);
            reader->read(&buffer, 0, numSamples, 0, true, true);
            
            // Convert to Eigen matrix format for Demucs
            Eigen::MatrixXf audioData(numChannels, numSamples);
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* channelData = buffer.getReadPointer(ch);
                for (int i = 0; i < numSamples; ++i)
                {
                    audioData(ch, i) = channelData[i];
                }
            }
            
            // Run Demucs inference
            auto out_targets = demucscpp::demucs_inference(*model, audioData, 
                [this](float progress, const std::string& message) {
                    if (threadShouldExit())
                    {
                        throw std::runtime_error("Processing cancelled by user");
                    }
                    
                    // Report progress percentage for Demucs
                    MessageManager::callAsync([this, progress, message]() {
                        // Convert progress from 0-1 to 0-100%
                        float progressPercentage = progress * 100.0f;
                        juce::String progressMessage = juce::String(message);
                        
                        // Report both percentage and message
                        for (auto& l : listeners_) l->stemProviderProgressReported(progressPercentage, progressMessage);
                    });
                });
            
            if (threadShouldExit()) return kStemProviderResult_Interrupted;
            
            // Save separated tracks
            OggVorbisAudioFormat oggFormat;
            
            // Map Demucs output targets to our part names
            // Demucs output order: drums, bass, other, vocals
            const static std::vector<std::string> demucsToPartNames = {
                "drums", "bass", "other", "vocals", "guitar", "piano"
            };

            // Get original sample rate
            auto originalSampleRate = MelissaDataSource::getInstance()->getSampleRate();

            for (int target = 0; target < demucsToPartNames.size(); ++target)
            {
                if (threadShouldExit()) return kStemProviderResult_Interrupted;
                
                auto outputFile = outputDirName.getChildFile(songName + "_" + demucsToPartNames[target] + ".ogg");
                
                AudioBuffer<float> outBuffer(numChannels, numSamples);
                
                // Get index in Demucs output (drums=0, bass=1, other=2, vocals=3)
                int demucsTargetIndex = target;
                if (target >= 4) demucsTargetIndex = 2; // Map piano to other
                
                // Copy data from Demucs output to AudioBuffer
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float* channelData = outBuffer.getWritePointer(ch);
                    for (int i = 0; i < numSamples; ++i)
                    {
                        channelData[i] = out_targets(demucsTargetIndex, ch, i);
                    }
                }
                
                // Save current part
                std::unique_ptr<AudioFormatWriter> writer(
                    oggFormat.createWriterFor(new FileOutputStream(outputFile), originalSampleRate, numChannels, 16, {}, 0));
                
                if (writer)
                {
                    writer->writeFromAudioSampleBuffer(outBuffer, 0, numSamples);
                }
                else
                {
                    return kStemProviderResult_FailedToExport;
                }
            }
            
            // Clean up temporary files
            if (needsResampling && tempFile.existsAsFile())
            {
                tempFile.deleteFile();
            }
            
            // Clean up temporary directory
            tempDir.deleteRecursively();
        }
        catch (const std::exception& e)
        {
            return kStemProviderResult_FailedToSplit;
        }
#else
        return kStemProviderResult_NotSupported;
#endif
    }
    else
    {
        return kStemProviderResult_NotSupported;
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
}
