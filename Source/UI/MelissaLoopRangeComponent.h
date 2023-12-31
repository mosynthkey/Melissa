//
//  MelissaLoopRangeComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"
#include "MelissaModel.h"
#include "MelissaModelListener.h"
#include "MelissaUISettings.h"
#include "MelissaWaveformMouseEventComponent.h"

class MelissaLoopRangeComponent : public juce::Component,
                                  public MelissaModelListener,
                                  public MelissaWaveformMouseEventListener
{
public:
    MelissaLoopRangeComponent();
    
private:
    // Component
    void paint(juce::Graphics& g) override;
    
    // MelissaModelListener
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    
    // MelissaWaveformMouseEventListener
    void mouseDown(float xRatio, bool isLeft = true) override;
    void mouseUp(float xRatio) override;
    void mouseMove(float xRatio) override;
    void mouseDrag(float xRatio) override;
    
    juce::Rectangle<float> getLoopStartEdgeRect() const;
    juce::Rectangle<float> getLoopEndEdgeRect() const;
    
    MelissaModel* model_;
    float aRatio_, bRatio_;
    float mouseClickXRatio_;
    bool mouseOnLoopStartEdge_, mouseOnLoopEndEdge_;
    bool draggingLoopStart_;
    
    enum MouseStatus
    {
        kMouseStatus_None,
        kMouseStatus_Range,
        kMouseStatus_DraggingStart,
        kMouseStatus_DraggingEnd,
    };
    MouseStatus mouseStatus_;
};
