//
//  MelissaSpeedTrainingProgressComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaModel.h"
#include "MelissaModelListener.h"

class MelissaSpeedTrainingProgressComponent : public Component, public MelissaModelListener, public Timer
{
public:
    MelissaSpeedTrainingProgressComponent();
    
    void setFont(const Font& font) { font_ = font; }
    
    // Component
    void paint(Graphics& g) override;
    
    // MelissaModelListener
    void speedIncStartChanged(int speedIncStart) override;
    void speedIncGoalChanged(int speedIncGoal) override;
    
    // Timer
    void timerCallback() override;
    
private:
    MelissaModel* model_;
    Font font_;
    float prevPlayingPosRatio_;
};
