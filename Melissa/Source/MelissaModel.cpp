#include "MelissaModel.h"

MelissaModel MelissaModel::instance_;

void MelissaModel::setLengthMSec(float lengthMSec)
{
    if (lengthMSec < 0.f) return;
    
    lengthMSec_ = lengthMSec;
}

void MelissaModel::setVolume(float volume)
{
    if (volume < 0.f || 2.f < volume) return;
    
    volume_ = volume;
    for (auto&& l : listeners_) l->volumeChanged(volume);
}

void MelissaModel::setPitch(int semitone)
{
    if (semitone < -24 || 24 < semitone) return;
    
    semitone_ = semitone;
    for (auto&& l : listeners_) l->pitchChanged(semitone);
}

void MelissaModel::setSpeed(int speed)
{
    if (speed < 20 || 200 < speed) return;
    
    speed_ = speed;
    for (auto&& l : listeners_) l->speedChanged(speed);
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
    return melissa_->getPlayingPosRatio();
}

void MelissaModel::setPlayingPosMSec(float playingPosMSec)
{
    if (playingPosMSec < 0 || lengthMSec_ < playingPosMSec || lengthMSec_ < 0.f) return;
    
    playingPosRatio_ = playingPosMSec / lengthMSec_;
    for (auto&& l : listeners_) l->playingPosChanged(playingPosMSec, playingPosMSec / lengthMSec_);
}

float MelissaModel::getPlayingPosMSec() const
{
    return melissa_->getPlayingPosMSec();
}

void MelissaModel::synchronize()
{
    setLengthMSec(lengthMSec_);
    setVolume(volume_);
    setPitch(semitone_);
    setSpeed(speed_);
    setLoopPosRatio(aPosRatio_, bPosRatio_);
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
lengthMSec_(-1), volume_(1.f), semitone_(0), speed_(100), aPosRatio_(0.f), bPosRatio_(1.f), playingPosRatio_(0.f), filePath_("")
{
    
}
