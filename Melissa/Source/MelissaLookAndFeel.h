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
        g.setFont(22);
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
    }
    
     void drawToggleButton(Graphics& g, ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool b = tb.getToggleState() || shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        const auto& c = tb.getLocalBounds();
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, b ? 0.8f : 0.4f));
        g.drawText(tb.getToggleState() ? "On" : "Off", 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
        g.setFont(22);
        g.drawRoundedRectangle(lineThickness / 2, lineThickness / 2, c.getWidth() - lineThickness - 1, c.getHeight() - lineThickness - 1, (c.getHeight() - lineThickness) / 2, lineThickness);
    }
    
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &s) override
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
    
    void drawLinearSliderThumb (Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle, Slider &) override
    {
        
    }
    
    void drawPopupMenuBackground (Graphics& g, int width, int height) override
    {
        g.fillAll(Colour(MelissaColourScheme::DialogBackgoundColour()));
    }
    
    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const String& text, const String &shortcutKeyText, const Drawable *icon, const Colour *textColour) override
    {
        g.setColour(Colour(MelissaColourScheme::DialogBackgoundColour()));
        g.fillRect(area);
        
        if (isHighlighted)
        {
            g.setColour(Colour(MelissaColourScheme::MainColour()).withAlpha(0.1f));
            g.fillRect(area.reduced(2, 0));
        }
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(22);
        g.drawText(text, area.reduced(10, 0), Justification::left);
    }
    
    void drawMenuBarBackground (Graphics& g, int width, int height, bool isMouseOverBar, MenuBarComponent &) override
    {
        if (isMouseOverBar)
        {
            g.fillAll(Colour(MelissaColourScheme::DialogBackgoundColour()));
        }
        else
        {
            g.fillAll(Colour(MelissaColourScheme::MainColour()).withAlpha(0.8f));
        }
    }
    
    Font getTextButtonFont(TextButton& tb, int buttonHeight) override
    {
        return Font(22);
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
    
    Font getComboBoxFont (ComboBox &) override
    {
        return Font(22);
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
        g.setColour(Colour(MelissaColourScheme::MainColour()).withAlpha(0.1f));
        g.fillAll();
    }
    
    void drawTableHeaderColumn(Graphics& g, TableHeaderComponent&, const String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override
    {
        g.setColour(Colours::white);
        g.setFont(22);
        g.drawText(columnName, 10, 0, width - 1, height, Justification::left);
    }
    
    void drawFileBrowserRow(Graphics& g, int width, int height,
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

        g.setFont(22);
        g.drawFittedText (filename,
                          x, 0, width - x, height,
                          Justification::centredLeft, 1);

    }
    
    Font getPopupMenuFont() override
    {
        return Font(22);
    }
    
    
    Font getMenuBarFont(MenuBarComponent&, int itemIndex, const String& itemText) override
    {
        return Font(22);
    }
    
    
    Button* createFileBrowserGoUpButton() override
    {
        auto goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

        Path arrowPath;
        arrowPath.addArrow({ 50.0f, 100.0f, 50.0f, 0.0f }, 2.0f, 100.0f, 50.0f);

        DrawablePath arrowImage;
        arrowImage.setFill(Colours::white.withAlpha(0.4f));
        arrowImage.setPath(arrowPath);

        goUpButton->setImages(&arrowImage);

        return goUpButton;
    }
    
    void layoutFileBrowserComponent(FileBrowserComponent &browserComp, DirectoryContentsDisplayComponent *fileListComponent, FilePreviewComponent *previewComp, ComboBox *currentPathBox, TextEditor *filenameBox, Button *goUpButton) override
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
        g.setFont(22);
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
