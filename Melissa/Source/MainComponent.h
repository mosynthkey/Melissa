#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaControlComponent.h"

class MelissaRoundButton;
class MelissaPlayButton;

class MainComponent : public Component
{
public:
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    
    std::unique_ptr<MelissaControlComponent> controlComponent_;
    std::unique_ptr<MelissaRoundButton> button_;
    std::unique_ptr<MelissaPlayButton> playButton_;
};
