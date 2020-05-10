//
//  MelissaModelListener.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

enum PlaybackStatus
{
    kPlaybackStatus_Playing,
    kPlaybackStatus_Pause,
    kPlaybackStatus_Stop
};

enum MetronomeStatus
{
    kMetronomeStatus_On_Sync,
    kMetronomeStatus_On_Free,
    kMetronomeStatus_Off,
};

enum OutputMode : int
{
    kOutputMode_LR,
    kOutputMode_LL,
    kOutputMode_RR,
    kNumOfOutputModes
};

enum SpeedMode
{
    kSpeedMode_Basic,
    kSpeedMode_Training,
    kNumOfSpeedModes
};

class MelissaModelListener
{
public:
    virtual ~MelissaModelListener() {};
    
    virtual void playbackStatusChanged(PlaybackStatus status) {}
    virtual void musicVolumeChanged(float volume) {}
    virtual void pitchChanged(int semitone) {}
    virtual void speedModeChanged(SpeedMode mode) {}
    virtual void speedChanged(int speed) {}
    virtual void speedIncStartChanged(int speedIncStart) {}
    virtual void speedIncValueChanged(int speedIncValue) {}
    virtual void speedIncPerChanged(int speedIncPer) {}
    virtual void speedIncGoalChanged(int speedIncGoal) {}
    virtual void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) {}
    virtual void playingPosChanged(float time, float ratio) {}
    virtual void metronomeStatusChanged(MetronomeStatus status) {}
    virtual void bpmChanged(float bpm) {}
    virtual void beatPositionChanged(float beatPositionMSec) {}
    virtual void accentUpdated(int accent) {}
    virtual void metronomeVolumeUpdated(float volume) {}
    virtual void musicMetronomeBalanceUpdated(float balance) {}
    virtual void outputModeChanged(OutputMode outputMode) {}
};

