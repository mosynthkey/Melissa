//
//  MelissaTutorialComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaHost.h"

class MelissaTutorialComponent : public juce::Component
{
public:
    struct Page
    {
        Component* targetComponent_;
        juce::String explanation;
    };
    
    MelissaTutorialComponent(MelissaHost* host);
    void setPages(const std::vector<Page>& pages);
    void update();
    
    // Component
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    MelissaHost* host_;
    std::unique_ptr<juce::TextButton> skipButton_;
    std::unique_ptr<juce::TextButton> nextButton_;
    std::unique_ptr<juce::Label> explanationLabel_;
    
    juce::Rectangle<int> targetRectangle_;
    
    size_t currentPage_;
    std::vector<Page> pages_;
};
