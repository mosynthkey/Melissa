//
//  MelissaIncDecButton.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

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
    enum Event
    {
        kEvent_Dec,
        kEvent_Inc,
        kEvent_Double,
        kEvent_Func,
    };
    
    enum ButtonPosition
    {
        kButtonPosition_None,
        kButtonPosition_Left,
        kButtonPosition_Right,
    };
    
    MelissaIncDecButton(int dragStep, const String& decTooltipStr = "", const String& incTooltipStr = "")  :
    funcButtonPos_(kButtonPosition_None),
    funcButtonTitle_(""),
    dragStep_(dragStep),
    prevMouseY_(0),
    diffY_(0)
    {
        label_ = std::make_unique<Label>();
        label_->setInterceptsMouseClicks(false, true);
        label_->setJustificationType(Justification::centred);
        label_->setFont(Font(MelissaUISettings::getFontSizeSub()));
        label_->setColour(Label::textColourId, Colour(0xddffffff));
        addAndMakeVisible(label_.get());
        
        decButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_DecButton, [this](bool b) { if (onClick_ != nullptr) onClick_(kEvent_Dec, b); });
        decButton_->setTooltip(decTooltipStr);
        addAndMakeVisible(decButton_.get());
        
        incButton_ = std::make_unique<IncDecButton>(IncDecButton::kType_IncButton, [this](bool b) { if (onClick_ != nullptr) onClick_(kEvent_Inc, b); });
        incButton_->setTooltip(incTooltipStr);
        addAndMakeVisible(incButton_.get());
        
        funcButton_ = nullptr;
    }
    
    void addFunctionButton(ButtonPosition funcButtonPos, const String& funcButtonTitle = "", const String& funcTooltipStr = "")
    {
        funcButtonPos_ = funcButtonPos;
        funcButtonTitle_ = funcButtonTitle;
        
        funcButton_ = std::make_unique<MelissaRoundButton>(funcButtonTitle);
        funcButton_->setTooltip(funcTooltipStr);
        funcButton_->setFont(MelissaUISettings::getFontSizeSub());
        funcButton_->onClick =  [this]() { if (onClick_ != nullptr) onClick_(kEvent_Func, false); };
        addAndMakeVisible(funcButton_.get());
    }
    
    void resized() override
    {
        const auto w = getWidth();
        const auto h = getHeight();
        const auto incDecButtonSize = h;
        const auto funcButtonWidth = 50;
        
        if (funcButtonPos_ == kButtonPosition_None)
        {
            label_->setBounds(0, 0, w, h);
        }
        else if (funcButtonPos_ == kButtonPosition_Left)
        {
            funcButton_->setBounds(Rectangle(0, 0, funcButtonWidth, h).reduced(4, 4));
            label_->setBounds(funcButtonWidth, 0, w - funcButtonWidth, h);
        }
        else
        {
            label_->setBounds(0, 0, w - funcButtonWidth, h);
            funcButton_->setBounds(Rectangle(w - funcButtonWidth, 0, funcButtonWidth, h).reduced(4, 4));
        }
        
        decButton_->setBounds(label_->getX(), 0, incDecButtonSize, incDecButtonSize);
        incButton_->setBounds(label_->getRight() - incDecButtonSize, 0, incDecButtonSize, incDecButtonSize);
    }
    
    void paint(Graphics& g) override
    {
        const auto b = getLocalBounds();
        g.setColour(MelissaUISettings::getSubColour());
        g.fillRoundedRectangle(b.toFloat(), b.getHeight() / 2);
    }
    
    void mouseDown(const MouseEvent& event) override
    {
        prevMouseY_ = event.getScreenY();
    }
    
    void mouseDrag(const MouseEvent& event) override
    {
        diffY_ += (event.getScreenY() - prevMouseY_);
        prevMouseY_ = event.getScreenY();
        
        if (diffY_ > dragStep_)
        {
            diffY_ = 0;
            onClick_(kEvent_Dec, event.mods.isShiftDown());
        }
        else if (diffY_ < -1 * dragStep_)
        {
            diffY_ = 0;
            onClick_(kEvent_Inc, event.mods.isShiftDown());
        }
    }
    
    void mouseDoubleClick(const MouseEvent& event) override
    {
        onClick_(kEvent_Double, false);
    }


    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override
    {
        onClick_((wheel.deltaY > 0) ? kEvent_Inc : kEvent_Dec, true);
    }
    
    void setText(const String& str)
    {
        label_->setText(str, dontSendNotification);
    }
    
    std::function<void(Event, bool)> onClick_;
    
private:
    std::unique_ptr<Label> label_;
    std::unique_ptr<IncDecButton> decButton_;
    std::unique_ptr<IncDecButton> incButton_;
    std::unique_ptr<MelissaRoundButton> funcButton_;
    
    ButtonPosition funcButtonPos_;
    String funcButtonTitle_;
    bool isMouseIn_;
    int dragStep_;
    int prevMouseY_;
    int diffY_;
};
