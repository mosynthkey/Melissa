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

class MelissaSpeedTrainingProgressComponent : public juce::Component, public MelissaModelListener, public juce::Timer
{
public:
    MelissaSpeedTrainingProgressComponent();
    
    void setFont(const juce::Font& font) { font_ = font; }
    
    // Component
    void paint(Graphics& g) override;
    
    // MelissaModelListener
    void speedIncStartChanged(int speedIncStart) override;
    void speedIncGoalChanged(int speedIncGoal) override;
    
    // juce::Timer
    void timerCallback() override;
    
private:
    MelissaModel* model_;
    juce::Font font_;
    float prevPlayingPosRatio_;
};
