//
//  SoundTouchStretcher.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include "MelissaStretcher.h"
#include "SoundTouch.h"
#include <memory>

class SoundTouchStretcher : public IMelissaStretcher
{
public:
    SoundTouchStretcher()
    {
        soundTouch_ = std::make_unique<soundtouch::SoundTouch>();
    }

    void setSampleRate(int32_t sampleRate) override
    {
        soundTouch_->setSampleRate(sampleRate);
    }

    void setChannels(int32_t numChannels) override
    {
        soundTouch_->setChannels(numChannels);
    }

    void setTempo(float tempo) override
    {
        soundTouch_->setTempo(tempo);
    }

    void setPitch(float pitch) override
    {
        soundTouch_->setPitch(pitch);
    }

    void putSamples(const float* samples, int32_t numSamples) override
    {
        soundTouch_->putSamples(samples, numSamples);
    }

    uint32_t receiveSamples(float* outBuffer, int32_t maxSamples) override
    {
        return soundTouch_->receiveSamples(outBuffer, maxSamples);
    }

    void clear() override
    {
        soundTouch_->clear();
    }

private:
    std::unique_ptr<soundtouch::SoundTouch> soundTouch_;
};
