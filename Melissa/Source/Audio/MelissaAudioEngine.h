//
//  MelissaAudioEngine.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <deque>
#include <mutex>
#include <vector>
#include "MelissaModelListener.h"
#include "SoundTouch.h"

class MelissaDataSource;
class MelissaModel;

class MelissaAudioEngine : public MelissaModelListener
{
public:
    MelissaAudioEngine();
    virtual ~MelissaAudioEngine() {};
    
    void updateBuffer();
    void setOutputSampleRate(int32_t sampleRate);
    
    float getPlayingPosMSec() const;
    float getPlayingPosRatio() const;
    
    void setSpeedIncPer(int32_t speedIncPer);
    void setSpeedIncValue(int32_t speedIncValue);
    void setSpeedIncMax(int32_t speedIncMax);
    
    int32_t getSpeedIncPer() const { return speedIncPer_; };
    int32_t getSpeedIncValue() const  { return speedIncValue_; };
    int32_t getSpeedIncMax() const { return speedIncMax_; }
    int32_t getCurrentSpeed() const { return speed_ + speedIncValue_; }
    
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
    MelissaModel* model_;
    MelissaDataSource* dataSource_;
    static constexpr size_t processLength_ = 4096;
    static constexpr size_t queLength_ = 10 * processLength_ * 2 /* Stereo */;
    
    std::unique_ptr<soundtouch::SoundTouch> soundTouch_;
    
    int32_t originalSampleRate_;
    size_t originalBufferLength_;
    
    std::deque<float> processedBufferQue_;
    std::deque<float> timeQue_;
    int32_t outputSampleRate_;
    
    size_t aIndex_, bIndex_, processStartIndex_;
    size_t readIndex_; // from buffer_
    float playingPosMSec_;
    
    int32_t speed_;
    float processingSpeed_;
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
