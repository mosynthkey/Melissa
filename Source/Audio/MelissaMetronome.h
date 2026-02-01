//
//  MelissaMetronome.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include <memory>
#include "MelissaBeepGenerator.h"
#include "MelissaModelListener.h"
#include "BeatThis/MelissaBeatDetector.h"

class MelissaMetronome : public MelissaModelListener
{
public:
    MelissaMetronome();
    void render(float* bufferToRender[], size_t numOfChannels, const std::vector<float>& timeIndicesMSec, size_t bufferLength);
    void setOutputSampleRate(int32_t sampleRate) { sampleRate_ = sampleRate; beepGen_.setOutputSampleRate(sampleRate); };

    // AI Beat Result
    void setBeatResult(const MelissaBeatResult& result);
    void clearBeatResult();
    bool hasBeatResult() const { return beatResult_.isValid; }

    // MelissaModelListener
    void playbackStatusChanged(PlaybackStatus status) override;
    void metronomeSwitchChanged(bool on) override;
    void metronomeModeChanged(MetronomeMode mode) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void accentChanged(int accent) override;
    void metronomeVolumeChanged(float volume) override;
    void musicMetronomeBalanceChanged(float balance) override;

private:
    void renderNormalMode(float* bufferToRender[], size_t numOfChannels, const std::vector<float>& timeIndicesMSec, size_t bufferLength);
    void renderAIMode(float* bufferToRender[], size_t numOfChannels, const std::vector<float>& timeIndicesMSec, size_t bufferLength);

    struct Metronome
    {
        Metronome() : on_(false), mode_(kMetronomeMode_Normal), volume_(1.f), beatPositionMSec_(0.f), bpm_(120.f), accent_(4), prevBeatSection_(0), prevAccentBeatSection_(0) { }
        bool on_;
        MetronomeMode mode_;
        float volume_;
        float beatPositionMSec_;
        float bpm_;
        int accent_;
        int prevBeatSection_;
        int prevAccentBeatSection_;
    } metronome_;

    MelissaBeepGenerator beepGen_;

    bool isMusicPlaying_;
    int32_t sampleRate_;
    float volumeBalance_;
    uint32_t timeMSec_;
    float timeMSecDecimal_;

    // AI Beat Detection result
    MelissaBeatResult beatResult_;
    size_t prevBeatIndex_;
};
