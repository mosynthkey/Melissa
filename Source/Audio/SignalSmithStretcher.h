//
//  SignalSmithStretcher.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include "MelissaStretcher.h"
#include <signalsmith-stretch/signalsmith-stretch.h>
#include <vector>

class SignalSmithStretcher : public IMelissaStretcher
{
public:
    SignalSmithStretcher()
    {
        inputBuf_[0].reserve(kBufSize);
        inputBuf_[1].reserve(kBufSize);
        outputBuf_[0].reserve(kBufSize);
        outputBuf_[1].reserve(kBufSize);
    }

    void setSampleRate(int32_t sampleRate) override
    {
        sampleRate_ = sampleRate;
        reconfigure();
    }

    void setChannels(int32_t numChannels) override
    {
        numChannels_ = numChannels;
        reconfigure();
    }

    void setTempo(float tempo) override { tempo_ = tempo; }

    void setPitch(float pitch) override
    {
        stretch_.setTransposeFactor(pitch);
    }

    void putSamples(const float* interleaved, int32_t numFrames) override
    {
        for (int i = 0; i < numFrames; ++i)
        {
            inputBuf_[0].push_back(interleaved[i * numChannels_ + 0]);
            inputBuf_[1].push_back(numChannels_ > 1 ? interleaved[i * numChannels_ + 1] : interleaved[i * numChannels_ + 0]);
        }
        pendingFrames_ += numFrames;
    }

    uint32_t receiveSamples(float* outBuffer, int32_t maxFrames) override
    {
        if (pendingFrames_ == 0 && outputFrames_ == 0)
            return 0;

        if (pendingFrames_ > 0)
        {
            const int inFrames  = static_cast<int>(pendingFrames_);
            const int outFrames = static_cast<int>(inFrames / tempo_);

            outputBuf_[0].resize(outFrames);
            outputBuf_[1].resize(outFrames);

            const float* inPtrs[2]  = { inputBuf_[0].data(),  inputBuf_[1].data()  };
            float*       outPtrs[2] = { outputBuf_[0].data(), outputBuf_[1].data() };
            stretch_.process(inPtrs, inFrames, outPtrs, outFrames);

            inputBuf_[0].clear();
            inputBuf_[1].clear();
            pendingFrames_ = 0;
            outputOffset_  = 0;
            outputFrames_  = outFrames;
        }

        const int n = std::min(static_cast<int>(outputFrames_ - outputOffset_), maxFrames);
        for (int i = 0; i < n; ++i)
        {
            outBuffer[i * numChannels_ + 0] = outputBuf_[0][outputOffset_ + i];
            if (numChannels_ > 1)
                outBuffer[i * numChannels_ + 1] = outputBuf_[1][outputOffset_ + i];
        }
        outputOffset_ += n;
        if (outputOffset_ >= outputFrames_)
            outputFrames_ = 0;

        return static_cast<uint32_t>(n);
    }

    void clear() override
    {
        stretch_.reset();
        inputBuf_[0].clear();
        inputBuf_[1].clear();
        outputBuf_[0].clear();
        outputBuf_[1].clear();
        pendingFrames_ = 0;
        outputFrames_  = 0;
        outputOffset_  = 0;
    }

    void flush() override
    {
        // no tail to flush — process() is synchronous
    }

private:
    static constexpr int kBufSize = 8192;

    void reconfigure()
    {
        stretch_.presetDefault(numChannels_, static_cast<float>(sampleRate_));
    }

    signalsmith::stretch::SignalsmithStretch<float> stretch_;

    int32_t sampleRate_   = 48000;
    int32_t numChannels_  = 2;
    float   tempo_        = 1.0f;

    std::vector<float> inputBuf_[2];
    std::vector<float> outputBuf_[2];
    int pendingFrames_ = 0;
    int outputFrames_  = 0;
    int outputOffset_  = 0;
};
