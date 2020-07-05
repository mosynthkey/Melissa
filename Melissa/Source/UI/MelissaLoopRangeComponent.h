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

class MelissaLoopRangeComponent : public Component,
                                  public MelissaModelListener
{
public:
    MelissaLoopRangeComponent();
    
private:
    // Component
    void paint(Graphics& g) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    void mouseMove(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    
    // MelissaModelListener
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    
    Rectangle<float> getLoopStartEdgeRect() const;
    Rectangle<float> getLoopEndEdgeRect() const;
    
    MelissaModel* model_;
    float aRatio_, bRatio_;
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
