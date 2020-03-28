#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"
#include "MelissaHost.h"

class MelissaBottomControlComponent : public Component
{
public:
    MelissaBottomControlComponent();
    
    // Component
    void paint(Graphics& g) override;
};
