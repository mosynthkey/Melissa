//
//  MelissaSeparatorComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"

class MelissaSeparatorComponent : public juce::Component
{
public:
    void paint(juce::Graphics& g)
    {
        g.setColour(MelissaUISettings::getSubColour());
        g.fillRoundedRectangle(0.f, 0.f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 1);
    }
};
