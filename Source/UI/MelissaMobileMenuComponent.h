//
//  MelissaMobileMenuComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaMobileMenuComponent : public juce::Component
{
public:
    MelissaMobileMenuComponent();
    ~MelissaMobileMenuComponent();
    
private:
    void resized() override;
    void paint(juce::Graphics& g) override;
};
