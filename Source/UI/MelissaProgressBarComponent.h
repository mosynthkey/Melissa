//
//  MelissaProgressBarComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"

class MelissaProgressBarComponent : public juce::Component, public juce::Timer
{
public:
    static constexpr int kFps = 30;
    MelissaProgressBarComponent() : counter_(0)
    {
        startTimerHz(kFps);
    }
    
    void timerCallback() override
    {
        ++counter_;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.setColour(MelissaUISettings::getMainColour());
        g.fillRoundedRectangle(getLocalBounds().toFloat(), getHeight() / 2.f);
        
        g.setColour(MelissaUISettings::getAccentColour(0.6f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), getHeight() / 2.f);
        
        const int barWidth = getWidth() / 2;
        const float xRatio = (sin(2 * M_PI * (counter_ / static_cast<float>(kFps)) / 3.f) + 1.f) / 2.f;
        const int x = (getWidth() - barWidth) * xRatio;
        g.setColour(MelissaUISettings::getAccentColour());
        g.fillRoundedRectangle(x, 0.f, barWidth, getHeight(), getHeight() / 2);
    }
    
private:
    int counter_;
};
