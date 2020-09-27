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
#include <memory>

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
    
    int32_t getPlayingSpeed() const { return currentSpeed_; }
    
    void render(float* bufferToRender[], std::vector<float>&  timeIndicesMSec, size_t bufferLength);
    
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
    
#if defined(ENABLE_SPEED_TRAINING)
    int32_t count_;
    SpeedMode speedMode_;
    int32_t speedIncStart_;
    int32_t speedIncPer_;
    int32_t speedIncValue_;
    int32_t speedIncGoal_;
#endif
    int32_t currentSpeed_;
    
    float volumeBalance_;
    OutputMode outputMode_;
    
    class Equalizer;
    bool eqSwitch_;
    std::unique_ptr<Equalizer> eq_;
    
    
    // MelissaModelListener
    void musicVolumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedChanged(int speed) override;
#if defined(ENABLE_SPEED_TRAINING)
    void speedModeChanged(SpeedMode mode) override;
    void speedIncStartChanged(int speedIncStart) override;
    void speedIncValueChanged(int speedIncValue) override;
    void speedIncPerChanged(int speedIncPer) override;
    void speedIncGoalChanged(int speedIncGoal) override;
#endif
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
    void musicMetronomeBalanceChanged(float balance) override;
    void outputModeChanged(OutputMode outputMode) override;
    void eqSwitchChanged(bool on) override;
    void eqFreqChanged(size_t band, float freq) override;
    void eqGainChanged(size_t band, float gain) override;
    void eqQChanged(size_t band, float q) override;
};
