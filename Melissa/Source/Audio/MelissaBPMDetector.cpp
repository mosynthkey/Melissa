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
    
    for (size_t bufferIndex = 0; bufferIndex < processBufferLength; bufferIndex += 2)
    {
        if (bufferLength_ * 2 <= processStartIndex_ + bufferIndex)
        {
            *processFinished = true;
            break;
        }
        buffer[bufferIndex + 0] = dataSource_->readBuffer(0, processStartIndex_ + bufferIndex);
        buffer[bufferIndex + 1] = dataSource_->readBuffer(1, processStartIndex_ + bufferIndex + 1);
    }
    processStartIndex_ += processBufferLength;
    bpmDetect_->inputSamples(buffer, processLength);
    
    if (!(*processFinished)) return;
    
    const auto beatsLength = bpmDetect_->getBeats(nullptr, nullptr, 0);
    std::vector<float> beatPosition, strength;
    beatPosition.resize(beatsLength);
    strength.resize(beatsLength);
    
    *bpm = std::round(bpmDetect_->getBpm());
    printf("bpm = %f\n", *bpm);
    // model->setBeatPositionMSec(beatPosition[strengthMaxIndex] * 1000.f);
    
    /*
    bpmDetect_->getBeats(beatPosition.data(), strength.data(), beatsLength);
    size_t strengthMaxIndex = 0;
    for (size_t strength_i = 0; strength_i < strength.size(); ++strength_i)
    {
        if (strength[strength_i] > strength[strengthMaxIndex])
        {
            strengthMaxIndex = strength_i;
        }
    }
     
    printf("bpm = %f\n", bpm);
    auto model = MelissaModel::getInstance();
    model->setBpm(std::round(bpm));
    model->setBeatPositionMSec(beatPosition[strengthMaxIndex] * 1000.f);
  */
}
