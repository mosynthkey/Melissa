//
//  MelissaBPMDetector.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaBPMDetector.h"

MelissaBPMDetector::MelissaBPMDetector() :
bpmDetect_(nullptr),
dataSource_(MelissaDataSource::getInstance()),
sampleRate_(0),
bufferLength_(0),
processStartIndex_(0)
{
}

void MelissaBPMDetector::initialize(int sampleRate, size_t bufferLength)
{
    sampleRate_   = sampleRate;
    bufferLength_ = bufferLength;
    
    bpmDetect_ = std::make_unique<soundtouch::BPMDetect>(2, sampleRate);
    processStartIndex_ = 0;
}

void MelissaBPMDetector::process(bool* processFinished, float* bpm)
{
    *processFinished = false;
    
    constexpr size_t processLength = 512;
    constexpr size_t processBufferLength = processLength * 2 /* Stereo */;
    float buffer[processBufferLength];
    
    float lCh[processLength], rCh[processLength];
    float* audioData[] = { lCh, rCh };
    const size_t startReadIndex = processStartIndex_;
    const size_t endReadIndex = std::min(startReadIndex + processLength - 1, bufferLength_ - 1);
    const size_t numReadSample = endReadIndex - startReadIndex + 1;
    dataSource_->readBuffer(MelissaDataSource::kReader_BPM, startReadIndex, static_cast<int>(numReadSample), kPlayPart_All, audioData);
    
    for (int sampleIndex = 0; sampleIndex < numReadSample; ++sampleIndex)
    {
        buffer[sampleIndex * 2 + 0] = lCh[sampleIndex];
        buffer[sampleIndex * 2 + 1] = rCh[sampleIndex];
    }
    bpmDetect_->inputSamples(buffer, processLength);
    
    processStartIndex_ += processBufferLength;
    if (bufferLength_ <= processStartIndex_) *processFinished = true;
    if (!(*processFinished)) return;
    
    const auto beatsLength = bpmDetect_->getBeats(nullptr, nullptr, 0);
    std::vector<float> beatPosition, strength;
    beatPosition.resize(beatsLength);
    strength.resize(beatsLength);
    
    *bpm = std::round(bpmDetect_->getBpm());
}
