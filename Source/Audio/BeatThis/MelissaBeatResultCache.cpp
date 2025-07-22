//
//  MelissaBeatResultCache.cpp
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#include "MelissaBeatResultCache.h" 
#include <JuceHeader.h>
#include <sstream>
#include <iomanip>

MelissaBeatResultCache* MelissaBeatResultCache::getInstance()
{
    static MelissaBeatResultCache instance;
    return &instance;
}

MelissaBeatResultCache::MelissaBeatResultCache()
{
    // Initialize cache directory (same directory as Settings.json)
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    cacheDirectory_ = appDataDir.getChildFile("Melissa");
    
    if (!cacheDirectory_.exists())
    {
        cacheDirectory_.createDirectory();
    }
}

MelissaBeatResultCache::~MelissaBeatResultCache() = default;

std::string MelissaBeatResultCache::getCacheDirectory()
{
    return cacheDirectory_.getFullPathName().toStdString();
}

std::string MelissaBeatResultCache::calculateFileHash(const std::string& filePath)
{
    juce::File file(filePath);
    if (!file.exists())
        return "";
    
    // Use file path and modification time for hash
    auto modTime = file.getLastModificationTime().toMilliseconds();
    auto fileSize = file.getSize();
    
    // Create simple hash from path, size, and modification time
    std::stringstream ss;
    ss << std::hex << std::hash<std::string>{}(filePath) 
       << std::hex << fileSize 
       << std::hex << modTime;
    
    auto hashStr = ss.str();
    return hashStr.length() > 16 ? hashStr.substr(0, 16) : hashStr;
}

std::string MelissaBeatResultCache::getAudioFileHash(const std::string& audioFilePath)
{
    return calculateFileHash(audioFilePath);
}

std::string MelissaBeatResultCache::getCacheFilePath(const std::string& audioFilePath)
{
    juce::File audioFile(audioFilePath);
    auto fileName = audioFile.getFileNameWithoutExtension().toStdString();
    auto hash = calculateFileHash(audioFilePath);
    
    if (hash.empty())
        return "";
    
    return cacheDirectory_.getChildFile(fileName + "_" + hash + ".beats").getFullPathName().toStdString();
}

bool MelissaBeatResultCache::hasCachedResult(const std::string& audioFilePath)
{
    auto cacheFile = getCacheFilePath(audioFilePath);
    if (cacheFile.empty())
        return false;
    
    juce::File file(cacheFile);
    return file.exists();
}

bool MelissaBeatResultCache::loadCachedResult(const std::string& audioFilePath, MelissaBeatResult& result)
{
    auto cacheFile = getCacheFilePath(audioFilePath);
    if (cacheFile.empty())
        return false;
    
    juce::File file(cacheFile);
    if (!file.exists())
        return false;
    
    try
    {
        auto content = file.loadFileAsString();
        auto lines = juce::StringArray::fromLines(content);
        
        result = MelissaBeatResult(); // Reset result
        
        for (const auto& line : lines)
        {
            auto trimmedLine = line.trim();
            
            if (trimmedLine.startsWith("#"))
            {
                // Parse metadata
                if (trimmedLine.startsWith("# BPM:"))
                {
                    auto bpmStr = trimmedLine.substring(6).trim();
                    result.estimatedBPM = bpmStr.getFloatValue();
                }
                // Skip other metadata for now
                continue;
            }
            
            if (trimmedLine.isEmpty())
                continue;
            
            // Parse beat data: position(seconds) beat_number
            auto tokens = juce::StringArray::fromTokens(trimmedLine, " \t", "\"");
            tokens.removeEmptyStrings(true);
            
            if (tokens.size() >= 2)
            {
                float position = tokens[0].getFloatValue();
                int beatNumber = tokens[1].getIntValue();
                
                result.beatPositions.push_back(position);
                result.beatCounts.push_back(beatNumber);
                
                if (beatNumber == 1) // Downbeat
                {
                    result.downbeatPositions.push_back(position);
                }
            }
        }
        
        result.isValid = !result.beatPositions.empty();
        return result.isValid;
    }
    catch (const std::exception& e)
    {
        DBG("Failed to load cached beat result: " << e.what());
        return false;
    }
}

void MelissaBeatResultCache::saveCachedResult(const std::string& audioFilePath, const MelissaBeatResult& result)
{
    if (!result.isValid)
        return;
    
    auto cacheFile = getCacheFilePath(audioFilePath);
    if (cacheFile.empty())
        return;
    
    try
    {
        juce::File file(cacheFile);
        
        std::stringstream content;
        
        // Write metadata
        content << "# BPM: " << result.estimatedBPM << "\n";
        content << "# Analysis: " << juce::Time::getCurrentTime().toISO8601(true).toStdString() << "\n";
        
        // Write beat data
        for (size_t i = 0; i < result.beatPositions.size(); ++i)
        {
            content << std::fixed << std::setprecision(3) << result.beatPositions[i];
            
            if (i < result.beatCounts.size())
            {
                content << "\t" << result.beatCounts[i];
            }
            else
            {
                content << "\t1"; // Default to downbeat if no count available
            }
            
            content << "\n";
        }
        
        file.replaceWithText(content.str());
        
        DBG("Saved beat result cache to: " << cacheFile);
    }
    catch (const std::exception& e)
    {
        DBG("Failed to save cached beat result: " << e.what());
    }
}

void MelissaBeatResultCache::clearCache()
{
    auto files = cacheDirectory_.findChildFiles(juce::File::findFiles, false, "*.beats");
    for (auto& file : files)
    {
        file.deleteFile();
    }
    
    DBG("Cleared beat result cache");
}

void MelissaBeatResultCache::cleanupOldCacheFiles()
{
    auto files = cacheDirectory_.findChildFiles(juce::File::findFiles, false, "*.beats");
    auto currentTime = juce::Time::getCurrentTime();
    
    for (auto& file : files)
    {
        auto modTime = file.getLastModificationTime();
        auto daysSinceModification = (currentTime - modTime).inDays();
        
        // Remove cache files older than 30 days
        if (daysSinceModification > 30)
        {
            file.deleteFile();
            DBG("Cleaned up old cache file: " << file.getFileName());
        }
    }
}