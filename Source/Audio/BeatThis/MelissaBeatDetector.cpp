//
//  MelissaBeatDetector.cpp
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#include "MelissaBeatDetector.h"
#include "MelissaBeatResultCache.h"
#include <thread>
#include <atomic>
#include <algorithm>
#include <iostream>

class MelissaBeatDetector::Impl
{
public:
    Impl() :
        isInitialized_(false),
        isAnalysisRunning_(false)
    {
    }
    
    ~Impl()
    {
        cancelAnalysis();
    }
    
    bool initialize(const std::string& modelPath)
    {
        std::cout << "MelissaBeatDetector::initialize called with path: " << modelPath << std::endl;
        
        try 
        {
            std::cout << "Creating BeatThis instance..." << std::endl;
            beatThis_ = std::make_unique<BeatThis::BeatThis>(modelPath);
            std::cout << "BeatThis instance created successfully" << std::endl;
            isInitialized_ = true;
            std::cout << "Beat detector initialized successfully!" << std::endl;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cout << "Failed to initialize BeatThis: " << e.what() << std::endl;
            DBG("Failed to initialize BeatThis: " << e.what());
            return false;
        }
    }
    
    void startAnalysisAsync(const ProgressCallback& progressCallback,
                           const CompletionCallback& completionCallback)
    {
        auto dataSource = MelissaDataSource::getInstance();
        
        std::cout << "startAnalysisAsync called. isInitialized_: " << isInitialized_ << ", isAnalysisRunning_: " << isAnalysisRunning_ << std::endl;
        
        if (!isInitialized_)
        {
            std::cout << "Beat detector not initialized!" << std::endl;
            if (completionCallback)
                completionCallback(MelissaBeatResult(), false);
            return;
        }
        
        if (isAnalysisRunning_)
        {
            std::cout << "Analysis already running!" << std::endl;
            if (completionCallback)
                completionCallback(MelissaBeatResult(), false);
            return;
        }
        
        // Check if we have a cached result first
        auto currentFilePath = dataSource->getCurrentSongFilePath().toStdString();
        if (!currentFilePath.empty())
        {
            auto cache = MelissaBeatResultCache::getInstance();
            if (cache->hasCachedResult(currentFilePath))
            {
                MelissaBeatResult cachedResult;
                if (cache->loadCachedResult(currentFilePath, cachedResult))
                {
                    cachedResult_ = cachedResult;
                    if (completionCallback)
                        completionCallback(cachedResult, true);
                    return;
                }
            }
        }
        
        isAnalysisRunning_ = true;
        
        // Start analysis in background thread
        analysisThread_ = std::thread([this, progressCallback, completionCallback]()
        {
            performAnalysis(progressCallback, completionCallback);
        });
        analysisThread_.detach();
    }
    
    void cancelAnalysis()
    {
        isAnalysisRunning_ = false;
        // Note: In a real implementation, we'd need to add cancellation points
        // in the BeatThis processing if it supports cancellation
    }
    
    bool isAnalysisRunning() const
    {
        return isAnalysisRunning_;
    }
    
    bool hasCachedResult() const
    {
        return cachedResult_.isValid;
    }
    
    const MelissaBeatResult& getCachedResult() const
    {
        return cachedResult_;
    }
    
private:
    void performAnalysis(const ProgressCallback& progressCallback,
                        const CompletionCallback& completionCallback)
    {
        MelissaBeatResult result;
        
        try
        {
            auto dataSource = MelissaDataSource::getInstance();
            
            std::cout << "Starting performAnalysis..." << std::endl;
            
            // Get audio data from data source
            const size_t bufferLength = dataSource->getBufferLength();
            const int sampleRate = dataSource->getSampleRate();
            
            std::cout << "Buffer length: " << bufferLength << ", Sample rate: " << sampleRate << std::endl;
            
            if (progressCallback)
                progressCallback(0.1f); // 10% - starting to read audio
            
            // Read all audio data (stereo -> mono conversion)
            std::vector<float> audioData;
            audioData.reserve(bufferLength);
            
            constexpr size_t chunkSize = 4096;
            float lCh[chunkSize], rCh[chunkSize];
            float* audioChannels[] = { lCh, rCh };
            
            for (size_t pos = 0; pos < bufferLength && isAnalysisRunning_; pos += chunkSize)
            {
                const size_t samplesToRead = std::min(chunkSize, bufferLength - pos);
                dataSource->readBuffer(MelissaDataSource::kReader_BPM, 
                                       static_cast<int>(pos),
                                       static_cast<int>(samplesToRead), 
                                       kPlayPart_All, 
                                       audioChannels);
                
                // Convert stereo to mono
                for (size_t i = 0; i < samplesToRead; ++i)
                {
                    audioData.push_back((lCh[i] + rCh[i]) * 0.5f);
                }
                
                if (progressCallback)
                {
                    float progress = 0.1f + 0.3f * static_cast<float>(pos) / bufferLength;
                    progressCallback(progress);
                }
            }
            
            if (!isAnalysisRunning_)
            {
                if (completionCallback)
                    completionCallback(result, false);
                return;
            }
            
            if (progressCallback)
                progressCallback(0.4f); // 40% - audio loaded, starting analysis
            
            std::cout << "Audio data size: " << audioData.size() << " samples" << std::endl;
            std::cout << "Calling BeatThis process_audio..." << std::endl;
            
            // Process with BeatThis
            BeatThis::BeatResult beatThisResult = beatThis_->process_audio(audioData, sampleRate, 1);
            
            std::cout << "BeatThis processing completed" << std::endl;
            
            if (progressCallback)
                progressCallback(0.9f); // 90% - analysis complete, processing results
            
            // Convert result
            result.beatPositions = std::move(beatThisResult.beats);
            result.downbeatPositions = std::move(beatThisResult.downbeats);
            result.beatCounts = std::move(beatThisResult.beat_counts);
            
            // Calculate estimated BPM from beat positions
            if (result.beatPositions.size() > 1)
            {
                // Calculate average beat interval
                float totalInterval = 0.0f;
                for (size_t i = 1; i < result.beatPositions.size(); ++i)
                {
                    totalInterval += result.beatPositions[i] - result.beatPositions[i-1];
                }
                float avgInterval = totalInterval / (result.beatPositions.size() - 1);
                result.estimatedBPM = 60.0f / avgInterval; // Convert to BPM
            }
            
            result.isValid = true;
            cachedResult_ = result;
            
            // Save to cache
            auto currentFilePath = dataSource->getCurrentSongFilePath().toStdString();
            if (!currentFilePath.empty())
            {
                auto cache = MelissaBeatResultCache::getInstance();
                cache->saveCachedResult(currentFilePath, result);
            }
            
            if (progressCallback)
                progressCallback(1.0f); // 100% - complete
            
            if (completionCallback)
                completionCallback(result, true);
        }
        catch (const std::exception& e)
        {
            std::cout << "BeatThis analysis failed with exception: " << e.what() << std::endl;
            DBG("BeatThis analysis failed: " << e.what());
            if (completionCallback)
                completionCallback(result, false);
        }
        
        isAnalysisRunning_ = false;
    }
    
    std::unique_ptr<BeatThis::BeatThis> beatThis_;
    std::atomic<bool> isInitialized_;
    std::atomic<bool> isAnalysisRunning_;
    std::thread analysisThread_;
    MelissaBeatResult cachedResult_;
};

// Public interface implementation
MelissaBeatDetector::MelissaBeatDetector() :
    pImpl_(std::make_unique<Impl>())
{
}

MelissaBeatDetector::~MelissaBeatDetector() = default;

bool MelissaBeatDetector::initialize(const std::string& modelPath)
{
    return pImpl_->initialize(modelPath);
}

void MelissaBeatDetector::startAnalysisAsync(const ProgressCallback& progressCallback,
                                                const CompletionCallback& completionCallback)
{
    pImpl_->startAnalysisAsync(progressCallback, completionCallback);
}

void MelissaBeatDetector::cancelAnalysis()
{
    pImpl_->cancelAnalysis();
}

bool MelissaBeatDetector::isAnalysisRunning() const
{
    return pImpl_->isAnalysisRunning();
}

bool MelissaBeatDetector::hasCachedResult() const
{
    return pImpl_->hasCachedResult();
}

const MelissaBeatResult& MelissaBeatDetector::getCachedResult() const
{
    return pImpl_->getCachedResult();
}
