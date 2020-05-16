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
    ~MelissaAudioEngine();
    
    void updateBuffer();
    void setOutputSampleRate(int32_t sampleRate);
    
    float getPlayingPosMSec() const;
    float getPlayingPosRatio() const;
    
    bool isMetoronomeOn() const { return metronome_.on_ ;};
    void setMetoronome(bool on) { metronome_.on_ = on; };
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
    
    class SampleIndexStretcher;
    std::unique_ptr<SampleIndexStretcher> sampleIndexStretcher_;
    
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
    int32_t speedIncGoal_;
    int32_t currentSpeed_;
    
    float volumeBalance_;
    OutputMode outputMode_;
    
    // metronome
    struct Metronome
    {
        Metronome() : on_(true), volume_(1.f), beatPositionMSec_(0.f), bpm_(120.f), accent_(4), prevBeatSection_(0), prevAccentBeatSection_(0), amp_(0.f), osc_(-1.f), pitch_(880) { }
        bool on_;
        float volume_;
        float beatPositionMSec_;
        float bpm_;
        int accent_;
        int prevBeatSection_;
        int prevAccentBeatSection_;
        
        // for beep
        float amp_;
        float osc_;
        float pitch_;
    } metronome_;
    
    // MelissaModelListener
    void musicVolumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedModeChanged(SpeedMode mode) override;
    void speedChanged(int speed) override;
    void speedIncStartChanged(int speedIncStart) override;
    void speedIncValueChanged(int speedIncValue) override;
    void speedIncPerChanged(int speedIncPer) override;
    void speedIncGoalChanged(int speedIncGoal) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
    void metronomeSwitchChanged(bool on) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void accentUpdated(int accent) override;
    void metronomeVolumeUpdated(float volume) override;
    void musicMetronomeBalanceUpdated(float balance) override;
    void outputModeChanged(OutputMode outputMode) override;
};
