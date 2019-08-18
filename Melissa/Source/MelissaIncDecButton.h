#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLabel.h"

class IncDecButton : public Component
{
public:
    enum Type
    {
        kType_IncButton,
        kType_DecButton
    } type_;
    
    IncDecButton(Type type) :
    type_(type)
    {
        
    }
    
    void paint(Graphics& g) override
    {
        const int32_t lineLength = 8, lineWidth = 2;
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRect((getWidth() - lineLength) / 2, (getHeight() - lineWidth) / 2, lineLength, lineWidth);
        if (type_ == kType_IncButton)
        {
            g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
            g.drawRect((getWidth() - lineWidth) / 2, (getHeight() - lineLength) / 2, lineWidth, lineLength);
        }
    }
};

class MelissaIncDecButton : public Component
{
public:
    MelissaIncDecButton()
    {
        label_ = std::make_unique<MelissaLabel>();
        addAndMakeVisible(label_.get());
        
        decButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_DecButton);
        addAndMakeVisible(decButton_.get());
        
        incButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_IncButton);
        addAndMakeVisible(incButton_.get());
    }
    
    void resized() override
    {
        label_->setBounds(0, 0, getWidth(), getHeight());
        
        const auto size = getHeight();
        decButton_->setBounds(0, 0, size, size);
        incButton_->setBounds(getWidth() - size, 0, size, size);
    }
    
    void setText(String str)
    {
        label_->setText(str);
    }
    
private:
    std::unique_ptr<MelissaLabel> label_;
    std::unique_ptr<IncDecButton> decButton_;
    std::unique_ptr<IncDecButton> incButton_;
};
