#pragma once

class MelissaModelListener
{
public:
    virtual ~MelissaModelListener() {};
    
    virtual void volumeChanged(float volume) {}
    virtual void pitchChanged(int semitone) {}
    virtual void speedChanged(int speed) {}
    virtual void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) {}
    virtual void playingPosChanged(float time, float ratio) {}
};

