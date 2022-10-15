//
//  MelissaStemControlComponent.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaButtons.h"
#include "MelissaDefinitions.h"
#include "MelissaLookAndFeel.h"
#include "MelissaStemProvider.h"

class MelissaStemControlComponent : public Component,
                                    public MelissaModelListener,
                                    public MelissaStemProviderListener
{
public:
    MelissaStemControlComponent();
    ~MelissaStemControlComponent();
    
private:
    // Component
    void resized() override;
    void paint(Graphics& g) override;
    
    // MelissaModelListener
    void playPartChanged(StemType playPart) override;
    
    // MelissaStemProviderListener
    void stemProviderStatusChanged(StemProviderStatus status) override;
    void stemProviderResultReported(StemProviderResult result) override;
    
    void updateAndArrangeControls();
    void toggleStems(int stemIndex);
    
    std::unique_ptr<ToggleButton> allButton_;
    std::unique_ptr<ToggleButton> stemSwitchButtons_[kNumStemTypes];
    std::unique_ptr<TextButton> createStemsButton_;
    
    StemProviderStatus status_;
    
    MelissaLookAndFeel_SimpleTextButton simpleTextButtonLaf_;
    MelissaLookAndFeel_StemToggleButton  stemToggleButtonLaf_;
};
