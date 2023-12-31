//
//  MelissaWaveformMouseEventComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"

class MelissaWaveformMouseEventListener
{
public:
    virtual ~MelissaWaveformMouseEventListener() {}
    
    virtual void mouseDown(float xRatio, bool isLeft = true) {};
    virtual void mouseUp(float xRatio) {};
    virtual void mouseMove(float xRatio) {}
    virtual void mouseDrag(float xRatio) {}
    virtual void mouseEnter(float xRatio) {}
    virtual void mouseExit(float xRatio) {}
};

class MelissaWaveformMouseEventComponent : public juce::Component
{
public:
    MelissaWaveformMouseEventComponent() {}
    
    void addListener(MelissaWaveformMouseEventListener* listener);
    void removeListener(MelissaWaveformMouseEventListener* listener);
    
    // Component
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
private:
    std::vector<MelissaWaveformMouseEventListener*> listeners_;
};
