//
//  MelissaModel.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaDefinitions.h"
#include "MelissaModel.h"

MelissaModel MelissaModel::instance_;

void MelissaModel::setLengthMSec(float lengthMSec)
{
    if (lengthMSec < 0.f) return;
    
    lengthMSec_ = lengthMSec;
}

void MelissaModel::setPlaybackStatus(PlaybackStatus playbackStatus)
{
    playbackStatus_ = playbackStatus;
    for (auto l : listeners_) l->playbackStatusChanged(playbackStatus);
}

void MelissaModel::togglePlaybackStatus()
{
    if (playbackStatus_ == kPlaybackStatus_Playing)
    {
        setPlaybackStatus(kPlaybackStatus_Pause);
    }
    else
    {
        setPlaybackStatus(kPlaybackStatus_Playing);
    }
}

void MelissaModel::setMusicVolume(float volume)
{
    if (volume < 0.f || 2.f < volume) return;
    
    musicVolume_ = volume;
    for (auto&& l : listeners_) l->musicVolumeChanged(volume);
}

void MelissaModel::setPitch(int semitone)
{
    if (semitone < -24 || 24 < semitone) return;
    
    semitone_ = semitone;
    for (auto&& l : listeners_) l->pitchChanged(semitone);
}

void MelissaModel::setSpeedMode(SpeedMode speedMode)
{
    speedMode_ = speedMode;
    for (auto&& l : listeners_) l->speedModeChanged(speedMode);
}

void MelissaModel::setSpeed(int speed)
{
    if (speed < kSpeedMin || kSpeedMax < speed) return;
    
    speed_ = speed;
    for (auto&& l : listeners_) l->speedChanged(speed);
}

void MelissaModel::setSpeedIncStart(int speedIncStart)
{
    if (speedIncStart < kSpeedMin || kSpeedMax < speedIncStart || speedIncGoal_ <= speedIncStart) return;
        
    speedIncStart_ = speedIncStart;
    for (auto&& l : listeners_) l->speedIncStartChanged(speedIncStart);
}

void MelissaModel::setSpeedIncValue(int speedIncValue)
{
    speedIncValue = std::clamp(speedIncValue, 1, 20);
    
    speedIncValue_ = speedIncValue;
    for (auto&& l : listeners_) l->speedIncValueChanged(speedIncValue);
}

void MelissaModel::setSpeedIncPer(int speedIncPer)
{
    speedIncPer = std::clamp(speedIncPer, 1, 50);
    
    speedIncPer_ = speedIncPer;
    for (auto&& l : listeners_) l->speedIncPerChanged(speedIncPer);
}

void MelissaModel::setSpeedIncGoal(int speedIncGoal)
{
    if (speedIncGoal < kSpeedMin || kSpeedMax < speedIncGoal || speedIncGoal <= speedIncStart_) return;
    
    speedIncGoal_ = speedIncGoal;
    for (auto&& l : listeners_) l->speedIncGoalChanged(speedIncGoal);
}

void MelissaModel::setLoopPosRatio(float aPosRatio, float bPosRatio)
{
    if (0 < lengthMSec_ && 0.f <= aPosRatio && aPosRatio < bPosRatio && bPosRatio <= 1.f)
    {
        aPosRatio_ = aPosRatio;
        bPosRatio_ = bPosRatio;
        for (auto&& l : listeners_) l->loopPosChanged(lengthMSec_ * aPosRatio_, aPosRatio_, lengthMSec_ * bPosRatio_, bPosRatio_);
    }
}

void MelissaModel::setLoopAPosRatio(float aPosRatio)
{
    if (0 < lengthMSec_ && 0.f <= aPosRatio && aPosRatio < bPosRatio_)
    {
        aPosRatio_ = aPosRatio;
        for (auto&& l : listeners_) l->loopPosChanged(lengthMSec_ * aPosRatio_, aPosRatio_, lengthMSec_ * bPosRatio_, bPosRatio_);
    }
}

void MelissaModel::setLoopAPosMSec(float aPosMSec)
{
    if (0 < lengthMSec_ && 0.f <= aPosMSec && aPosMSec < bPosRatio_ * lengthMSec_)
    {
        aPosRatio_ = aPosMSec / lengthMSec_;
        for (auto&& l : listeners_) l->loopPosChanged(lengthMSec_ * aPosRatio_, aPosRatio_, lengthMSec_ * bPosRatio_, bPosRatio_);
    }
}

void MelissaModel::setLoopBPosRatio(float bPosRatio)
{
    if (0 < lengthMSec_ && aPosRatio_ < bPosRatio_)
    {
        bPosRatio_ = bPosRatio;
        for (auto&& l : listeners_) l->loopPosChanged(lengthMSec_ * aPosRatio_, aPosRatio_, lengthMSec_ * bPosRatio_, bPosRatio_);
    }
}

void MelissaModel::setLoopBPosMSec(float bPosMSec)
{
    if (0 < lengthMSec_ && aPosRatio_  * lengthMSec_ < bPosMSec)
    {
        bPosRatio_ = bPosMSec / lengthMSec_;
        for (auto&& l : listeners_) l->loopPosChanged(lengthMSec_ * aPosRatio_, aPosRatio_, lengthMSec_ * bPosRatio_, bPosRatio_);
    }
}

void MelissaModel::setPlayingPosRatio(float playingPosRatio)
{
    if (playingPosRatio < 0.f || 1.f < playingPosRatio || lengthMSec_ < 0.f) return;
    
    playingPosRatio_ = playingPosRatio;
    for (auto&& l : listeners_) l->playingPosChanged(lengthMSec_ * playingPosRatio, playingPosRatio);
}

float MelissaModel::getPlayingPosRatio() const
{
    return playingPosRatio_;
}

void MelissaModel::setPlayingPosMSec(float playingPosMSec)
{
    if (playingPosMSec < 0 || lengthMSec_ < playingPosMSec || lengthMSec_ < 0.f) return;
    
    playingPosRatio_ = playingPosMSec / lengthMSec_;
    for (auto&& l : listeners_) l->playingPosChanged(playingPosMSec, playingPosMSec / lengthMSec_);
}

float MelissaModel::getPlayingPosMSec() const
{
    return playingPosRatio_ * lengthMSec_;
}

void MelissaModel::updatePlayingPosMSecFromDsp(float playingPosMSec)
{
    if (playingPosMSec < 0 || lengthMSec_ < playingPosMSec || lengthMSec_ < 0.f) return;
    
    playingPosRatio_ = playingPosMSec / lengthMSec_;
}

void  MelissaModel::setMetronomeSwitch(bool on)
{
    metronomeSwitch_ = on;
    for (auto&& l : listeners_) l->metronomeSwitchChanged(on);
}

void MelissaModel::setBpm(float bpm)
{
    bpm_ = bpm;
    for (auto&& l : listeners_) l->bpmChanged(bpm);
}

void MelissaModel::setBeatPositionMSec(float beatPositionMSec)
{
    beatPositionMSec_ = beatPositionMSec;
    
    for (auto&& l : listeners_) l->beatPositionChanged(beatPositionMSec);
}

void MelissaModel::setAccent(int accent)
{
    if (accent < 0 || 16 <= accent) return;
    
    accent_ = accent;
    for (auto&& l : listeners_) l->accentChanged(accent);
}

void MelissaModel::setMetronomeVolume(float volume)
{
    if (volume < 0.f || 1.f < volume) return;
    
    metronomeVolume_ = volume;
    for (auto&& l : listeners_) l->metronomeVolumeChanged(volume);
}

void MelissaModel::setMusicMetronomeBalance(float balance)
{
    if (balance < 0.f || 1.f < balance) return;
    
    musicMetronomeBalance_ = balance;
    for (auto&& l : listeners_) l->musicMetronomeBalanceChanged(balance);
}

void MelissaModel::setOutputMode(OutputMode outputMode)
{
    if (outputMode < 0 || kNumOfOutputModes <= outputMode) return;
    
    outputMode_ = outputMode;
    for (auto&& l : listeners_) l->outputModeChanged(outputMode);
}

void MelissaModel::setEqSwitch(bool on)
{
    eqSwitch_ = on;
    for (auto&& l : listeners_) l->eqSwitchChanged(on);
}

void MelissaModel::setEqFreq(size_t band, float freq)
{
    eqFreq_ = freq = std::clamp<float>(freq, kEqFreqMin, kEqFreqMax);
    for (auto&& l : listeners_) l->eqFreqChanged(band, freq);
}

void MelissaModel::setEqGain(size_t band, float gain)
{
    gain = std::clamp<float>(gain, kEqGainMin, kEqGainMax);
    for (auto&& l : listeners_) l->eqGainChanged(band, gain);
}

void MelissaModel::setEqQ(size_t band, float eqQ)
{
    eqQ = std::clamp<float>(eqQ, kEqQMin, kEqQMax);
    for (auto&& l : listeners_) l->eqQChanged(band, eqQ);
}

void MelissaModel::addListener(MelissaModelListener* listener)
{
    for (auto&& l : listeners_)
    {
        if (l == listener) return;
    }
    
    listeners_.emplace_back(listener);
}

void MelissaModel::removeListener(MelissaModelListener* listener)
{
    for (size_t listener_i = 0; listener_i < listeners_.size(); ++listener_i)
    {
        if (listeners_[listener_i] == listener)
        {
            listeners_.erase(listeners_.begin(), listeners_.begin() + 1);
            return;
        }
    }
}

MelissaModel* MelissaModel::getInstance()
{
    return &instance_;
}

MelissaModel::MelissaModel() :
playbackStatus_(kPlaybackStatus_Stop), metronomeSwitch_(false), lengthMSec_(-1), musicVolume_(1.f), metronomeVolume_(1.f), musicMetronomeBalance_(0.5f), semitone_(0),
speed_(100), currentSpeed_(100), speedIncStart_(70), speedIncValue_(1), speedIncPer_(10), speedIncGoal_(100), aPosRatio_(0.f), bPosRatio_(1.f), playingPosRatio_(0.f),
bpm_(-1), beatPositionMSec_(0.f), accent_(4), filePath_(""), outputMode_(kOutputMode_LR), eqSwitch_(false), eqFreq_(500), eqGain_(0.f), eqQ_(0.f)
{
}
