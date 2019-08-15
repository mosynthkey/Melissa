#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaControlComponent.h"

class MelissaRoundButton;
class MelissaPlayButton;

class MainComponent   : public AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (Graphics& g) override;
    void resized() override;

private:
    std::unique_ptr<MelissaControlComponent> controlComponent_;
    std::unique_ptr<MelissaRoundButton> button_;
    std::unique_ptr<MelissaPlayButton> playButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
