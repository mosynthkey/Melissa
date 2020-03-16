#pragma once

#include <vector>

class MelissaModelListener
{
public:
    virtual ~MelissaModelListener() {};
    
    virtual void volumeChanged(float volume) = 0;
    virtual void pitchChanged(int semitone) = 0;
    virtual void speedChanged(int speed) = 0;
    virtual void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) = 0;
    virtual void playingPosChanged(float time, float ratio) = 0;
};

class MelissaModel
{
public:
    void setLengthMSec(float totalMSec);
    float getLengthMSec() const { return lengthMSec_; }
    
    void setVolume(float volume);
    float getVolume() const { return volume_; }
    
    void setPitch(int semitone);
    int getPitch() const  { return semitone_; }
    
    void setSpeed(float speed);
    float getSpeed() const  { return speed_; }
    
    void setLoopPosRatio(float aRatio, float bRatio);
    
    void setLoopAPosRatio(float aPosRatio);
    float getLoopAPosRatio() const { return aPosRatio_; }
    
    void setLoopAPosMSec(float aPosMSec);
    float getLoopAPosMSec() const { return aPosRatio_ * lengthMSec_; }
    
    void setLoopBPosRatio(float bPosRatio);
    float getLoopBPosRatio() const { return bPosRatio_; }
    
    void setLoopBPosMSec(float bPosMSec);
    float getLoopBPosMSec() const { return bPosRatio_ * lengthMSec_; }
    
    void setPlayingPosRatio(float playbackPosRatio);
    float getPlayingPosRatio() const { return playingPosRatio_; }
    
    void setPlayingPosMSec(float playbackPosMSec);
    float getPlayingPosMSec() const { return playingPosRatio_ * lengthMSec_; }
    
    // Listener
    void addListener(MelissaModelListener* listener);
    void removeListener(MelissaModelListener* listener);
    
    // Singleton
    static MelissaModel* getInstance();
    MelissaModel(const MelissaModel&) = delete;
    MelissaModel& operator=(const MelissaModel&) = delete;
    MelissaModel(MelissaModel&&) = delete;
    MelissaModel& operator=(MelissaModel&&) = delete;
    
private:
    MelissaModel();
    ~MelissaModel() {};
    
    std::vector<MelissaModelListener*> listeners_;
    float lengthMSec_;
    float volume_;
    int semitone_;
    float speed_;
    float aPosRatio_, bPosRatio_;
    float playingPosRatio_;
    
    // Singleton
    static MelissaModel instance_;
};
