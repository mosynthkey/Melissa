//
//  MelissaToolbarComponent.h
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"
#include "MelissaUISettings.h"

class MelissaToolbarComponent : public juce::Component
{
public:
    MelissaToolbarComponent(const juce::String& title = "");

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Get the bounds for content (area after title and separator)
    juce::Rectangle<int> getContentBounds() const;

private:
    juce::String title_;
    int titleWidth_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaToolbarComponent)
};
