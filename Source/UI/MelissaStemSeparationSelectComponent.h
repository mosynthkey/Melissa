#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"

class MelissaStemSeparationSelectComponent : public juce::Component
{
public:    
    MelissaStemSeparationSelectComponent();
    ~MelissaStemSeparationSelectComponent() override;

    void resized() override;

private:
    enum SeparationOption
    {
        kOption_Demucs,
        kOption_Spleeter,
        kNumOptions
    };

    juce::Label explanationLabel_;
    std::unique_ptr<juce::TextButton> optionButtons_[kNumOptions];
    std::unique_ptr<juce::TextButton> cancelButton_;
};

