//
//  MelissaStemProvider.cpp
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#include "MelissaStemProvider.h"
#include "MelissaDataSource.h"

#ifdef MELISSA_USE_STEM_SEPARATION
#include "spleeter/spleeter.h"
#include "input_file.h"
#include "output_folder.h"
#include "utils.h"
#include "split.h"
#include "nlohmann/json.hpp"
#include "demucs.hpp"
#include "Eigen/Dense"
#include <fstream>
#include <ctime>
#endif

using namespace juce;

// Define temporary directory name for resampling
static const String kTempDirectoryName = "temp_stems_processing";

MelissaStemProvider MelissaStemProvider::instance_;

static const String stemFileName = "stem_info.json";

MelissaStemProvider::MelissaStemProvider() : Thread("MelissaStemProviderProcessThread"),
status_(kStemProviderStatus_Ready),
result_(kStemProviderResult_UnknownError),
separatorType_(kSeparatorType_Spleeter),
audioFileFormat_(kStemOutputAudioFormat_Ogg)
{
    
}

MelissaStemProvider::~MelissaStemProvider()
{
    
}

bool MelissaStemProvider::requestStems(const File& file, SeparatorType type, StemOutputAudioFormat audioFormat)
{
    if (status_ == kStemProviderStatus_Processing || isThreadRunning()) return false;
    
    separatorType_ = type;
    audioFileFormat_ = audioFormat;
    songFile_ = file;
    startThread();
    
    return true;
}

void MelissaStemProvider::getStemFiles(const File& fileToOpen, File& originalFile, std::map<std::string, File>& stemFiles)
{

    if (!fileToOpen.existsAsFile()) return;
    originalFile = fileToOpen;
    
#ifdef MELISSA_USE_STEM_SEPARATION
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
            
            auto checkStemAvailability = [&](const std::string& partName, File& stemFile)
            {
                auto& partInfo = j[partName];
                stemFile = stemDir.getChildFile(partInfo["file_name"].get<std::string>());
                const auto md5 = partInfo["md5"].get<std::string>();
                return (stemFile.existsAsFile() && MD5(stemFile).toHexString().toStdString() == md5);
            };
            
            for (auto& partName : partNames_)
            {
                File stemFile;
                if (partName == "guitar") continue; // guitar is only available with demucs separation, check later
                if (!checkStemAvailability(partName, stemFile))
                {
                    stemFiles.clear();
                    // Result 2
                    return;
                }
                stemFiles[partName] = stemFile;
            }
            
            File guitarStemFile;
            if (checkStemAvailability("guitar", guitarStemFile))
            {
                stemFiles["guitar"] = guitarStemFile;
                status_ = kStemProviderStatus_Available_Full;
            }
            else
            {
                status_ = kStemProviderStatus_Available_NoGuitar;
            }
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
        status_ = kStemProviderStatus_Available_Full;
    }
    else if (stemFiles.size() == (kNumStemFiles - 1))
    {
        status_ = kStemProviderStatus_Available_NoGuitar;
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
    auto songFile = juce::File(MelissaDataSource::getInstance()->getCurrentSongFilePath());
    if (!songFile.existsAsFile()) return;
    
    const auto currentSongDirectory = songFile.getParentDirectory();
    const auto songName = File::createLegalFileName(songFile.getFileName());
    
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
    
    if (result_ == kStemProviderResult_Success_Full)
    {
        status_ = kStemProviderStatus_Available_Full;
    }
    else if (result_ == kStemProviderResult_Success_NoGuitar)
    {
        status_ = kStemProviderStatus_Available_NoGuitar;
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

    if (result_ == kStemProviderResult_Success_Full || result_ == kStemProviderResult_Success_NoGuitar)
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
#ifndef MELISSA_USE_STEM_SEPARATION
    return kStemProviderResult_NotSupported;
#endif
    
    const auto currentSongDirectory = songFile_.getParentDirectory();
    const auto songName = File::createLegalFileName(songFile_.getFileName());
    
    // Create output directory
    File outputDirName(currentSongDirectory.getChildFile(songName + "_stems"));
    if (outputDirName.createDirectory().failed()) return kStemProviderResult_FailedToReadSourceFile;
    
    if (separatorType_ == kSeparatorType_Spleeter)
    {
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
        OutputFolder output_folder(outputDirName.getFullPathName().toStdString(), songName.toStdString(), sampleRate, static_cast<int>(bufferLength), static_cast<StemOutputAudioFormat>(audioFileFormat_));

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
    }
    else if (separatorType_ == kSeparatorType_Demucs)
    {
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
            const auto originalLengthInSamples = reader->lengthInSamples;
            
            if (needsResampling)
            {
                // Create a temporary file for resampled audio
                tempFile = tempDir.getChildFile(songName + "_resampled.wav");
                
                // Read the entire audio file
                const int numSamples = static_cast<int>(reader->lengthInSamples);
                const int numChannels = reader->numChannels > 2 ? 2 : static_cast<int>(reader->numChannels); // Use at most 2 channels
                
                AudioBuffer<float> audioBuffer(numChannels, numSamples);
                reader->read(&audioBuffer, 0, numSamples, 0, true, true);

                MemoryAudioSource audioSource(audioBuffer, false);
                ResamplingAudioSource resamplingAudioSource(&audioSource, false, numChannels);
                constexpr float outputSampleRate = 44100.0;
                resamplingAudioSource.setResamplingRatio(reader->sampleRate / outputSampleRate);
                resamplingAudioSource.prepareToPlay(2048, outputSampleRate);

                WavAudioFormat wavFormat;
                auto writer = std::unique_ptr<AudioFormatWriter>(wavFormat.createWriterFor(new FileOutputStream(tempFile), outputSampleRate, numChannels, 16, StringPairArray(), 0));
                if (writer == nullptr) return kStemProviderResult_FailedToReadSourceFile;
                writer->writeFromAudioSource(resamplingAudioSource, reader->lengthInSamples * outputSampleRate / reader->sampleRate);
                writer->flush();
                
                // Use the resampled file for processing
                sourceFile = tempFile;
                
                // Re-open the reader for the resampled file
                reader = std::unique_ptr<AudioFormatReader>(formatManager.createReaderFor(sourceFile));
                if (!reader) return kStemProviderResult_FailedToReadSourceFile;
            }
            
            // Load the Demucs model
            auto settingsDir = (File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Melissa"));
            auto modelPath = settingsDir.getChildFile("models").getChildFile("demucs").getChildFile("htdemucs_6s.ort").getFullPathName();

            // Create Ort::SessionOptions
            Ort::SessionOptions session_options;
            session_options.SetExecutionMode(ExecutionMode::ORT_PARALLEL);
            session_options.SetIntraOpNumThreads(4);
            session_options.SetInterOpNumThreads(4);
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

            // Create model struct
            demucsonnx::demucs_model model;
            
            // Load model from file
            std::ifstream file(modelPath.toStdString(), std::ios::binary | std::ios::ate);
            if (!file) return kStemProviderResult_FailedToInitialize;
            
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            std::vector<char> file_data(size);
            if (!file.read(file_data.data(), size)) return kStemProviderResult_FailedToInitialize;
            
            bool success = false;
            try
            {
                success = demucsonnx::load_model(file_data, model, session_options);
            }
            catch (const Ort::Exception& e)
            {
                DBG("ONNX Runtime Exception during model loading: " + String(e.what()));
                return kStemProviderResult_FailedToInitialize;
            }
            catch (const std::exception& e)
            {
                DBG("Exception during model loading: " + String(e.what()));
                return kStemProviderResult_FailedToInitialize;
            }
            
            if (!success) return kStemProviderResult_FailedToInitialize;
            
            // Process audio with Demucs
            const int numSamples = static_cast<int>(reader->lengthInSamples);
            const int numChannels = reader->numChannels > 2 ? 2 : static_cast<int>(reader->numChannels); // Use at most 2 channels
            
            // Read audio data
            AudioBuffer<float> buffer(numChannels, numSamples);
            reader->read(&buffer, 0, numSamples, 0, true, true);
            
            // Convert to Eigen matrix format for Demucs
            Eigen::MatrixXf audioData(2, numSamples);
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* channelData = buffer.getReadPointer(ch);
                for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
                {
                    audioData(ch, sampleIndex) = channelData[sampleIndex];
                }
                
                // If mono, duplicate to second channel
                if (numChannels == 1 && ch == 0)
                {
                    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
                    {
                        audioData(1, sampleIndex) = channelData[sampleIndex];
                    }
                }
            }
            
            // Run Demucs inference
            Eigen::Tensor3dXf out_targets;
            try
            {
                out_targets = demucsonnx::demucs_inference(model, audioData, 
                    [this](float progress, const std::string& message) 
                {
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
            }
            catch (const Ort::Exception& e)
            {
                DBG("ONNX Runtime Exception during inference: " + String(e.what()));
                return kStemProviderResult_FailedToSplit;
            }
            catch (const std::runtime_error& e)
            {
                DBG("Runtime error during inference: " + String(e.what()));
                return kStemProviderResult_Interrupted;
            }
            catch (const std::exception& e)
            {
                DBG("Exception during inference: " + String(e.what()));
                return kStemProviderResult_FailedToSplit;
            }
            
            if (threadShouldExit()) return kStemProviderResult_Interrupted;
            
            // Save separated tracks
            OggVorbisAudioFormat oggFormat;
            WavAudioFormat wavFormat;
            
            // Map Demucs output targets to our part names
            // Demucs output order: drums, bass, other, vocals, guitar, piano
            constexpr int kNumParts = 6;
            const static std::string partNames[kNumParts] = {
                "drums", "bass", "other", "vocals", "guitar", "piano"
            };

            // Get original sample rate
            auto originalSampleRate = MelissaDataSource::getInstance()->getSampleRate();

            for (size_t partIndex = 0; partIndex < kNumParts; ++partIndex)
            {
                if (threadShouldExit()) return kStemProviderResult_Interrupted;
                
                AudioBuffer<float> outBuffer(numChannels, numSamples);
                
                // Copy data from Demucs output to AudioBuffer
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float* channelData = outBuffer.getWritePointer(ch);
                    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
                    {
                        channelData[sampleIndex] = out_targets(partIndex, ch, sampleIndex);
                    }
                }
                
                MemoryAudioSource audioSource(outBuffer, false);
                ResamplingAudioSource resamplingAudioSource(&audioSource, false, numChannels);
                constexpr float fromSampleRate = 44100.0;
                resamplingAudioSource.setResamplingRatio(fromSampleRate / originalSampleRate);
                resamplingAudioSource.prepareToPlay(2048, originalSampleRate);

                const String fileExtension = (audioFileFormat_ == kStemOutputAudioFormat_Wav) ? ".wav" : ".ogg";
                auto outputFile = outputDirName.getChildFile(songName + "_" + partNames[partIndex] + fileExtension);
                
                std::unique_ptr<AudioFormatWriter> writer;
                if (audioFileFormat_ == kStemOutputAudioFormat_Wav)
                {
                    writer.reset(wavFormat.createWriterFor(new FileOutputStream(outputFile), originalSampleRate, numChannels, 16, StringPairArray(), 0));
                }
                else // Ogg format
                {
                    writer.reset(oggFormat.createWriterFor(new FileOutputStream(outputFile), originalSampleRate, numChannels, 16, StringPairArray(), 0));
                }
                
                if (writer == nullptr) return kStemProviderResult_FailedToReadSourceFile;
                                
                // Save current part
                if (writer)
                {
                    writer->writeFromAudioSource(resamplingAudioSource, originalLengthInSamples);
                    writer->flush();
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
        catch (const Ort::Exception& e)
        {
            DBG("ONNX Runtime Exception: " + String(e.what()));
            return kStemProviderResult_FailedToSplit;
        }
        catch (const std::exception& e)
        {
            DBG("Standard Exception: " + String(e.what()));
            return kStemProviderResult_FailedToSplit;
        }
        catch (...)
        {
            DBG("Unknown exception occurred during Demucs processing");
            return kStemProviderResult_UnknownError;
        }
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
        
        const String fileExtension = (audioFileFormat_ == kStemOutputAudioFormat_Wav) ? ".wav" : ".ogg";
        
        for (auto& part : partNames_)
        {
            const auto stemFileName = songName + "_" + part + fileExtension;
            stemSettings[part]["file_name"] = stemFileName.toStdString();
            
            File stemFile(outputDirName.getChildFile(stemFileName));
            
            if (!stemFile.existsAsFile() && part == "guitar") continue;
            
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
    
    status_ = (separatorType_ == kSeparatorType_Demucs) ? kStemProviderStatus_Available_Full : kStemProviderStatus_Available_NoGuitar;
    return (separatorType_ == kSeparatorType_Demucs) ? kStemProviderResult_Success_Full : kStemProviderResult_Success_NoGuitar;
}
