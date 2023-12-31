//
//  MelissaSectionComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"

class MelissaSectionComponent : public juce::Component
{
public:
    MelissaSectionComponent(const juce::String& title = "");
    
    // Compoent
    void paint(juce::Graphics& g) override;
    
private:
    juce::String title_;
};
