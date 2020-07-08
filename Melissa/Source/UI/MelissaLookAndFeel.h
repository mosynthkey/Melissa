//
//  MelissaLookAndFeel.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <math.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

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
        setColour(ListBox::outlineColourId, Colours::white.withAlpha(0.2f));
    }
    
    virtual ~MelissaLookAndFeel() {};
    
    virtual void setBottomComponent(Component* bottomComponent)
    {
        bottomComponent_ = bottomComponent;
    }
    
    virtual void drawButtonBackground(Graphics& g, Button& b, const Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& c = b.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, shouldDrawButtonAsHighlighted ? 0.8f : 0.4f));
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
    }
    
    virtual void drawButtonText(Graphics& g, TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(MelissaUISettings::getFontSizeMain());
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
    }
    
    virtual void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool b = tb.getToggleState() || shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        const auto& c = tb.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, b ? 0.8f : 0.4f));
        g.drawText(tb.getToggleState() ? "On" : "Off", 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
        g.setFont(MelissaUISettings::getFontSizeMain());
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
    }
    
    virtual void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &s) override
    {
        if (style != Slider::LinearHorizontal) return;
        
        const auto& c = s.getLocalBounds();
        const float xOffset = lineThickness / 2;
        const float yOffset = lineThickness / 2;
        const float w = c.getWidth() - lineThickness - 1;
        const float h = c.getHeight() - lineThickness - 1;
        const float cornerSize = (c.getHeight() - 12 - lineThickness) / 2;
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(xOffset, yOffset + 6, w, h - 12, cornerSize, lineThickness);
        
        {
            Rectangle<int> rect(xOffset + lineThickness, yOffset + 6 + 1, (sliderPos - 1) - (xOffset + lineThickness) - 4, h - 12 - 2);
            const int r = rect.getHeight();
            const int x0 = rect.getX();
            const int x1 = x0 + r / 2;
            const int x2 = x0 + rect.getWidth() + 2;
            const int y0 = rect.getY() + 1;
            const int y1 = rect.getY() + 1 + rect.getHeight();
            
            //g.setGradientFill(ColourGradient(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f), x0, (y1 + y0) / 2, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.6f), x2 - x0, (y1 + y0) / 2, false));
            g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4));
            
            Path path;
            path.addArc(x0, y0, r, r, M_PI, 2 * M_PI);
            path.lineTo(x1, y0);
            path.lineTo(x2, y0);
            path.addArc(x2, y0, r, r, 0, M_PI);
            path.lineTo(x2, y1);
            path.lineTo(x1, y1);
            path.closeSubPath();
            g.fillPath(path);
        }
    }
    
    virtual void drawLinearSliderThumb (Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle, Slider &) override
    {
        
    }
    
    virtual void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        constexpr float thickness = 4.f;
        
        Path path;
        path.addArc(x, y, width, height, angle, rotaryEndAngle, true);
        path.lineTo(x + width / 2, y + height / 2);
        path.addArc(x + thickness, y + thickness, width - thickness * 2, height - thickness * 2, rotaryEndAngle, angle, false);
        path.closeSubPath();
        g.setColour(Colours::white.withAlpha(0.4f));
        g.fillPath(path);
        
        path.clear();
        path.addArc(x, y, width, height, rotaryStartAngle, angle, true);
        path.lineTo(x + width / 2, y + height / 2);
        path.addArc(x + thickness, y + thickness, width - thickness * 2, height - thickness * 2, angle, rotaryStartAngle, false);
        path.closeSubPath();
        g.setColour(Colour(MelissaUISettings::getAccentColour()));
        g.fillPath(path);
        
    }
    
    virtual void drawPopupMenuBackground (Graphics& g, int width, int height) override
    {
        g.fillAll(Colour(MelissaUISettings::getDialogBackgoundColour()));
    }
    
    virtual void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const String& text, const String &shortcutKeyText, const Drawable *icon, const Colour *textColour) override
    {
        g.setColour(Colour(MelissaUISettings::getDialogBackgoundColour()));
        g.fillRect(area);
        
        if (isHighlighted)
        {
            g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(0.1f));
            g.fillRect(area.reduced(2, 0));
        }
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(MelissaUISettings::getFontSizeSub());
        g.drawText(text, area.reduced(10, 0), Justification::left);
    }
    
    virtual void drawMenuBarBackground (Graphics& g, int width, int height, bool isMouseOverBar, MenuBarComponent &) override
    {
        if (isMouseOverBar)
        {
            g.fillAll(Colour(MelissaUISettings::getDialogBackgoundColour()));
        }
        else
        {
            g.fillAll(Colour(MelissaUISettings::getMainColour()).withAlpha(0.8f));
        }
    }
    
    virtual Font getTextButtonFont(TextButton& tb, int buttonHeight) override
    {
        return Font(MelissaUISettings::getFontSizeMain());
    }
    
    virtual void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& te) override
    {
        // no background
    }
    
    virtual void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& te) override
    {
        const auto& c = te.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
    }
     
    virtual void drawComboBox(Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& cb) override
    {
        const auto& c = cb.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
        
        constexpr int triHeight = 6;
        constexpr int triWidth = 12;
        g.drawLine(width - 10 - triWidth, (height - triHeight) / 2, width - 10 - triWidth / 2, (height + triHeight) / 2, lineThickness);
        g.drawLine(width - 10 - triWidth / 2, (height + triHeight) / 2, width - 10, (height - triHeight) / 2, lineThickness);
    }
    
    virtual Font getComboBoxFont(ComboBox &) override
    {
        return Font(MelissaUISettings::getFontSizeMain());
    }
    
    virtual void drawScrollbar(Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
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
    
    virtual void drawTableHeaderBackground(Graphics& g, TableHeaderComponent& c) override
    {
        g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(0.1f));
        g.fillAll();
    }
    
    virtual void drawTableHeaderColumn(Graphics& g, TableHeaderComponent&, const String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override
    {
        g.setColour(Colours::white);
        g.setFont(MelissaUISettings::getFontSizeMain());
        g.drawText(columnName, 10, 0, width - 1, height, Justification::left);
    }
    
    virtual void drawFileBrowserRow(Graphics& g, int width, int height,
                            const File&, const String& filename, Image* icon,
                            const String& fileSizeDescription,
                            const String& fileTimeDescription,
                            bool isDirectory, bool isItemSelected,
                            int /*itemIndex*/, DirectoryContentsDisplayComponent& dcc) override
    {
        auto fileListComp = dynamic_cast<Component*> (&dcc);

        if (isItemSelected)
            g.fillAll (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::highlightColourId)
                                               : findColour (DirectoryContentsDisplayComponent::highlightColourId));

        const int x = 32;
        g.setColour (Colours::black);

        if (icon != nullptr && icon->isValid())
        {
            g.drawImageWithin (*icon, 2, 2, x - 4, height - 4,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                               false);
        }
        else
        {
            if (auto* d = isDirectory ? getDefaultFolderImage()
                                      : getDefaultDocumentFileImage())
                d->drawWithin (g, Rectangle<float> (2.0f, 2.0f, x - 4.0f, height - 4.0f),
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        }

        if (isItemSelected)
            g.setColour (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::highlightedTextColourId)
                                                 : findColour (DirectoryContentsDisplayComponent::highlightedTextColourId));
        else
            g.setColour (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::textColourId)
                                                 : findColour (DirectoryContentsDisplayComponent::textColourId));

        g.setFont(MelissaUISettings::getFontSizeMain());
        g.drawFittedText (filename,
                          x, 0, width - x, height,
                          Justification::centredLeft, 1);

    }
    
    virtual Font getPopupMenuFont() override
    {
        return Font(MelissaUISettings::getFontSizeMain());
    }
    
    
    virtual Font getMenuBarFont(MenuBarComponent&, int itemIndex, const String& itemText) override
    {
        return Font(MelissaUISettings::getFontSizeMain());
    }
    
    
    virtual Button* createFileBrowserGoUpButton() override
    {
        class UpButton : public Button
        {
        public:
            UpButton() : Button("") { }
            
            void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
            {
                constexpr int lineHeight = 2;
                
                const bool highlighed = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
                g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(highlighed ? 1.f : 0.6f));
                
                const int w = getWidth();
                const int h = getHeight();
                g.fillRoundedRectangle(0, 0, w, lineHeight, lineHeight / 2);
                g.fillRoundedRectangle(0, (h - lineHeight) / 2, w, lineHeight, lineHeight / 2);
                g.fillRoundedRectangle(0, h - lineHeight, w, lineHeight, lineHeight / 2);
            }
        };

        return new UpButton();
    }
    
    virtual void layoutFileBrowserComponent(FileBrowserComponent &browserComp, DirectoryContentsDisplayComponent *fileListComponent, FilePreviewComponent *previewComp, ComboBox *currentPathBox, TextEditor *filenameBox, Button *goUpButton) override
    {
        const auto w = browserComp.getWidth();
        const auto h = browserComp.getHeight();
        
        const int goUpButtonWidth = 60;
        const int margin = 10;
        const int controlHeight = 30;
        currentPathBox->setBounds(0, 0, w - goUpButtonWidth - margin, controlHeight);
        goUpButton->setBounds(currentPathBox->getRight() + margin, 0, goUpButtonWidth, controlHeight);
        //filenameBox->setBounds(0, h - controlHeight, w, controlHeight);
        
        const int listHeight = h - (controlHeight + margin);
        if (auto listAsComp = dynamic_cast<Component*>(fileListComponent))
        {
            listAsComp->setBounds(0, currentPathBox->getBottom() + margin, w, listHeight);
        }
    }
    
    virtual Rectangle<int> getTooltipBounds(const String& tipText, juce::Point<int> screenPos, Rectangle<int> parentArea) override
    {
        if (bottomComponent_ == nullptr)
        {
            int w, h;
            std::tie(w, h) = MelissaUtility::getStringSize(Font(MelissaUISettings::getFontSizeSub()), tipText);
            w += 10;
            h += 4;
            
            return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                   screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                                   w, h).constrainedWithin(parentArea);
        }
        else
        {
            return bottomComponent_->getLocalBounds().reduced(20, 0).withWidth(bottomComponent_->getWidth() / 2);
        }
    }
    
    virtual void drawTooltip(Graphics& g, const String& text, int width, int height) override
    {
        Rectangle<int> bounds (width, height);
        
        if (bottomComponent_ == nullptr)
        {
            g.setColour(Colours::white.withAlpha(0.8f));
            g.fillRoundedRectangle(0, 0, width, height, height / 2);
            
            g.setColour(Colours::black/* Colour(MelissaUISettings::getDialogBackgoundColour())*/);
            g.fillRoundedRectangle(1, 1, width - 2, height - 2, (height - 2) / 2);
        }
        g.setGradientFill(ColourGradient(Colour(0xFF1A1A1A), 0, height, Colour(0xFF1C222B), width, 0, true));
        g.fillAll();
        g.setColour(Colours::white.withAlpha(0.8f));
        g.setFont(Font(MelissaUISettings::getFontSizeSub()));
        g.drawText(text, 0, 0, width, height, (bottomComponent_ == nullptr) ? Justification::centred : Justification::left);
    }
    
private:
    Component* bottomComponent_;
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
        g.setFont(MelissaUISettings::getFontSizeMain());
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

class MelissaLookAndFeel_SlideToggleButton : public LookAndFeel_V4
{
     void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool on = tb.getToggleState();
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        
        const auto colour = Colour(MelissaUISettings::getAccentColour());
        
        auto outsideRect = tb.getLocalBounds().reduced(lineThickness, lineThickness).toFloat();
        g.setColour(colour);
        g.drawRoundedRectangle(outsideRect, outsideRect.getHeight() / 2, lineThickness);
        
        auto insideRect = outsideRect.reduced(3, 3);
        g.setColour(colour.withAlpha((highlighted ? 0.2f : 0.1f) + (on ? 0.4f : 0.f)));
        g.fillRoundedRectangle(insideRect, insideRect.getHeight() / 2);
        
        const auto circleSize = insideRect.getHeight();
        const auto circleXPos = on ? insideRect.getRight() - circleSize : insideRect.getX();
        g.setColour(colour);
        g.fillRoundedRectangle(insideRect.withX(circleXPos).withWidth(circleSize), insideRect.getHeight() / 2);
    }
};

class MelissaLookAndFeel_CircleToggleButton : public LookAndFeel_V4
{
public:
    void setFont(Font font)
    {
        font_ = font;
    }
    
    void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool on = tb.getToggleState();
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        
        const auto colour = Colour(MelissaUISettings::getAccentColour());
        
        const int circleSize = 18;
        auto outsideRect = Rectangle<int>(0, (tb.getHeight() - circleSize) / 2, circleSize, circleSize).reduced(lineThickness, lineThickness).toFloat();
        g.setColour(colour);
        g.drawRoundedRectangle(outsideRect, outsideRect.getHeight() / 2, lineThickness);
        
        auto insideRect = outsideRect.reduced(1, 1);
        g.setColour(colour.withAlpha(highlighted ? 0.2f : 0.f));
        g.fillRoundedRectangle(insideRect, insideRect.getHeight() / 2);
        
        const auto textX = outsideRect.getRight() + 8;
        g.setColour(Colours::white);
        g.setFont(font_);
        g.drawText(tb.getButtonText(), textX, 0, tb.getWidth() - textX, tb.getHeight(), Justification::left);
        
        if (on)
        {
            auto circleRect = insideRect.reduced(2, 2);
            g.setColour(colour);
            g.fillRoundedRectangle(circleRect, circleRect.getHeight() / 2);
        }
    }
    
private:
    Font font_;
};

class MelissaLookAndFeel_SelectorToggleButton : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel_SelectorToggleButton() {};
    
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
        g.setFont(MelissaUISettings::getFontSizeMain());
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
        
        if (tb.getToggleState())
        {
            // draw triangle
            const float triSize = 10;
            const float x0 = 0;
            const float y0 = (tb.getHeight() - triSize) / 2;
            const float x1 = triSize;
            const float y1 = y0 + triSize / 2;
            const float x2 = 0;
            const float y2 = y0 + triSize;
            
            Path path;
            path.startNewSubPath(x0, y0);
            path.lineTo(x1, y1);
            path.lineTo(x2, y2);
            path.closeSubPath();
            
            g.setColour(Colour(MelissaUISettings::getAccentColour()));
            g.fillPath(path);
        }
    }
};

class MelissaLookAndFeel_CrossFader : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel_CrossFader() { }
    
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &s) override
    {
        if (style != Slider::LinearHorizontal) return;
        
        const auto& c = s.getLocalBounds();
        
        const int lineThickness = 4;
        const int faderThickness = 8;
        
        g.setColour(Colours::white.withAlpha(0.4f));
        g.fillRoundedRectangle(0, (c.getHeight() - lineThickness) / 2, c.getWidth(), lineThickness, lineThickness / 2);
        
        const float ratio = (sliderPos - x) / width;
        const float faderXPos = (c.getWidth() - faderThickness) * ratio;
        g.setColour(Colours::white.withAlpha(1.f));
        g.fillRoundedRectangle(faderXPos, 0, faderThickness, c.getHeight(), lineThickness / 2);
    }
};

class MelissaLookAndFeel_SimpleTextButton : public LookAndFeel_V4
{
public:
    MelissaLookAndFeel_SimpleTextButton(int fontSize, const Justification& justification = Justification::centred) :
    fontSize_(fontSize),
    justification_(justification)
    {
        
    }
    
    virtual ~MelissaLookAndFeel_SimpleTextButton() { }
    
    void drawButtonBackground(Graphics& g, Button& b, const Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {}
    
    void drawButtonText(Graphics& g, TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        g.setColour(Colours::white.withAlpha(shouldDrawButtonAsHighlighted ? 0.8f : 0.4f));
        g.setFont(fontSize_);
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), justification_);
    }

    int getFontSize() const { return fontSize_; }
    
private:
    int fontSize_;
    Justification justification_;
};

class MelissaLookAndFeel_SimpleTextEditor : public MelissaLookAndFeel
{
public:
    void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& te) override
    {
        // no background
    }

    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& te) override
    {
    }
};
