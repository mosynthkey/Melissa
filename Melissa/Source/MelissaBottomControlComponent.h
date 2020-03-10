#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaColourScheme.h"
#include "MelissaHost.h"
#include "MelissaSimpleTextButton.h"

class MelissaBottomControlComponent : public Component
{
public:
    MelissaBottomControlComponent(MelissaHost* host);
    void setTooltipText(const String& tooltipText);
    
    // Component
    void paint(Graphics& g) override;
    void resized() override;
    
public:
    MelissaHost* host_;
    std::unique_ptr<Label> tooltipLabel_;
    std::unique_ptr<MelissaSimpleTextButton> preferencesButton_;
};
