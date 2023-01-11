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
    void playPartChanged(PlayPart playPart) override;
    void customPartVolumeChanged(CustomPartVolume part, float volume) override;
    
    // MelissaStemProviderListener
    void stemProviderStatusChanged(StemProviderStatus status) override;
    void stemProviderResultReported(StemProviderResult result) override;
    void stemProviderEstimatedTimeReported(float estimatedTime) override;
    
    void updatePosition();
    void updateAndArrangeControls();
    void toggleStems(PlayPart playPart);
    enum Mode
    {
        kMode_NoStems,
        kMode_Mix,
        kMode_Solo,
    };
    Mode mode_;
    
    std::unique_ptr<ToggleButton> mixButton_;
    std::unique_ptr<ToggleButton> soloButton_;
    
    std::unique_ptr<ToggleButton> allButton_;
    std::unique_ptr<ToggleButton> stemSwitchButtons_[kNumStemSoloButtons];
    std::unique_ptr<TextButton> createStemsButton_;
    enum { kNumMixKnobs = 5, };
    std::unique_ptr<Slider> partKnobs_[kNumMixKnobs];
    int xCenterList_[kNumMixKnobs];
    
    class ProgressBar;
    std::unique_ptr<ProgressBar> progressBar_;
    
    StemProviderStatus status_;
    
    MelissaLookAndFeel_SimpleTextButton simpleTextButtonLaf_;
    MelissaLookAndFeel_StemToggleButton stemToggleButtonLaf_;
};
