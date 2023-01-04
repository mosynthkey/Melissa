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

class MelissaTapTempoButton : public Component, public Timer
{
public:
    MelissaTapTempoButton();
    void setFont(const Font& font) { font_ = font; }
    void correctWithPlaybackSpeed(bool shouldCorrect) { shouldCorrect_ = shouldCorrect; }
    
    // Component
    void paint(Graphics& g) override;
    void mouseMove(const MouseEvent &event) override;
    void mouseExit(const MouseEvent &event) override;
    void mouseDown(const MouseEvent& event) override;
    
    // Timer
    void timerCallback() override;
    
private:
    std::vector<float> measuredBpms_;
    bool isActive_, shouldCorrect_;
    std::chrono::system_clock::time_point prevTimePoint_;
    Font font_;
};
