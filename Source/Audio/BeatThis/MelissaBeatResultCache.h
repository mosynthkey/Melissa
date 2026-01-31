//
//  MelissaBeatResultCache.h
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#pragma once

#include <JuceHeader.h>
#include "MelissaBeatDetector.h"
#include <string>

class MelissaBeatResultCache
{
public:
    static MelissaBeatResultCache* getInstance();
    
    // Cache management
    bool loadCachedResult(const std::string& audioFilePath, MelissaBeatResult& result);
    void saveCachedResult(const std::string& audioFilePath, const MelissaBeatResult& result);
    bool hasCachedResult(const std::string& audioFilePath);
    void removeCachedResult(const std::string& audioFilePath);
    void clearCache();
    
    // File hash management
    std::string getAudioFileHash(const std::string& audioFilePath);
    
private:
    MelissaBeatResultCache();
    ~MelissaBeatResultCache();
    
    std::string getCacheFilePath(const std::string& audioFilePath);
    std::string getCacheDirectory();
    std::string calculateFileHash(const std::string& filePath);
    
    juce::File cacheDirectory_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaBeatResultCache)
};