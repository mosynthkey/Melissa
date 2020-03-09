#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaColourScheme.h"

class MelissaSimpleTextButton : public Button
{
public:
    MelissaSimpleTextButton(const String& text) : Button("")
    {
        label_ = std::make_unique<Label>();
        label_->setText(text, dontSendNotification);
        label_->setJustificationType(Justification::right);
        label_->setInterceptsMouseClicks(false, true);
        addAndMakeVisible(label_.get());
    }
    
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        label_->setColour(Label::textColourId, Colours::white.withAlpha(highlighted ? 0.8f : 0.4f));
    }
    
    void resized() override
    {
        label_->setBounds(0, 0, getWidth(), getHeight());
    }
    
private:
    std::unique_ptr<Label> label_;
};
