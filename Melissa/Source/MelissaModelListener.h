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
    virtual void bpmChanged(float bpm) {}
    virtual void beatPositionChanged(float beatPositionMSec) {}
};

