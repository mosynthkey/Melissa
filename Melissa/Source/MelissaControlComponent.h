#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

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
        g.fillAll(Colour(0x30ffffff));
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MelissaControlComponent);
};
