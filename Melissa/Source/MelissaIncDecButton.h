#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLabel.h"

class IncDecButton : public Button
{
public:
    enum Type
    {
        kType_IncButton,
        kType_DecButton
    } type_;
    
    IncDecButton(Type type, std::function<void(bool)> onClick) :
    Button(""), type_(type), onClick_(onClick) { }
    
    void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const int32_t lineLength = 8, lineWidth = 2;
        
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, highlighted ? 0.8f : 0.4f));
        g.drawRect((getWidth() - lineLength) / 2, (getHeight() - lineWidth) / 2, lineLength, lineWidth);
        if (type_ == kType_IncButton)
        {
            const int32_t halfLength = (lineLength - lineWidth) / 2;
            const int32_t y = (getHeight() - lineLength) / 2;
            g.drawRect((getWidth() - lineWidth) / 2, y, lineWidth, halfLength);
            g.drawRect((getWidth() - lineWidth) / 2, y + halfLength + lineWidth, lineWidth, halfLength);
        }
    }
    
    void clicked(const ModifierKeys& modifiers) override
    {
        if (onClick_ != nullptr) onClick_(modifiers.isShiftDown());
    }
    
    std::function<void(bool)> onClick_;
};

class MelissaIncDecButton : public Component
{
public:
    MelissaIncDecButton()
    {
        label_ = std::make_unique<MelissaLabel>();
        addAndMakeVisible(label_.get());
        
        decButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_DecButton, [this](bool b) { if (onClick_ != nullptr) onClick_(false, b); });
        addAndMakeVisible(decButton_.get());
        
        incButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_IncButton, [this](bool b) { if (onClick_ != nullptr) onClick_(true, b); });
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
    
    std::function<void(bool, bool)> onClick_;
    
private:
    std::unique_ptr<MelissaLabel> label_;
    std::unique_ptr<IncDecButton> decButton_;
    std::unique_ptr<IncDecButton> incButton_;
};
