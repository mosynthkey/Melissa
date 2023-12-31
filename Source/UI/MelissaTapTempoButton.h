//
//  MelissaTapTempoButton.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <chrono>
#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"

class MelissaTapTempoButton : public juce::Component, public juce::Timer
{
public:
    MelissaTapTempoButton();
    void setFont(const juce::Font& font) { font_ = font; }
    void correctWithPlaybackSpeed(bool shouldCorrect) { shouldCorrect_ = shouldCorrect; }
    
    // Component
    void paint(juce::Graphics& g) override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    // juce::Timer
    void timerCallback() override;
    
private:
    std::vector<float> measuredBpms_;
    bool isActive_, shouldCorrect_;
    std::chrono::system_clock::time_point prevTimePoint_;
    juce::Font font_;
};
