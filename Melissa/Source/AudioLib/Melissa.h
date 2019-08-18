//
//  Melissa.hpp
//  Melissa_JUCE - App
//
//  Created by Masaki on 2019/07/29.
//

#pragma once

#include "SoundTouch.h"
#include <mutex>
#include <vector>

class Melissa
{
public:
    Melissa();
    virtual ~Melissa() {};
    
    void setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate);
    void setOutputSampleRate(int32_t sampleRate);
    
    void setAPosMSec(float aPosMSec);
    void setBPosMSec(float bPosMSec);
    void setPlayingPosMSec(float playingPosMSec);
    void setAPosRatio(float ratio);
    void setBPosRatio(float ratio);
    void setPlayingPosRatio(float ratio);
    int32_t getTotalLengthMSec() const;
    int32_t getPlayingPosMSec() const;
    float getPlayingPosRatio() const;
    
    void setSpeed(float speed);
    void setPitch(int32_t semitone);
    float setVolume(float volume);
    
    void render(float* bufferToRender[], size_t bufferLength);
    
    void process();
    bool needToProcess();
    float getProgress() const;
    bool isBufferSet() const;
    
    void reset();
    
    // for debug
    std::string getStatusString() const;
    
private:
    std::unique_ptr<soundtouch::SoundTouch> soundTouch_;
    
    bool isOriginalBufferPrepared_;
    std::vector<float> originalBuffer_[2 /* Stereo */];
    int32_t originalSampleRate_;
    
    std::vector<float> processedBuffer_[2 /* Stereo */];
    int32_t outputSampleRate_;
    
    size_t aIndex_, bIndex_;
    size_t playingIndex_, readIndex_, writeIndex_;
    
    float   speed_;
    int32_t semitone_;
    float   volume_;
    
    float   progress_;
    bool    isProcessDone_;
    
    std::mutex mutex_;
    bool isProcessedBufferPrepared_;
    void prepareProcessedBuffer_();
    
    static constexpr size_t processLength_ = 4096;
    float bufferForSoundTouch_[2 * processLength_];
    int32_t totalReceivedSampleSize_;
};
