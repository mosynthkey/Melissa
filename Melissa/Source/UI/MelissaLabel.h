//
//  MelissaLabel.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaLabel : public Component, public SettableTooltipClient
{
public:
    MelissaLabel() :
    isMouseIn_(false)
    {
        label_ = std::make_unique<Label>();
        label_->setJustificationType(Justification::centred);
        label_->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub)));
        label_->setColour(Label::textColourId, Colour(0xddffffff));
        label_->setInterceptsMouseClicks(false, true);
        addAndMakeVisible(label_.get());
    }
    
    void resized() override
    {
        label_->setBounds(0, 0, getWidth(), getHeight());
    }
    
    void paint(Graphics& g) override
    {
        constexpr float t = 1.f; // thickness
        const auto b = getLocalBounds().reduced(t, t);
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, isMouseIn_ ? 0.6f : 0.4f));
        g.drawRoundedRectangle(b.toFloat(), b.getHeight() / 2, t);
    }
    
    void mouseEnter(const MouseEvent& e) override
    {
        isMouseIn_ = true;
        repaint();
    }
    
    void mouseExit(const MouseEvent& e) override
    {
        isMouseIn_ = false;
        repaint();
    }
    
    void setText(String str)
    {
        label_->setText(str, dontSendNotification);
    }
    
private:
    std::unique_ptr<Label> label_;
    bool isMouseIn_;
};
