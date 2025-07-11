//
//  MelissaAudioEngine.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <vector>
#include "MelissaBeepGenerator.h"
#include "MelissaModelListener.h"
#include "SoundTouch.h"

class MelissaDataSource;
class MelissaModel;

class MelissaEqualizer
{
public:
    MelissaEqualizer();
    void reset();
    void updateCoefs();
    void process(float *inBuffer, float *outBuffer);
    void setSampleRate(float sampleRate);
    void setFreq(float freq);
    void setGain(float gain);
    void setQ(float q);

private:
    float a[3], b[3];
    static constexpr size_t kNumOfZs_ = 4;
    static constexpr size_t kNumOfChs = 2;
    float z_[kNumOfChs][kNumOfZs_];
    float shouldUpdateCoefs_;
    float sampleRate_, freq_, gainDb_, q_;
};

class MelissaAudioEngine : public MelissaModelListener
{
public:
    MelissaAudioEngine();
    ~MelissaAudioEngine();

    void updateBuffer();
    void setOutputSampleRate(int32_t sampleRate);

    float getPlayingPosMSec() const;
    float getPlayingPosRatio() const;

    int32_t getPlayingSpeed() const { return currentPlaybackSpeed_; }

    void render(float *bufferToRender[], size_t numOfChannels, std::vector<float> &timeIndicesMSec, size_t bufferLength);

    void process();
    bool needToProcess() const;
    bool isBufferSet() const;

    void reset();
    void resetProcessedBuffer();

    enum Status
    {
        kStatus_Playing,
        kStatus_RequestingForNextSong,
        kStatus_WaitingNextSongToLoad,
    };

    Status getStatus() const;
    void setStatus(Status status);

    void setTrimMode(bool shouldTrim);
    void resetSpeedTraining();

private:
    MelissaModel *model_;
    MelissaDataSource *dataSource_;
    static constexpr size_t processLength_ = 4096;
    static constexpr size_t queLength_ = 10 * processLength_ * 2 /* Stereo */;

    std::unique_ptr<soundtouch::SoundTouch> soundTouch_;

    PlaybackStatus playbackStatus_;
    PlaybackMode playbackMode_;

    int32_t originalSampleRate_;
    size_t originalBufferLength_;

    std::deque<float> processedBufferQue_;
    std::deque<float> timeQue_;
    std::deque<float> speedQue_;
    int32_t outputSampleRate_;

    class SampleIndexStretcher;
    std::unique_ptr<SampleIndexStretcher> sampleIndexStretcher_;
    std::unique_ptr<SampleIndexStretcher> speedStretcher_;

    size_t aIndex_, bIndex_, processStartIndex_;
    size_t readIndex_; // from buffer_
    float playingPosMSec_;
    bool shouldTrim_;
    size_t trimStartIndex_, trimEndIndex_;

    int32_t speed_;
    float processingSpeed_;
    float semitone_;
    float volume_;

    float bufferForSoundTouch_[2 * processLength_];
    bool needToReset_;
    bool loop_;
    bool shouldProcess_;
    std::mutex mutex_;

#if defined(ENABLE_SPEED_TRAINING)
    int32_t count_;
    SpeedMode speedMode_;
    int32_t speedIncStart_;
    int32_t speedIncPer_;
    int32_t speedIncValue_;
    int32_t speedIncGoal_;
#endif
    int32_t currentProcessingPlaybackSpeed_;
    int32_t currentPlaybackSpeed_;
    float volumeBalance_;
    OutputMode outputMode_;

    bool eqSwitch_;
    std::unique_ptr<MelissaEqualizer> eq_;

    PlayPart playPart_;

    Status status_;

    MelissaBeepGenerator beepGen_;
    bool enableCountIn_;
    float previousRenderedPosMSec_;
    size_t countInSampleIndex_;
    bool forcePreCountOn_;

    // MelissaModelListener
    void playbackStatusChanged(PlaybackStatus status) override;
    void playbackModeChanged(PlaybackMode mode) override;
    void musicVolumeChanged(float volume) override;
    void pitchChanged(float semitone) override;
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
    void playPartChanged(PlayPart playPart) override;
    void preCountSwitchChanged(bool preCountSwitch) override;

    void updateLoopParameters();
    void setupPreCountIfNeeded();

    void doTrim();
};
