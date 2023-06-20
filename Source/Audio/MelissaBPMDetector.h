//
//  MelissaBPMDetector.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BPMDetect_for_Melissa.h"
#include "MelissaAudioEngine.h"
#include "MelissaDataSource.h"

class MelissaBPMDetector
{
public:
    MelissaBPMDetector();
    void initialize(int sampleRate, size_t bufferLength);
    void process(bool* processFinished, float* bpm, float* beatPosMSec);
    
private:    
    std::unique_ptr<soundtouch::BPMDetect> bpmDetect_;
    MelissaDataSource* dataSource_;
    int sampleRate_;
    size_t bufferLength_;
    size_t processStartIndex_;
};
