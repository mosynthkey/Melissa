//
//  MelissaPopupMessageComponent.h
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaPopupMessageComponent : public juce::Component, public juce::Timer
{
public:
    MelissaPopupMessageComponent();
    ~MelissaPopupMessageComponent();
    void show(const juce::String& text);
    
private:
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    juce::String text_;
    
    juce::ComponentAnimator animator_;
};
