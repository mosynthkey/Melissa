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

class MelissaMarkerMemoComponent : public Component,
                                   public MelissaDataSourceListener
{
public:
    MelissaMarkerMemoComponent();
    void setFont(Font font) { font_ = font; }
    void setMarkerListener(MelissaMarkerListener* listener) { listener_ = listener; }
    
    // Component
    void paint(Graphics& g) override;
    void mouseDown(const MouseEvent& event) override;
    
private:
    void markerUpdated() override { repaint(); }
    
    MelissaDataSource* dataSource_;
    Font font_;
    MelissaMarkerListener* listener_;
    
    struct MelissaMarkerLabelInfo
    {
        int x_, width_;
        float posRatio_;
        String memo_;
        Colour colour_;
        bool shorten_;
        int originalWidth_;
    };
    std::vector<MelissaMarkerLabelInfo> markerLabelInfo_;
};
