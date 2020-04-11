//
//  MelissaBPMDetector.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BPMDetect.h"
#include "MelissaAudioEngine.h"
#include "MelissaDataSource.h"

class MelissaBPMDetector : public AsyncUpdater
{
public:
    MelissaBPMDetector();
    void start();
    
private:
    // AsyncUpdater
    void handleAsyncUpdate() override;
    
    std::unique_ptr<soundtouch::BPMDetect> bpmDetect_;
    MelissaDataSource* dataSource_;
};
