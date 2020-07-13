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
    
    virtual void mouseDown(float xRatio) {};
    virtual void mouseUp(float xRatio) {};
    virtual void mouseMove(float xRatio) {}
    virtual void mouseDrag(float xRatio) {}
    virtual void mouseEnter(float xRatio) {}
    virtual void mouseExit(float xRatio) {}
};

class MelissaWaveformMouseEventComponent : public Component
{
public:
    MelissaWaveformMouseEventComponent() {}
    
    void addListener(MelissaWaveformMouseEventListener* listener);
    void removeListener(MelissaWaveformMouseEventListener* listener);
    
    // Component
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    void mouseMove(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseEnter(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;
    
private:
    std::vector<MelissaWaveformMouseEventListener*> listeners_;
};
