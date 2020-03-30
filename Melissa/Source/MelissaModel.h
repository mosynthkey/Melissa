#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaAudioEngine.h"
#include "MelissaModelListener.h"

class MelissaModel
{
public:
    void setMelissaAudioEngine(MelissaAudioEngine* audioEngine) { audioEngine_ = audioEngine; }
    
    void setLengthMSec(float lengthMSec);
    float getLengthMSec() const { return lengthMSec_; }
    
    void setPlaybackStatus(PlaybackStatus status);
    void togglePlaybackStatus();
    PlaybackStatus getPlaybackStatus() const { return playbackStatus_; }
    
    void setVolume(float volume);
    float getVolume() const { return volume_; }
    
    void setPitch(int semitone);
    int getPitch() const  { return semitone_; }
    
    void setSpeed(int speed);
    int getSpeed() const  { return speed_; }
    
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
    float getPlayingPosRatio() const;
    
    void setPlayingPosMSec(float playbackPosMSec);
    float getPlayingPosMSec() const;
    
    void synchronize();
    
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
    
    MelissaAudioEngine* audioEngine_;
    
    PlaybackStatus playbackStatus_;
    std::vector<MelissaModelListener*> listeners_;
    float lengthMSec_;
    float volume_;
    int semitone_;
    int speed_;
    float aPosRatio_, bPosRatio_;
    float playingPosRatio_;
    String filePath_;
    
    // Singleton
    static MelissaModel instance_;
};
