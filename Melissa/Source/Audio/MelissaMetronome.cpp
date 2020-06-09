//
//  MelissaMetronome.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <cmath>
#include "MelissaMetronome.h"
#include "MelissaModel.h"

MelissaMetronome::MelissaMetronome() :
isMusicPlaying_(false),
sampleRate_(1),
volumeBalance_(0.5f),
timeMSec_(0.f)
{
    MelissaModel::getInstance()->addListener(this);
}

void MelissaMetronome::render(float* bufferToRender[], const std::vector<float>& timeIndicesMSec, size_t bufferLength)
{
    const auto currentSpeed = MelissaModel::getInstance()->getCurrentSpeed();
    
    for (int iSample = 0; iSample < bufferLength; ++iSample)
    {
        if (isMusicPlaying_)
        {
            timeMSec_ = timeIndicesMSec[iSample];
        }
        else
        {
            timeMSec_ += (currentSpeed / 100.f) / sampleRate_ * 1000.f;
        }
        
        metronome_.osc_ += 2.f / (sampleRate_ / metronome_.pitch_);
        if (metronome_.osc_ > 1.f) metronome_.osc_ = -1.f;
        
        const auto metronomeOsc = metronome_.osc_ * metronome_.amp_ * metronome_.volume_ * (metronome_.on_ ? 1.f : 0.f) * volumeBalance_;

        bufferToRender[0][iSample] += metronomeOsc;
        bufferToRender[1][iSample] += metronomeOsc;

        const int beatSection = (timeMSec_ - metronome_.beatPositionMSec_) / ((60.f / metronome_.bpm_) * 1000.f);
        if (metronome_.accent_ != 0 && (beatSection / metronome_.accent_ != metronome_.prevBeatSection_ / metronome_.accent_))
        {
            metronome_.amp_ = 1.f;
            metronome_.pitch_ = 880 * 2;
        }
        else if (beatSection != metronome_.prevBeatSection_)
        {
            metronome_.amp_ = 1.f;
            metronome_.pitch_ = 880;
        }
        metronome_.prevBeatSection_ = beatSection;
        
        if (metronome_.amp_ > 0.f)
        {
            metronome_.amp_ -= 1.f / static_cast<float>(sampleRate_) * 20.f;
        }
        else if (metronome_.amp_ < 0.f)
        {
            metronome_.amp_ = 0.f;
        }
    }
}

void MelissaMetronome::playbackStatusChanged(PlaybackStatus status)
{
    isMusicPlaying_ = (status == kPlaybackStatus_Playing);
}

void MelissaMetronome::metronomeSwitchChanged(bool on)
{
    metronome_.on_ = on;
}

void MelissaMetronome::bpmChanged(float bpm)
{
    metronome_.bpm_ = bpm;
}

void MelissaMetronome::beatPositionChanged(float beatPositionMSec)
{
    metronome_.beatPositionMSec_ = beatPositionMSec;
}

void MelissaMetronome::accentChanged(int accent)
{
    metronome_.accent_ = accent;
}

void MelissaMetronome::metronomeVolumeChanged(float volume)
{
    metronome_.volume_ = volume;
}

void MelissaMetronome::musicMetronomeBalanceChanged(float balance)
{
    volumeBalance_ = 1.f - cos(M_PI / 2.f * balance);
}
