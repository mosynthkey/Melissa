//
//  Melissa.hpp
//  Melissa_JUCE - App
//
//  Created by Masaki on 2019/07/29.
//

#pragma once

#include "SoundTouch.h"
#include <deque>
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
    
    float getAPosMSec() const;
    float getBPosMSec() const;
    int32_t getTotalLengthMSec() const;
    int32_t getPlayingPosMSec() const;
    float getAPosRatio() const;
    float getBPosRatio() const;
    float getPlayingPosRatio() const;
    
    void setSpeed(float speed);
    void setPitch(int32_t semitone);
    void setVolume(float volume);
    
    float getSpeed() const { return speed_; }
    int32_t getPitch() const { return semitone_; }
    float getVolume() const { return volume_; }
    
    bool isMetoronomeOn() const { return metronome_.on_ ;};
    uint32_t getBpm() const { return metronome_.bpm_; }
    float getMetronomeOffsetSec() const { return metronome_.offsetSec_; }
    
    void setMetoronome(bool on) { metronome_.on_ = on; };
    void setBpm(uint32_t bpm) { metronome_.bpm_ = bpm; }
    void setMetronomeOffsetSec(float offsetSec) { metronome_.offsetSec_ = offsetSec; }
    void analyzeBpm();
    
    void render(float* bufferToRender[], size_t bufferLength);
    
    void process();
    bool needToProcess() const;
    bool isBufferSet() const;
    
    void reset();
    void resetProcessedBuffer();
    
    // for debug
    std::string getStatusString() const;
    
private:
    static constexpr size_t processLength_ = 4096;
    static constexpr size_t queLength_ = 10 * processLength_ * 2 /* Stereo */;
    
    std::unique_ptr<soundtouch::SoundTouch> soundTouch_;
    
    bool isOriginalBufferPrepared_;
    std::vector<float> originalBuffer_[2 /* Stereo */];
    int32_t originalSampleRate_;
    size_t originalBufferLength_;
    
    std::deque<float> processedBufferQue_;
    int32_t outputSampleRate_;
    
    size_t aIndex_, bIndex_, startIndex_;
    size_t readIndex_; // from originalBuffer_
    
    float   speed_;
    int32_t semitone_;
    float   volume_;
    
    float bufferForSoundTouch_[2 * processLength_];
    bool needToReset_;
    std::mutex mutex_;
    size_t sampleTime_;
    
    // metronome
    struct Metronome
    {
        Metronome() : on_(false), volume_(1.f), offsetSec_(0.f), bpm_(20), count_(0),
        amp_(0.f), osc_(-1.f) { }
        bool on_;
        float volume_;
        float offsetSec_;
        uint32_t bpm_;
        int32_t count_;
        
        float amp_;
        float osc_;
    } metronome_;
};
