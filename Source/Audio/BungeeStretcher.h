//
//  BungeeStretcher.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include "MelissaRingBuffer.h"
#include "MelissaStretcher.h"
#include <bungee/Bungee.h>
#include <bungee/Stream.h>
#include <algorithm>
#include <cmath>
#include <memory>

class BungeeStretcher : public IMelissaStretcher
{
    // kSpeedMin = 20 → tempo = 0.2 → 5× stretch
    static constexpr int    kMaxInputFrames  = 4096;
    static constexpr int    kNumCh           = 2;
    static constexpr float  kMinTempo        = 0.2f;
    static constexpr int    kMaxOutputFrames =
        static_cast<int>(kMaxInputFrames / kMinTempo) + 2;  // = 20482

    // processedBufferQue_ capacity = queLength_ stereo frames
    // queLength_ = 10 * processLength_ * 2 = 81920 (defined in MelissaAudioEngine).
    // positionBuffer_ must match so process() can pre-fill without overflow.
    static constexpr size_t kPosCapacity = 10 * kMaxInputFrames * 2;  // = 81920

public:
    BungeeStretcher()
        : sampleRate_(48000), numChannels_(2), tempo_(1.0f), pitch_(1.0f)
    {
        recreateStream();
    }

    void setSampleRate(int32_t sampleRate) override
    {
        if (sampleRate_ == sampleRate) return;
        sampleRate_ = sampleRate;
        recreateStream();
    }

    void setChannels(int32_t numChannels) override
    {
        if (numChannels_ == numChannels) return;
        numChannels_ = numChannels;
        recreateStream();
    }

    void setTempo(float tempo) override { tempo_ = tempo; }
    void setPitch(float pitch) override { pitch_ = pitch; }

    // Engine calls this just before putSamples() to register the starting
    // source frame index of the next input batch.
    void notifyInputStart(int64_t sourceFrameIndex) override
    {
        currentInputStart_ = sourceFrameIndex;
    }

    // Accumulate interleaved input frames.
    void putSamples(const float* samples, int32_t numFrames) override
    {
        for (int i = 0; i < numFrames * numChannels_; ++i)
            pendingInput_.push(samples[i]);
        pendingFrameCount_ += numFrames;
    }

    // Process pending input, buffer all output internally, and drain up to
    // maxFrames from the internal FIFO.
    uint32_t receiveSamples(float* outBuffer, int32_t maxFrames) override
    {
        if (pendingFrameCount_ > 0)
            processAndBuffer();

        if (outputBuffer_.empty())
            return 0;

        const int32_t available =
            static_cast<int32_t>(outputBuffer_.size() / numChannels_);
        const int32_t n = std::min(available, maxFrames);
        for (int i = 0; i < n * numChannels_; ++i)
            outBuffer[i] = outputBuffer_.pop();
        return n;
    }

    // Accurate per-frame source positions computed from Bungee's actual
    // input/output mapping. The engine uses these instead of SampleIndexStretcher.
    bool popSourcePositions(float* positions, int32_t numFrames) override
    {
        if (positionBuffer_.size() < static_cast<size_t>(numFrames))
            return false;
        for (int i = 0; i < numFrames; ++i)
            positions[i] = positionBuffer_.pop();
        return true;
    }

    void clear() override
    {
        recreateStream();
        pendingFrameCount_  = 0;
        currentInputStart_  = 0;
        pendingInput_.clear();
        outputBuffer_.clear();
        positionBuffer_.clear();
    }

private:
    void processAndBuffer()
    {
        const int32_t inputFrames = pendingFrameCount_;

        // De-interleave
        for (int i = 0; i < inputFrames; ++i)
            for (int c = 0; c < numChannels_; ++c)
                inputCh_[c][i] = pendingInput_.pop();

        pendingFrameCount_ = 0;

        const double outputIdeal =
            static_cast<double>(inputFrames) / static_cast<double>(tempo_);

        const float* inPtrs[kNumCh]  = { inputCh_[0],  inputCh_[1]  };
        float*       outPtrs[kNumCh] = { outputCh_[0], outputCh_[1] };

        const int32_t actualOut =
            stream_->process(inPtrs, outPtrs, inputFrames, outputIdeal, pitch_);

        const double posStep = (actualOut > 0)
            ? static_cast<double>(inputFrames) / actualOut
            : 0.0;

        // Subtract Bungee's algorithmic latency so the reported source position
        // matches the audio content being output, not the input being consumed.
        // maxInputFrameCount() / 2 is the grain half-window = actual latency.
        const int32_t latency = stretcher_->maxInputFrameCount() / 2;

        for (int i = 0; i < actualOut; ++i)
        {
            for (int c = 0; c < numChannels_; ++c)
                outputBuffer_.push(outputCh_[c][i]);

            const float pos = static_cast<float>(
                std::max(0.0, currentInputStart_ + i * posStep - latency));
            positionBuffer_.push(pos);
        }

        currentInputStart_ += inputFrames;
    }

    void recreateStream()
    {
        const Bungee::SampleRates sr{ sampleRate_, sampleRate_ };
        // log2SynthesisHopAdjust: Bungee officially supports -1, 0, +1 only.
        // Non-zero values degrade audio quality per Bungee docs.
        //   0 → maxInputFrameCount ≈ 16384 → ~341 ms latency @ 48 kHz (best quality)
        //  -1 → ~170 ms latency (supported, minor quality trade-off)
        stretcher_ = std::make_unique<Bungee::Stretcher<Bungee::Basic>>(sr, numChannels_, 0);
        // Stream's maxFrameCount must be >= processLength_ (4096).
        // 8192 gives headroom for both the current batch and grain overlap.
        stream_    = std::make_unique<Bungee::Stream<Bungee::Basic>>(*stretcher_, 8192, numChannels_);
    }

    int32_t sampleRate_;
    int32_t numChannels_;
    float   tempo_;
    float   pitch_;

    std::unique_ptr<Bungee::Stretcher<Bungee::Basic>> stretcher_;
    std::unique_ptr<Bungee::Stream<Bungee::Basic>>    stream_;

    int64_t pendingFrameCount_  = 0;
    int64_t currentInputStart_  = 0;

    // Interleaved input FIFO
    MelissaRingBuffer<float, kMaxInputFrames * kNumCh> pendingInput_;

    // Interleaved audio output FIFO
    MelissaRingBuffer<float, kMaxOutputFrames * kNumCh> outputBuffer_;

    // Per-output-frame source position FIFO (one float per output frame).
    // Must be large enough to hold all frames queued in processedBufferQue_
    // before render() has a chance to drain them.
    MelissaRingBuffer<float, kPosCapacity> positionBuffer_;

    // Per-channel scratch arrays
    float inputCh_[kNumCh][kMaxInputFrames];
    float outputCh_[kNumCh][kMaxOutputFrames];
};
