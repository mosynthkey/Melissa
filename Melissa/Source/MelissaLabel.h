#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaLabel : public Component
{
public:
    MelissaLabel()
    {
        label_ = std::make_unique<Label>();
        label_->setJustificationType(Justification::centred);
        label_->setFont(Font(18));
        label_->setColour(Label::textColourId, Colour(0xccffffff));
        addAndMakeVisible(label_.get());
    }
    
    void resized() override
    {
        label_->setBounds(0, 0, getWidth(), getHeight());
    }
    
    void paint(Graphics& g) override
    {
        const auto& b = getLocalBounds();
        constexpr float t = 1.4; // thickness
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(t / 2, t / 2, b.getWidth() - t - 1, b.getHeight() - t - 1, (b.getHeight() - t) / 2, t);
    }
    
    void setText(String str)
    {
        label_->setText(str, dontSendNotification);
    }
    
private:
    std::unique_ptr<Label> label_;
};
