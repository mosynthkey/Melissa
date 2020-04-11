//
//  MelissaBPMDetector.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaBPMDetector.h"

MelissaBPMDetector::MelissaBPMDetector() :
bpmDetect_(nullptr),
dataSource_(MelissaDataSource::getInstance())
{
    
}

void MelissaBPMDetector::start()
{
    cancelPendingUpdate();
    triggerAsyncUpdate();
}

void MelissaBPMDetector::handleAsyncUpdate()
{
    bpmDetect_ = std::make_unique<soundtouch::BPMDetect>(2, dataSource_->getSampleRate());
    
    const auto numOfSamples = dataSource_->getBufferLength();
    
    constexpr size_t processLength = 4096;
    constexpr size_t bufferLength = processLength * 2 /* Stereo */;
    float buffer[bufferLength];
    
    for (size_t bufferStart_i = 0; bufferStart_i < numOfSamples; bufferStart_i += bufferLength)
    {
        for (size_t buffer_i = 0; buffer_i < bufferLength && (bufferStart_i + buffer_i < numOfSamples); buffer_i += 2)
        {
            buffer[buffer_i + 0] = dataSource_->readBuffer(0, bufferStart_i + buffer_i);
            buffer[buffer_i + 1] = dataSource_->readBuffer(1, bufferStart_i + buffer_i + 1);
        }
        bpmDetect_->inputSamples(buffer, processLength);
    }
    
    const auto beatsLength = bpmDetect_->getBeats(nullptr, nullptr, 0);
    std::vector<float> beatPosition, strength;
    beatPosition.resize(beatsLength);
    strength.resize(beatsLength);
    bpmDetect_->getBeats(beatPosition.data(), strength.data(), beatsLength);
    
    size_t strengthMaxIndex = 0;
    for (size_t strength_i = 0; strength_i < strength.size(); ++strength_i)
    {
        if (strength[strength_i] > strength[strengthMaxIndex])
        {
            strengthMaxIndex = strength_i;
        }
    }
    
    auto model = MelissaModel::getInstance();
    model->setBpm(std::round(bpmDetect_->getBpm()));
    model->setBeatPositionMSec(beatPosition[strengthMaxIndex] * 1000.f);
}
