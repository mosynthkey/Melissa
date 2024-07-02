//
//  MelissaMobileSupport.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaMobileSupport
{
public:
    MelissaMobileSupport() {}

    static juce::File importFile(const juce::URL &fileUrl);
    static juce::Array<juce::File> getFileList();
};

#ifdef ENABLE_MOBILEAD
class MelissaAdComponent : public juce::Component
{
public:
    MelissaAdComponent();
    ~MelissaAdComponent();
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
private:
    juce::UIViewComponent uiViewComponent;
};
#endif
