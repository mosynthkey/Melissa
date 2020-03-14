#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaColourScheme.h"

class MelissaControlComponent : public Component
{
public:
    MelissaControlComponent()
    {
        setOpaque(false);
    }
    
    ~MelissaControlComponent() {}
    
    void paint(Graphics& g) override
    {
        g.setColour(Colour(MelissaColourScheme::MainColour()).withAlpha(0.06f));
        g.fillAll();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MelissaControlComponent);
};
