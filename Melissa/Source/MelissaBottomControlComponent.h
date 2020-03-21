#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaColourScheme.h"
#include "MelissaHost.h"
#include "MelissaSimpleTextButton.h"

class MelissaBottomControlComponent : public Component
{
public:
    MelissaBottomControlComponent();
    
    // Component
    void paint(Graphics& g) override;
};
