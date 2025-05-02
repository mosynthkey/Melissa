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
#include "MelissaStemSeparationSelectComponent.h"

class MelissaStemControlComponent : public juce::Component,
                                    public MelissaModelListener,
                                    public MelissaStemProviderListener
{
public:
    MelissaStemControlComponent();
    ~MelissaStemControlComponent();
    
private:
    // Component
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // MelissaModelListener
    void playPartChanged(PlayPart playPart) override;
    void customPartVolumeChanged(CustomPartVolume part, float volume) override;
    
    // MelissaStemProviderListener
    void stemProviderStatusChanged(StemProviderStatus status) override;
    void stemProviderResultReported(StemProviderResult result) override;
    void stemProviderEstimatedTimeReported(float estimatedTime) override;
    void stemProviderProgressReported(float progressPercentage, const juce::String& message) override;
    
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
    
    std::unique_ptr<juce::ToggleButton> mixButton_;
    std::unique_ptr<juce::ToggleButton> soloButton_;
    
    std::unique_ptr<juce::ToggleButton> allButton_;
    std::unique_ptr<juce::ToggleButton> stemSwitchButtons_[kNumStemSoloButtons];
    std::unique_ptr<juce::TextButton> createStemsButton_;
    std::unique_ptr<MelissaStemSeparationSelectComponent> stemSeparationSelectComponent_;
    
    enum { kNumMixKnobs = 6, };
    std::unique_ptr<juce::Slider> partKnobs_[kNumMixKnobs];
    int xCenterList_[kNumMixKnobs];
    
    class ProgressBar;
    std::unique_ptr<ProgressBar> progressBar_;
    
    StemProviderStatus status_;
    
    MelissaLookAndFeel_SimpleTextButton simpleTextButtonLaf_;
    MelissaLookAndFeel_StemToggleButton stemToggleButtonLaf_;
};
