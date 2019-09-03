#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaLookAndFeel : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel() {};
    
    void drawButtonBackground(Graphics& g, Button& b, const Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& c = b.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, shouldDrawButtonAsHighlighted ? 0.8f : 0.4f));
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void drawButtonText(Graphics& g, TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
    }
    
     void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool b = tb.getToggleState() || shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        const auto& c = tb.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, b ? 0.8f : 0.4f));
        g.drawText(tb.getToggleState() ? "On" : "Off", 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &s) override
    {
        if (style != Slider::LinearHorizontal) return;
        
        constexpr int32_t lineSize = 10;

    }
    
    Font getTextButtonFont(TextButton& tb, int buttonHeight) override
    {
        return Font(14);
    }
    
    Font getLabelFont(Label& l) override
    {
        return Font(14);
    }
    
    void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& te) override
    {
        // no background
    }
    /*
    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& te) override
    {
        const auto& c = te.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    */
    void drawComboBox(Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& cb) override
    {
        const auto& c = cb.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void drawScrollbar(Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        const bool highlight = isMouseOver || isMouseDown;
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, highlight ? 0.6f : 0.4f));
        
        if (isScrollbarVertical)
        {
            g.fillRoundedRectangle(x, y + thumbStartPosition, width, thumbSize, width / 2);
        }
        else
        {
            g.fillRoundedRectangle(x + thumbStartPosition, y, thumbSize, height, height / 2);
        }
        
        //g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void drawTableHeaderBackground(Graphics& g, TableHeaderComponent& c) override
    {
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.2f));
        g.fillAll();
    }
    
    void drawTableHeaderColumn(Graphics& g, TableHeaderComponent&, const String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override
    {
        g.setColour(Colours::white);
        g.drawText(columnName, 10, 0, width - 1, height, Justification::left);
    }
    
private:
    const float lineThickNess_ = 1.4;
};

class MelissaLookAndFeel_Tab : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel_Tab() {};
    
    void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool b = tb.getToggleState() || shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, b ? 0.8f : 0.4f));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
        g.fillRect(0, tb.getHeight() - 1, tb.getWidth(), 1);
    }
};
