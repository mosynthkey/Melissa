//
//  MelissaStretcher.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include <cstdint>

class IMelissaStretcher
{
public:
    virtual ~IMelissaStretcher() = default;

    virtual void setSampleRate(int32_t sampleRate) = 0;
    virtual void setChannels(int32_t numChannels) = 0;
    virtual void setTempo(float tempo) = 0;  // 1.0 = normal speed
    virtual void setPitch(float pitch) = 0;  // 1.0 = no pitch shift

    virtual void putSamples(const float* samples, int32_t numFrames) = 0;
    virtual uint32_t receiveSamples(float* outBuffer, int32_t maxFrames) = 0;

    virtual void clear() = 0;

    // Called by the engine just before putSamples() to inform the stretcher
    // which source frame index this batch starts at. Grain-based stretchers
    // (Bungee) use this to build an accurate per-output-frame position map.
    virtual void notifyInputStart(int64_t sourceFrameIndex) {}

    // Optional accurate per-frame source positions. If the stretcher can
    // provide them (grain-based), it fills positions[] and returns true.
    // The engine then uses these instead of SampleIndexStretcher estimates.
    // Returns false when not supported (SoundTouch path is unchanged).
    virtual bool popSourcePositions(float* positions, int32_t numFrames)
    {
        return false;
    }
};
