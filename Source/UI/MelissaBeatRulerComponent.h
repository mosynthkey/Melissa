//
//  MelissaBeatRulerComponent.h
//  Melissa
//
//  Copyright(c) 2024 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../Audio/BeatThis/MelissaBeatDetector.h"

class MelissaBeatRulerComponent : public juce::Component
{
public:
    MelissaBeatRulerComponent();
    ~MelissaBeatRulerComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setBeatResult(const MelissaBeatResult& result);
    void setAudioLength(float lengthInSeconds);
    void clearBeats();

private:
    MelissaBeatResult beatResult_;
    float audioLengthSeconds_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaBeatRulerComponent)
};
