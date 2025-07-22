//
//  MelissaBeatDetector.h
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#pragma once

#include <JuceHeader.h>
#include "../../../Submodule/beat_this_cpp/Source/beat_this_api.h"
#include "../MelissaDataSource.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>

struct MelissaBeatResult
{
    std::vector<float> beatPositions;    // Beat positions in seconds
    std::vector<float> downbeatPositions; // Downbeat positions in seconds
    std::vector<int> beatCounts;         // Beat numbers (1=downbeat, 2,3,4...=other beats)
    float estimatedBPM;                  // Estimated BPM from beat analysis
    bool isValid;                        // Whether the analysis was successful
    
    MelissaBeatResult() : estimatedBPM(0.0f), isValid(false) {}
};

class MelissaBeatDetector
{
public:
    using ProgressCallback = std::function<void(float progress)>; // 0.0 to 1.0
    using CompletionCallback = std::function<void(const MelissaBeatResult& result, bool success)>;
    
    MelissaBeatDetector();
    ~MelissaBeatDetector();
    
    // Initialize with model path
    bool initialize(const std::string& modelPath);
    
    // Async processing
    void startAnalysisAsync(const ProgressCallback& progressCallback,
                           const CompletionCallback& completionCallback);
    void cancelAnalysis();
    bool isAnalysisRunning() const;
    
    // Get cached result if available
    bool hasCachedResult() const;
    const MelissaBeatResult& getCachedResult() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaBeatDetector)
};