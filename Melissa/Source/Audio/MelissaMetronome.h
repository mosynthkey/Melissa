//
//  MelissaMetronome.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include "MelissaModelListener.h"

class MelissaMetronome : public MelissaModelListener
{
public:
    MelissaMetronome();
    void render(float* bufferToRender[], const std::vector<float>& timeIndicesMSec, size_t bufferLength);
    void setOutputSampleRate(int32_t sampleRate) { sampleRate_ = sampleRate; };
    
    // MelissaModelListener
    void playbackStatusChanged(PlaybackStatus status) override;
    void metronomeSwitchChanged(bool on) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void accentChanged(int accent) override;
    void metronomeVolumeChanged(float volume) override;
    void musicMetronomeBalanceChanged(float balance) override;
    
private:
    struct Metronome
    {
        Metronome() : on_(false), volume_(1.f), beatPositionMSec_(0.f), bpm_(120.f), accent_(4), prevBeatSection_(0), prevAccentBeatSection_(0), amp_(0.f), osc_(-1.f), pitch_(880) { }
        bool on_;
        float volume_;
        float beatPositionMSec_;
        float bpm_;
        int accent_;
        int prevBeatSection_;
        int prevAccentBeatSection_;
        
        // for beep
        float amp_;
        float osc_;
        float pitch_;
    } metronome_;
    
    bool isMusicPlaying_;
    int32_t sampleRate_;
    float volumeBalance_;
    float timeMSec_;
};
