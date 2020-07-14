//
//  MelissaMarkerMemoComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"

class MelissaMarkerMemoComponent : public Component,
                                   public MelissaDataSourceListener
{
public:
    MelissaMarkerMemoComponent();
    void setFont(Font font) { font_ = font; }
    
    // Component
    void paint(Graphics& g) override;
    
private:
    void markerUpdated() override { repaint(); }
    
    MelissaDataSource* dataSource_;
    Font font_;
};
