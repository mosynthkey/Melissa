//
//  SignalSmithStretcher.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include "MelissaStretcher.h"
#include <memory>

// Implementation is in SignalSmithStretcher.cpp to isolate Accelerate framework
// headers from JUCE compilation units (avoids juce::Point / Carbon Point conflict).
class SignalSmithStretcher : public IMelissaStretcher
{
public:
    SignalSmithStretcher();
    ~SignalSmithStretcher() override;

    void setSampleRate(int32_t sampleRate) override;
    void setChannels(int32_t numChannels) override;
    void setTempo(float tempo) override;
    void setPitch(float pitch) override;

    void putSamples(const float* interleaved, int32_t numFrames) override;
    uint32_t receiveSamples(float* outBuffer, int32_t maxFrames) override;

    void clear() override;
    void flush() override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
