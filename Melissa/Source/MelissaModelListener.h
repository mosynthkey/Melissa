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

class MelissaModelListener
{
public:
    virtual ~MelissaModelListener() {};
    
    virtual void playbackStatusChanged(PlaybackStatus status) {}
    virtual void volumeChanged(float volume) {}
    virtual void pitchChanged(int semitone) {}
    virtual void speedChanged(int speed) {}
    virtual void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) {}
    virtual void playingPosChanged(float time, float ratio) {}
    virtual void metronomeStatusChanged(MetronomeStatus status) {}
    virtual void bpmChanged(float bpm) {}
    virtual void beatPositionChanged(float beatPositionMSec) {}
    virtual void accentUpdated(int accent) {}
    virtual void outputModeChanged(OutputMode outputMode) {}
};

