//
//  MelissaBottomControlComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaUISettings.h"
#include "MelissaHost.h"

class UpdateButtonLookAndFeel : public LookAndFeel_V4
{
public:
    UpdateButtonLookAndFeel() {}
    virtual ~UpdateButtonLookAndFeel() {}
    
    void drawButtonBackground(Graphics& g, Button& b, const Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {}
    
    void drawButtonText(Graphics& g, TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(MelissaUISettings::getTextColour(highlighted ? 0.8f : 0.4f));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Small));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::right);
    }
};

class MelissaBottomControlComponent : public Component,
                                      public Timer
{
public:
    MelissaBottomControlComponent();
    ~MelissaBottomControlComponent() { updateButton_->setLookAndFeel(nullptr); }
    
    // Component
    void paint(Graphics& g) override;
    void resized() override;
    
    // Timer (check the latest version asynchronously)
    void timerCallback() override;
    
private:
    std::unique_ptr<TextButton> updateButton_;
    UpdateButtonLookAndFeel laf_;
};
