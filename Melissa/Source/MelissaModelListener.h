//
//  MelissaModelListener.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "MelissaDefinitions.h"

enum PlaybackStatus
{
    kPlaybackStatus_Playing,
    kPlaybackStatus_Pause,
    kPlaybackStatus_Stop
};

enum PlaybackMode
{
    kPlaybackMode_LoopOneSong,
    kPlaybackMode_LoopPlaylistSongs,
    kNumOfPlaybackModes
};

enum OutputMode : int
{
    kOutputMode_LR,
    kOutputMode_LL,
    kOutputMode_RR,
    kOutputMode_CenterCancel,
    kNumOfOutputModes
};

enum SpeedMode : int
{
    kSpeedMode_Basic,
#if defined(ENABLE_SPEED_TRAINING)
    kSpeedMode_Training,
#endif
    kNumOfSpeedModes
};

class MelissaModelListener
{
public:
    virtual ~MelissaModelListener() {};
    
    virtual void playbackStatusChanged(PlaybackStatus status) {}
    virtual void playbackModeChanged(PlaybackMode mode) {}
    virtual void musicVolumeChanged(float volume) {}
    virtual void pitchChanged(float semitone) {}
    virtual void speedChanged(int speed) {}
#if defined(ENABLE_SPEED_TRAINING)
    virtual void speedModeChanged(SpeedMode mode) {}
    virtual void speedIncStartChanged(int speedIncStart) {}
    virtual void speedIncValueChanged(int speedIncValue) {}
    virtual void speedIncPerChanged(int speedIncPer) {}
    virtual void speedIncGoalChanged(int speedIncGoal) {}
#endif
    virtual void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) {}
    virtual void playingPosChanged(float time, float ratio) {}
    virtual void metronomeSwitchChanged(bool on) {}
    virtual void bpmChanged(float bpm) {}
    virtual void beatPositionChanged(float beatPositionMSec) {}
    virtual void accentChanged(int accent) {}
    virtual void metronomeVolumeChanged(float volume) {}
    virtual void musicMetronomeBalanceChanged(float balance) {}
    virtual void outputModeChanged(OutputMode outputMode) {}
    virtual void eqSwitchChanged(bool on) {}
    virtual void eqFreqChanged(size_t band, float freq) {}
    virtual void eqGainChanged(size_t band, float gain) {}
    virtual void eqQChanged(size_t band, float q) {}
    virtual void playPartChanged(StemType playPart) {}
};

