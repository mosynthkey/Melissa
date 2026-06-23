//
//  SignalSmithStretcher.cpp
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//
// NOTE: This file intentionally does NOT include JuceHeader.h.
// signalsmith-stretch pulls in Accelerate/Carbon which defines struct Point,
// conflicting with juce::Point when using namespace juce is active.
// Keeping this file free of JUCE headers avoids the ambiguity.

#include "SignalSmithStretcher.h"
#include <signalsmith-stretch/signalsmith-stretch.h>
#include <algorithm>
#include <vector>

struct SignalSmithStretcher::Impl
{
    signalsmith::stretch::SignalsmithStretch<float> stretch;

    int32_t sampleRate  = 48000;
    int32_t numChannels = 2;
    float   tempo       = 1.0f;

    std::vector<float> inputBuf[2];
    std::vector<float> outputBuf[2];
    int pendingFrames = 0;
    int outputFrames  = 0;
    int outputOffset  = 0;

    void reconfigure()
    {
        stretch.presetDefault(numChannels, static_cast<float>(sampleRate));
    }
};

SignalSmithStretcher::SignalSmithStretcher()
    : impl_(std::make_unique<Impl>())
{
    impl_->reconfigure();
}

SignalSmithStretcher::~SignalSmithStretcher() = default;

void SignalSmithStretcher::setSampleRate(int32_t sampleRate)
{
    if (impl_->sampleRate == sampleRate) return;
    impl_->sampleRate = sampleRate;
    impl_->reconfigure();
}

void SignalSmithStretcher::setChannels(int32_t numChannels)
{
    if (impl_->numChannels == numChannels) return;
    impl_->numChannels = numChannels;
    impl_->reconfigure();
}

void SignalSmithStretcher::setTempo(float tempo)
{
    impl_->tempo = tempo;
}

void SignalSmithStretcher::setPitch(float pitch)
{
    impl_->stretch.setTransposeFactor(pitch);
}

void SignalSmithStretcher::putSamples(const float* interleaved, int32_t numFrames)
{
    auto& d = *impl_;
    for (int i = 0; i < numFrames; ++i)
    {
        d.inputBuf[0].push_back(interleaved[i * d.numChannels + 0]);
        d.inputBuf[1].push_back(d.numChannels > 1
            ? interleaved[i * d.numChannels + 1]
            : interleaved[i * d.numChannels + 0]);
    }
    d.pendingFrames += numFrames;
}

uint32_t SignalSmithStretcher::receiveSamples(float* outBuffer, int32_t maxFrames)
{
    auto& d = *impl_;

    if (d.pendingFrames == 0 && d.outputFrames == 0)
        return 0;

    if (d.pendingFrames > 0)
    {
        const int inFrames  = d.pendingFrames;
        const int outFrames = static_cast<int>(inFrames / d.tempo);

        d.outputBuf[0].resize(outFrames);
        d.outputBuf[1].resize(outFrames);

        const float* inPtrs[2]  = { d.inputBuf[0].data(),  d.inputBuf[1].data()  };
        float*       outPtrs[2] = { d.outputBuf[0].data(), d.outputBuf[1].data() };
        d.stretch.process(inPtrs, inFrames, outPtrs, outFrames);

        d.inputBuf[0].clear();
        d.inputBuf[1].clear();
        d.pendingFrames = 0;
        d.outputOffset  = 0;
        d.outputFrames  = outFrames;
    }

    const int n = std::min(d.outputFrames - d.outputOffset, maxFrames);
    for (int i = 0; i < n; ++i)
    {
        outBuffer[i * d.numChannels + 0] = d.outputBuf[0][d.outputOffset + i];
        if (d.numChannels > 1)
            outBuffer[i * d.numChannels + 1] = d.outputBuf[1][d.outputOffset + i];
    }
    d.outputOffset += n;
    if (d.outputOffset >= d.outputFrames)
        d.outputFrames = 0;

    return static_cast<uint32_t>(n);
}

void SignalSmithStretcher::clear()
{
    auto& d = *impl_;
    d.stretch.reset();
    d.inputBuf[0].clear();
    d.inputBuf[1].clear();
    d.outputBuf[0].clear();
    d.outputBuf[1].clear();
    d.pendingFrames = 0;
    d.outputFrames  = 0;
    d.outputOffset  = 0;
}

void SignalSmithStretcher::flush()
{
    // no tail — process() is synchronous
}
