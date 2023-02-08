//
//  MelissaMarkerMemoComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaMarkerListener.h"

class MelissaMarkerMemoComponent : public juce::Component,
                                   public MelissaDataSourceListener
{
public:
    MelissaMarkerMemoComponent();
    void setFont(juce::Font font) { font_ = font; }
    void setMarkerListener(MelissaMarkerListener* listener) { listener_ = listener; }
    
    // Component
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
private:
    void markerUpdated() override { repaint(); }
    
    MelissaDataSource* dataSource_;
    juce::Font font_;
    MelissaMarkerListener* listener_;
    
    struct MelissaMarkerLabelInfo
    {
        int x_, width_;
        float posRatio_;
        juce::String memo_;
        juce::Colour colour_;
        bool shorten_;
        int originalWidth_;
        int markerIndex_;
    };
    std::vector<MelissaMarkerLabelInfo> markerLabelInfo_;
};
