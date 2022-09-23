//
//  MelissaStemControlComponent.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDefinitions.h"
#include "MelissaStemProvider.h"

class MelissaStemControlComponent : public Component, public MelissaStemProviderListener
{
public:
    MelissaStemControlComponent();
    
private:
    void resized() override;
    
    void stemProviderStatusChanged(StemProviderStatus status) override;
    void stemProviderResultReported(StemProviderResult result) override;
    
    std::unique_ptr<TextButton> createStemsButton_;
    std::unique_ptr<TextButton> originalButton_;
    std::unique_ptr<TextButton> stemSwitchButtons_[kNumStemTypes];
    std::unique_ptr<Label> statusLabel_;
};
