#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

static constexpr float lineThickness = 1.4;

class MelissaLookAndFeel : public LookAndFeel_V4
{
public:
    MelissaLookAndFeel()
    {
        setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        setColour(ListBox::outlineColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        setColour(ListBox::backgroundColourId, Colours::transparentWhite);
        setColour(DirectoryContentsDisplayComponent::highlightColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.2f));
    }
    
    virtual ~MelissaLookAndFeel() {};
    
    void drawButtonBackground(Graphics& g, Button& b, const Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& c = b.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, shouldDrawButtonAsHighlighted ? 0.8f : 0.4f));
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
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
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
    }
    
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &s) override
    {
        if (style != Slider::LinearBar) return;
        
        const auto& c = s.getLocalBounds();
        const float xOffset = lineThickness / 2;
        const float yOffset = lineThickness / 2;
        const float w = c.getWidth() - lineThickness - 1;
        const float h = c.getHeight() - lineThickness - 1;
        const float cornerSize = (c.getHeight() - lineThickness) / 2;
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(xOffset, yOffset, w, h, cornerSize, lineThickness);
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.3f));
        g.fillRoundedRectangle(xOffset + lineThickness, yOffset + lineThickness, sliderPos - x, h - lineThickness * 2, h / 2 - lineThickness);
    }
    
    void drawLinearSliderBackground(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& s) override
    {
        
    }
    
    void drawLinearSliderThumb (Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle, Slider &) override
    {
        
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
    
    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& te) override
    {
        const auto& c = te.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
    }
     
    void drawComboBox(Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& cb) override
    {
        const auto& c = cb.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
        
        constexpr int triHeight = 6;
        constexpr int triWidth = 12;
        g.drawLine(width - 10 - triWidth, (height - triHeight) / 2, width - 10 - triWidth / 2, (height + triHeight) / 2, lineThickness);
        g.drawLine(width - 10 - triWidth / 2, (height + triHeight) / 2, width - 10, (height - triHeight) / 2, lineThickness);
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
};

class MelissaLookAndFeel_Tab : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel_Tab() {};
    
    void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        float alpha = 0.2f;
        if (tb.getToggleState())
        {
            alpha = 0.8f;
        }
        else if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            alpha = 0.6f;
        }
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, alpha));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
        g.fillRect(0, tb.getHeight() - 1, tb.getWidth(), 1);
    }
};

class MelissaLookAndFeel_Memo : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel_Memo() {};
    
    void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& te) override
    {
        // no background
    }
    
    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& te) override
    {
        const auto& c = te.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRect(c, lineThickness);
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
    }
};
