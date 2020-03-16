//
//  Melissa.h
//  Melissa_JUCE - App
//
//  Created by Masaki on 2019/07/29.
//

#pragma once

#include <deque>
#include <mutex>
#include <vector>
#include "MelissaModel.h"
#include "SoundTouch.h"

class Melissa : public MelissaModelListener
{
public:
    Melissa();
    virtual ~Melissa() {};
    
    void setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate);
    void setOutputSampleRate(int32_t sampleRate);
    
    void setAPosMSec(float aPosMSec);
    void setAPosRatio(float ratio);
    void setBPosMSec(float bPosMSec);
    void setBPosRatio(float ratio);
    void setABPosRatio(float aRatio, float bRatio);
    void setPlayingPosMSec(float playingPosMSec);
    void setPlayingPosRatio(float ratio);
    
    float getAPosMSec() const;
    float getAPosRatio() const;
    float getBPosMSec() const;
    float getBPosRatio() const;
    int32_t getTotalLengthMSec() const;
    float getPlayingPosMSec() const;
    float getPlayingPosRatio() const;
    
    void setSpeed(int32_t speed);
    void setSpeedIncPer(int32_t speedIncPer);
    void setSpeedIncValue(int32_t speedIncValue);
    void setSpeedIncMax(int32_t speedIncMax);
    void setPitch(int32_t semitone);
    void setVolume(float volume);
    
    int32_t getSpeed() const { return speed_; }
    int32_t getSpeedIncPer() const { return speedIncPer_; };
    int32_t getSpeedIncValue() const  { return speedIncValue_; };
    int32_t getSpeedIncMax() const { return speedIncMax_; }
    int32_t getCurrentSpeed() const { return speed_ + speedIncValue_; }
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
    std::deque<float> timeQue_;
    int32_t outputSampleRate_;
    
    size_t aIndex_, bIndex_, processStartIndex_;
    size_t readIndex_; // from originalBuffer_
    float playingPosMSec_;
    
    int32_t speed_;
    int32_t semitone_;
    float   volume_;
    
    float bufferForSoundTouch_[2 * processLength_];
    bool needToReset_;
    std::mutex mutex_;
    
    int32_t count_;
    int32_t speedIncPer_;
    int32_t speedIncValue_;
    int32_t speedIncMax_;
    int32_t currentSpeed_;
    
    // metronome
    struct Metronome
    {
        Metronome() : on_(false), volume_(1.f), offsetSec_(0.f), bpm_(120), count_(0),
        amp_(0.f), osc_(-1.f) { }
        bool on_;
        float volume_;
        float offsetSec_;
        uint32_t bpm_;
        int32_t count_;
        
        // for beep
        float amp_;
        float osc_;
    } metronome_;
    
    // MelissaModelListener
    void volumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedChanged(int speed) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
};
