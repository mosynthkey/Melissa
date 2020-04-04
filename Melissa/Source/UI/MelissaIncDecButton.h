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
    enum ClickEvent
    {
        kClickEvent_Dec,
        kClickEvent_Inc,
        kClickEvent_Double
    };
    
    MelissaIncDecButton(int dragStep, const String& decTooltipStr = "", const String& incTooltipStr = "") :
    dragStep_(dragStep),
    prevMouseY_(0),
    diffY_(0)
    
    {
        label_ = std::make_unique<MelissaLabel>();
        label_->setInterceptsMouseClicks(false, true);
        addAndMakeVisible(label_.get());
        
        decButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_DecButton, [this](bool b) { if (onClick_ != nullptr) onClick_(kClickEvent_Dec, b); });
        decButton_->setTooltip(decTooltipStr);
        addAndMakeVisible(decButton_.get());
        
        incButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_IncButton, [this](bool b) { if (onClick_ != nullptr) onClick_(kClickEvent_Inc, b); });
        incButton_->setTooltip(incTooltipStr);
        addAndMakeVisible(incButton_.get());
    }
    
    void resized() override
    {
        label_->setBounds(0, 0, getWidth(), getHeight());
        
        const auto size = getHeight();
        decButton_->setBounds(0, 0, size, size);
        incButton_->setBounds(getWidth() - size, 0, size, size);
    }
    
    void mouseDown(const MouseEvent& event) override
    {
        prevMouseY_ = event.getMouseDownY();
    }
    
    void mouseDrag(const MouseEvent& event) override
    {
        diffY_ += (event.getScreenY() - prevMouseY_);
        prevMouseY_ = event.getScreenY();
        
        if (diffY_ > dragStep_)
        {
            diffY_ = 0;
            onClick_(kClickEvent_Dec, false);
            
        }
        else if (diffY_ < -1 * dragStep_)
        {
            diffY_ = 0;
            onClick_(kClickEvent_Inc, false);
        }
        
    }
    
    void mouseDoubleClick(const MouseEvent& event) override
    {
        onClick_(kClickEvent_Double, false);
    }
    
    void setText(String str)
    {
        label_->setText(str);
    }
    
    std::function<void(ClickEvent, bool)> onClick_;
    
private:
    std::unique_ptr<MelissaLabel> label_;
    std::unique_ptr<IncDecButton> decButton_;
    std::unique_ptr<IncDecButton> incButton_;
    
    int dragStep_;
    int prevMouseY_;
    int diffY_;
};
