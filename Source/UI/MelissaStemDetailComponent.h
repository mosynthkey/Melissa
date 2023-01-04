//
//  MelissaStemDetailComponent.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaButtons.h"
#include "MelissaModelListener.h"

class MelissaStemDetailComponent : public Component, public MelissaModelListener
{
public:
    MelissaStemDetailComponent();
    ~MelissaStemDetailComponent() {}
    void resized() override;
    void paint(Graphics& g) override;    
    enum { kNumParts = 5 };

private:
    std::unique_ptr<MelissaRoundButton> resetButton_;
    std::unique_ptr<Slider> partKnobs_[kNumParts];
    int xCenterList_[kNumParts];
    int labelWidth_;
    void updatePosition();
    void customPartVolumeChanged(CustomPartVolume part, float volume) override;
};
