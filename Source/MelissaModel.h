//
//  MelissaModel.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

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
    
    void setMusicVolume(float volume);
    float getMusicVolume() const { return musicVolume_; }
    
    void setPitch(float semitone);
    float getPitch() const  { return semitone_; }
    
    void setSpeed(int speed);
    int getSpeed() const  { return speed_; }
    
    int getPlayingSpeed() { return audioEngine_->getPlayingSpeed(); }
    void setSpeedIncStart(int speedIncStart);
    
#if defined(ENABLE_SPEED_TRAINING)
    void setSpeedMode(SpeedMode speedMode);
    SpeedMode getSpeedMode() { return speedMode_; }
    int getSpeedIncStart() { return speedIncStart_; }
    void setSpeedIncValue(int speedIncValue);
    int getSpeedIncValue() { return speedIncValue_; }
    void setSpeedIncPer(int speedIncPer);
    int getSpeedIncPer() { return speedIncPer_; }
    void setSpeedIncGoal(int speedIncGoal);
    int getSpeedIncGoal() { return speedIncGoal_; }
    void resetSpeedTraining();
#endif
    
    void setLoopPosRatio(float aRatio, float bRatio);
    
    void setLoopAPosRatio(float aPosRatio);
    float getLoopAPosRatio() const { return aPosRatio_; }
    
    void setLoopAPosMSec(float aPosMSec);
    float getLoopAPosMSec() const { return aPosRatio_ * lengthMSec_; }
    
    void setLoopBPosRatio(float bPosRatio);
    float getLoopBPosRatio() const { return bPosRatio_; }
    
    void setLoopBPosMSec(float bPosMSec);
    float getLoopBPosMSec() const { return bPosRatio_ * lengthMSec_; }
    
    void setPlayingPosRatio(float playingPosRatio);
    float getPlayingPosRatio() const;
    
    void setPlayingPosMSec(float playingPosMSec);
    float getPlayingPosMSec() const;
    void updatePlayingPosMSecFromDsp(float playingPosMSec);
    
    void setPlaybackMode(PlaybackMode mode);
    PlaybackMode getPlaybackMode() const { return playbackMode_; }
    
    // Metronome
    void  setMetronomeSwitch(bool on);
    bool  getMetronomeSwitch() const { return metronomeSwitch_; }
    void  setBpm(float bpm);
    float getBpm() { return bpm_; }
    void  setBeatPositionMSec(float beatPositionMSec);
    float getBeatPositionMSec() { return beatPositionMSec_; }
    void  setAccent(int accent);
    int   getAccent() { return accent_; }
    void  setMetronomeVolume(float volume);
    float getMetronomeVolume() { return metronomeVolume_; }
    
    // Output
    void setMusicMetronomeBalance(float balance);
    float getMusicMetronomeBalance() { return musicMetronomeBalance_; }
    void setOutputMode(OutputMode outputMode);
    OutputMode getOutputMode() const { return outputMode_; }
    
    // EQ
    void  setEqSwitch(bool on);
    bool  getEqSwitch() { return eqSwitch_; }
    void  setEqFreq(size_t band, float freq);
    float getEqFreq(size_t band) { return eqFreq_; }
    void  setEqGain(size_t band, float gain);
    float getEqGain(size_t band) { return eqGain_; }
    void  setEqQ(size_t band, float eqQ);
    float getEqQ(size_t band) { return eqQ_; }
    
    // Part
    void setPlayPart(PlayPart playPart);
    PlayPart getPlayPart() const { return playPart_; }
    void setCustomPartVolume(CustomPartVolume part, float volume);
    float getCustomPartVolume(CustomPartVolume part) const { return customPartVolume_[part]; }
    
    // Main volume
    void setMainVolume(float mainVolume);
    float getMainVolume() const { return mainVolume_; }
    
    // Pre Count
    void setPreCountSwitch(bool preCountSwitch);
    bool getPreCountSwitch() const { return preCountSwitch_; }
    
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
    PlaybackMode playbackMode_;
    bool metronomeSwitch_;
    std::vector<MelissaModelListener*> listeners_;
    float lengthMSec_;
    float musicVolume_;
    float metronomeVolume_;
    float musicMetronomeBalance_;
    float semitone_;
    SpeedMode speedMode_;
    int speed_;
    int currentSpeed_;
    int speedIncStart_;
    int speedIncValue_;
    int speedIncPer_;
    int speedIncGoal_;
    float aPosRatio_, bPosRatio_;
    float playingPosRatio_;
    float bpm_, beatPositionMSec_;
    int accent_;
    juce::String filePath_;
    OutputMode outputMode_;
    bool eqSwitch_;
    float eqFreq_;
    float eqGain_;
    float eqQ_;
    PlayPart playPart_;
    float customPartVolume_[kNumCustomPartVolumes];
    float mainVolume_;
    bool preCountSwitch_;
    
    // Singleton
    static MelissaModel instance_;
};
