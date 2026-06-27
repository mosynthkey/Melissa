//
//  MelissaLookAndFeel.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <math.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

static constexpr float lineThickness = 1.4f;

class MelissaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MelissaLookAndFeel()
    {
        updateColour();
    }

    ~MelissaLookAndFeel() override {}

    virtual void updateColour()
    {
        using namespace juce;

        setColour(juce::Label::textColourId, MelissaUISettings::getTextColour());
        setColour(ComboBox::textColourId, MelissaUISettings::getTextColour());
        setColour(ListBox::backgroundColourId, Colours::transparentWhite);
        setColour(ListBox::textColourId, MelissaUISettings::getTextColour());
        setColour(DirectoryContentsDisplayComponent::highlightColourId, MelissaUISettings::getTextColour());
        setColour(TableListBox::backgroundColourId, MelissaUISettings::getMainColour());
        setColour(ListBox::outlineColourId, Colours::transparentWhite);
        setColour(TextEditor::highlightColourId, MelissaUISettings::getSubColour());

        setColour(TextEditor::textColourId, MelissaUISettings::getTextColour());
        setColour(TextEditor::highlightedTextColourId, MelissaUISettings::getTextColour());
        setColour(TextEditor::highlightColourId, MelissaUISettings::getAccentColour(0.5f));

        setColour(ToggleButton::tickDisabledColourId, MelissaUISettings::getTextColour(0.2f));
        setColour(ToggleButton::tickColourId, MelissaUISettings::getTextColour());
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& /*backgroundColour*/, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
    {
        g.setColour(juce::Colour(MelissaUISettings::getSubColour()));
        g.fillRoundedRectangle(b.getLocalBounds().toFloat(), 4.f);
    }

    virtual void drawButtonText(juce::Graphics& g, juce::TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool highlighted = (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown);
        g.setColour(MelissaUISettings::getTextColour(highlighted ? 1.f : 0.8f));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), juce::Justification::centred);
    }

    virtual void drawToggleButton(juce::Graphics& g, juce::ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        g.setColour(MelissaUISettings::getMainColour());

        float textAlpha;
        if (tb.getToggleState())
        {
            textAlpha = 1.f;

            g.setColour(MelissaUISettings::getAccentColour());
            MelissaUtility::fillRoundRectangle(g, 0, 0, 2, tb.getHeight(), 1, 1, 1, 1);
        }
        else if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            textAlpha = 0.6f;
        }
        else
        {
            textAlpha = 0.2f;
        }

        g.setColour(MelissaUISettings::getTextColour(textAlpha));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
        g.drawText(tb.getButtonText(), 4, 0, tb.getWidth() - 4, tb.getHeight(), juce::Justification::centredLeft);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/, const juce::Slider::SliderStyle style, juce::Slider &s) override
    {
        using namespace juce;

        if (style == juce::Slider::LinearHorizontal)
        {
            const auto& c = s.getLocalBounds();
            const float xOffset = lineThickness / 2;
            const float yOffset = lineThickness / 2;
            const float w = static_cast<float>(c.getWidth()) - lineThickness - 1.f;
            const float h = static_cast<float>(c.getHeight()) - lineThickness - 1.f;
            const float cornerSize = (static_cast<float>(c.getHeight()) - 12.f - lineThickness) / 2.f;

            g.setColour(MelissaUISettings::getSubColour());
            g.fillRoundedRectangle(xOffset, yOffset + 6.f, w, h - 12.f, cornerSize);

            {
                const float r0  = xOffset + lineThickness;
                const float r1  = yOffset + 7.f;
                const float rw  = (sliderPos - 1.f) - r0 - 4.f;
                const float rh  = h - 14.f;

                const float r  = rh;
                const float x0 = r0;
                const float x1 = x0 + r / 2.f;
                const float x2 = x0 + rw + 2.f;
                const float y0 = r1 + 1.f;
                const float y1 = y0 + r;

                g.setColour(MelissaUISettings::getAccentColour(s.isMouseOver() ? 1.f : 0.6f));

                Path path;
                path.addArc(x0, y0, r, r, MathConstants<float>::pi, 2.f * MathConstants<float>::pi);
                path.lineTo(x1, y0);
                path.lineTo(x2, y0);
                path.addArc(x2, y0, r, r, 0.f, MathConstants<float>::pi);
                path.lineTo(x2, y1);
                path.lineTo(x1, y1);
                path.closeSubPath();
                g.fillPath(path);

                g.setColour(MelissaUISettings::getAccentColour());
                g.fillEllipse(x2, y0, r, r);
            }
        }
        else if (style == Slider::LinearVertical)
        {
            g.setColour(MelissaUISettings::getAccentColour());
            g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(y),
                                   static_cast<float>(width), static_cast<float>(height), 6.f, lineThickness);
        }
    }

    void drawLinearSliderThumb(juce::Graphics&, int, int, int, int, float, float, float, const juce::Slider::SliderStyle, juce::Slider&) override {}


    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        using namespace juce;

        const float fx = static_cast<float>(x), fy = static_cast<float>(y);
        const float fw = static_cast<float>(width), fh = static_cast<float>(height);

        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        constexpr float thickness = 4.f;
        const auto colour = juce::Colour(MelissaUISettings::getAccentColour());

        bool centerBase = slider.getRange().getStart() < 0;

        if (centerBase)
        {
            Path path;
            path.addArc(fx, fy, fw, fh, rotaryStartAngle, rotaryEndAngle, true);
            path.addArc(fx + thickness, fy + thickness, fw - thickness * 2.f, fh - thickness * 2.f, rotaryEndAngle, rotaryStartAngle, false);
            path.closeSubPath();
            g.setColour(colour.withAlpha(0.2f));
            g.fillPath(path);

            path.clear();
            path.addArc(fx, fy, fw, fh, angle, 2.f * MathConstants<float>::pi, true);
            path.addArc(fx + thickness, fy + thickness, fw - thickness * 2.f, fh - thickness * 2.f, 2.f * MathConstants<float>::pi, angle, false);
            path.closeSubPath();
            g.setColour(colour);
            g.fillPath(path);
        }
        else
        {
            Path path;
            path.addArc(fx, fy, fw, fh, angle, rotaryEndAngle, true);
            path.addArc(fx + thickness, fy + thickness, fw - thickness * 2.f, fh - thickness * 2.f, rotaryEndAngle, angle, false);
            path.closeSubPath();
            g.setColour(colour.withAlpha(0.2f));
            g.fillPath(path);

            path.clear();
            path.addArc(fx, fy, fw, fh, rotaryStartAngle, angle, true);
            path.addArc(fx + thickness, fy + thickness, fw - thickness * 2.f, fh - thickness * 2.f, angle, rotaryStartAngle, false);
            path.closeSubPath();
            g.setColour(colour);
            g.fillPath(path);
        }

        // draw marker
        {
            constexpr float markerR = 3.f;
            const float r = fw / 2.f - thickness - markerR * 2.f;
            const float mx = (fx + fw / 2.f) + r * std::sin(angle);
            const float my = (fy + fh / 2.f) - r * std::cos(angle);
            g.setColour(colour);
            g.fillEllipse(mx - markerR, my - markerR, markerR * 2.f, markerR * 2.f);
        }
    }

    void drawPopupMenuBackground(juce::Graphics& g, int /*width*/, int /*height*/) override
    {
        g.fillAll(MelissaUISettings::getMainColour());
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& /*shortcutKeyText*/, const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/) override
    {
        using namespace juce;

        g.setColour(MelissaUISettings::getMainColour());
        g.fillRect(area);

        if (isSeparator)
        {
            g.setColour(MelissaUISettings::getTextColour(0.2f));
            const auto separatorRect = Rectangle<int>(10, area.getHeight() / 2 - 0.5f, area.getWidth() - 20, 1);
            g.fillRect(separatorRect);
        }
        else
        {
            if (isHighlighted)
            {
                g.setColour(MelissaUISettings::getSubColour());
                g.fillRect(area.reduced(2, 2));
            }

            g.setColour(MelissaUISettings::getTextColour(isActive ? 0.8f : 0.2f));
            g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
            g.drawText(text, area.reduced(25, 0), Justification::left);

            if (hasSubMenu)
            {
                constexpr int triWidth = 6;
                constexpr int triHeight = 8;

                const int x0 = area.getRight() - triWidth - 10;
                const int x1 = x0 + triWidth;
                const int y0 = area.getY() + area.getHeight() / 2 - triHeight / 2;
                const int y1 = y0 + triHeight / 2;
                const int y2 = y1 + triHeight / 2;

                Path triangle;
                triangle.addTriangle(x0, y0, x0, y2, x1, y1);
                g.fillPath(triangle);
            }

            if (isTicked)
            {
                g.setColour(MelissaUISettings::getAccentColour());
                g.fillEllipse(10, (area.getHeight() - 5) / 2, 5, 5);
            }
        }
    }

    void drawMenuBarBackground(juce::Graphics& g, int /*width*/, int /*height*/, bool isMouseOverBar, juce::MenuBarComponent&) override
    {
        if (isMouseOverBar)
        {
            g.fillAll(MelissaUISettings::getMainColour());
        }
        else
        {
            g.fillAll(juce::Colour(MelissaUISettings::getMainColour(0.8f)));
        }
    }

    juce::Font getTextButtonFont(juce::TextButton& /*tb*/, int /*buttonHeight*/) override
    {
        return juce::Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    }

    void fillTextEditorBackground(juce::Graphics& g, int /*width*/, int /*height*/, juce::TextEditor& te) override
    {
        const auto b = te.getLocalBounds().toFloat();
        g.setColour(MelissaUISettings::getSubColour());
        g.fillRoundedRectangle(b, b.getHeight() / 2.f);
    }

    void drawTextEditorOutline(juce::Graphics& g, int /*width*/, int /*height*/, juce::TextEditor& te) override
    {
        const auto b = te.getLocalBounds().toFloat();
        g.setColour(MelissaUISettings::getMainColour());
        g.drawRoundedRectangle(b, b.getHeight() / 2.f, 4.f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/, int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& cb) override
    {
        const bool enabled = cb.isEnabled();
        const auto b = cb.getLocalBounds().toFloat();

        g.setColour(MelissaUISettings::getSubColour());
        g.fillRoundedRectangle(b, b.getHeight() / 2.f);

        g.setColour(MelissaUISettings::getTextColour(enabled ? 0.5f : 0.1f));

        constexpr float triHeight = 6.f;
        constexpr float triWidth  = 12.f;
        const float fw = static_cast<float>(width), fh = static_cast<float>(height);
        g.drawLine(fw - 10.f - triWidth,  (fh - triHeight) / 2.f, fw - 10.f - triWidth / 2.f, (fh + triHeight) / 2.f, lineThickness);
        g.drawLine(fw - 10.f - triWidth / 2.f, (fh + triHeight) / 2.f, fw - 10.f, (fh - triHeight) / 2.f, lineThickness);
    }

    virtual juce::Font getComboBoxFont(juce::ComboBox &) override
    {
        return juce::Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    }

    virtual void positionComboBoxText(juce::ComboBox& cb, juce::Label& labelToPosition) override
    {
        labelToPosition.setBounds(cb.getLocalBounds().reduced(cb.getHeight() / 2, 0));
    }

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& /*scrollbar*/, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        using namespace juce;

        const bool highlight = isMouseOver || isMouseDown;

        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, highlight ? 0.6f : 0.4f));

        const float fx = static_cast<float>(x), fy = static_cast<float>(y);
        const float fw = static_cast<float>(width), fh = static_cast<float>(height);
        const float fThumb = static_cast<float>(thumbStartPosition), fSize = static_cast<float>(thumbSize);

        if (isScrollbarVertical)
            g.fillRoundedRectangle(fx, fy + fThumb, fw, fSize, fw / 2.f);
        else
            g.fillRoundedRectangle(fx + fThumb, fy, fSize, fh, fh / 2.f);
    }

    virtual void drawTableHeaderBackground(juce::Graphics& g, juce::TableHeaderComponent& c) override
    {
        g.setColour(MelissaUISettings::getMainColour());
        g.fillAll();
        g.setColour(MelissaUISettings::getSubColour());
        g.fillRect(10, c.getHeight() - 2, c.getWidth() - 20, 1);
    }

    void drawTableHeaderColumn(juce::Graphics& g, juce::TableHeaderComponent&, const juce::String& columnName, int /*columnId*/, int width, int height, bool /*isMouseOver*/, bool /*isMouseDown*/, int /*columnFlags*/) override
    {
        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(columnName, 10, 0, width - 1, height, juce::Justification::left);
    }

    virtual juce::Font getPopupMenuFont() override
    {
        return juce::Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    }

    juce::Font getMenuBarFont(juce::MenuBarComponent&, int /*itemIndex*/, const juce::String& /*itemText*/) override
    {
        return juce::Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    }

    virtual juce::Rectangle<int> getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override
    {
        using namespace juce;

        const auto font = MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub);
        const int padding = 10;
        const int lineHeight = 20;
        const auto lines = StringArray::fromLines(tipText);

        int w = 0;
        for (const auto& line : lines)
            w = jmax(w, (int)font.getStringWidthFloat(line));
        w += padding * 2;
        const int h = lineHeight * lines.size() + padding;

        return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                               screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                               w, h).constrainedWithin(parentArea);
    }

    virtual void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        using namespace juce;

        Rectangle<int> bounds(width, height);

        g.setColour(MelissaUISettings::getSubColour());
        g.fillAll();

        g.setColour(MelissaUISettings::getMainColour());
        g.fillRect(bounds.reduced(2, 2));

        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
        g.setColour(MelissaUISettings::getTextColour());

        const int padding = 10;
        const int lineHeight = 20;
        const auto lines = StringArray::fromLines(text);
        for (int i = 0; i < lines.size(); ++i)
            g.drawText(lines[i], padding, i * lineHeight + padding / 2, width - padding * 2, lineHeight, Justification::centredLeft);
    }

private:
    std::unique_ptr<juce::Drawable> goUpDirectoryIcon_, goUpDirectoryHighlightedIcon_;
};

class MelissaLookAndFeel_Tab : public juce::LookAndFeel_V4
{
public:
    ~MelissaLookAndFeel_Tab() override {}

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        g.setColour(MelissaUISettings::getMainColour());

        float textAlpha;
        if (tb.getToggleState())
        {
            MelissaUtility::fillRoundRectangle(g, 0, 0, tb.getWidth(), tb.getHeight(), 6, 6, 0, 0);
            textAlpha = 1.f;

            g.setColour(MelissaUISettings::getAccentColour());
            g.fillRect(0, tb.getHeight() - 2, tb.getWidth(), 2);
        }
        else if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            MelissaUtility::fillRoundRectangle(g, 1, 1, tb.getWidth() - 2, tb.getHeight() - 2, 6, 6, 0, 0);
            textAlpha = 0.6f;
        }
        else
        {
            MelissaUtility::fillRoundRectangle(g, 1, 1, tb.getWidth() - 2, tb.getHeight() - 2, 6, 6, 0, 0);
            textAlpha = 0.2f;
        }

        g.setColour(MelissaUISettings::getTextColour(textAlpha));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), juce::Justification::centred);
    }
};

class MelissaLookAndFeel_FileBrowser : public MelissaLookAndFeel
{
public:
    MelissaLookAndFeel_FileBrowser()
    {
        updateColour();
    }

    ~MelissaLookAndFeel_FileBrowser() override {}

    void updateColour() override
    {
        MelissaLookAndFeel::updateColour();

        setColour(juce::Label::textColourId, MelissaUISettings::getTextColour());
        setColour(juce::DirectoryContentsDisplayComponent::highlightColourId, MelissaUISettings::getAccentColour(0.4f));
        setColour(juce::DirectoryContentsDisplayComponent::textColourId, MelissaUISettings::getTextColour());
        setColour(juce::DirectoryContentsDisplayComponent::highlightedTextColourId, MelissaUISettings::getTextColour());
        setColour(juce::ComboBox::textColourId, MelissaUISettings::getTextColour());

        setColour(juce::FileBrowserComponent::currentPathBoxTextColourId, MelissaUISettings::getTextColour());
        setColour(juce::FileBrowserComponent::filenameBoxTextColourId, MelissaUISettings::getTextColour());
    }

    void drawFileBrowserRow(juce::Graphics& g, int width, int height,
                            const juce::File&, const juce::String& filename, juce::Image* icon,
                            const juce::String& /*fileSizeDescription*/,
                            const juce::String& /*fileTimeDescription*/,
                            bool isDirectory, bool isItemSelected,
                                    int /*itemIndex*/, juce::DirectoryContentsDisplayComponent& dcc) override
    {
        using namespace juce;

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

        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawFittedText (filename,
                          x, 0, width - x, height,
                          Justification::centredLeft, 1);

    }

    virtual juce::Button* createFileBrowserGoUpButton() override
    {
        using namespace juce;

        auto goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

        Path arrowPath;
        arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);

        DrawablePath arrowImage;
        arrowImage.setFill(MelissaUISettings::getTextColour(0.4f));
        arrowImage.setPath(arrowPath);

        DrawablePath arrowImageHighlighted;
        arrowImageHighlighted.setFill(MelissaUISettings::getTextColour());
        arrowImageHighlighted.setPath(arrowPath);

        goUpButton->setImages(&arrowImage, &arrowImageHighlighted);

        return goUpButton;
    }

    void layoutFileBrowserComponent(juce::FileBrowserComponent &browserComp, juce::DirectoryContentsDisplayComponent *fileListComponent, juce::FilePreviewComponent* /*previewComp*/, juce::ComboBox *currentPathBox, juce::TextEditor* /*filenameBox*/, juce::Button *goUpButton) override
    {
        const auto w = browserComp.getWidth();
        const auto h = browserComp.getHeight();

        const int goUpButtonWidth = 30;
        const int margin = 10;
        const int controlHeight = 30;
        currentPathBox->setBounds(0, 0, w - goUpButtonWidth - margin, controlHeight);
        goUpButton->setBounds(currentPathBox->getRight() + margin, 0, goUpButtonWidth, controlHeight);
        //filenameBox->setBounds(0, h - controlHeight, w, controlHeight);

        const int listHeight = h - (controlHeight + margin);
        if (auto listAsComp = dynamic_cast<juce::Component*>(fileListComponent))
        {
            listAsComp->setBounds(0, currentPathBox->getBottom() + margin, w, listHeight);
        }
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/, int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& cb) override
    {
        const bool enabled = cb.isEnabled();
        const auto b = cb.getLocalBounds().toFloat();
        g.setColour(MelissaUISettings::getMainColour());
        g.fillRoundedRectangle(b, b.getHeight() / 2.f);

        g.setColour(MelissaUISettings::getTextColour(enabled ? 0.5f : 0.1f));
        constexpr float triHeight = 6.f;
        constexpr float triWidth  = 12.f;
        const float fw = static_cast<float>(width), fh = static_cast<float>(height);
        g.drawLine(fw - 10.f - triWidth,  (fh - triHeight) / 2.f, fw - 10.f - triWidth / 2.f, (fh + triHeight) / 2.f, lineThickness);
        g.drawLine(fw - 10.f - triWidth / 2.f, (fh + triHeight) / 2.f, fw - 10.f, (fh - triHeight) / 2.f, lineThickness);
    }

    void fillTextEditorBackground(juce::Graphics&, int, int, juce::TextEditor&) override {}

    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& /*backgroundColour*/, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
    {
        g.setColour(MelissaUISettings::getMainColour());
        g.fillRoundedRectangle(b.getLocalBounds().toFloat(), 6.f);
    }
};

class MelissaLookAndFeel_Memo : public MelissaLookAndFeel
{
public:
    ~MelissaLookAndFeel_Memo() override {}

    void fillTextEditorBackground(juce::Graphics& g, int, int, juce::TextEditor&) override
    {
        g.fillAll(MelissaUISettings::getMainColour());
    }

    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}

    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& /*scrollbar*/, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        const bool highlight = isMouseOver || isMouseDown;

        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, highlight ? 0.6f : 0.4f));

        const float fx = static_cast<float>(x), fy = static_cast<float>(y);
        const float fw = static_cast<float>(width), fh = static_cast<float>(height);
        const float fThumb = static_cast<float>(thumbStartPosition), fSize = static_cast<float>(thumbSize);

        if (isScrollbarVertical)
            g.fillRoundedRectangle(fx, fy + fThumb, fw, fSize, fw / 2.f);
        else
            g.fillRoundedRectangle(fx + fThumb, fy, fSize, fh, fh / 2.f);
    }
};

class MelissaLookAndFeel_SlideToggleButton : public juce::LookAndFeel_V4
{
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool on = tb.getToggleState();
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;

        const auto colour = juce::Colour(MelissaUISettings::getAccentColour());

        auto outsideRect = tb.getLocalBounds().toFloat().reduced(lineThickness, lineThickness);
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

class MelissaLookAndFeel_CircleToggleButton : public juce::LookAndFeel_V4
{
public:
    void setFont(juce::Font font)
    {
        font_ = font;
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        using namespace juce;

        const bool on = tb.getToggleState();
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;

        const auto colour = Colour(MelissaUISettings::getAccentColour());

        constexpr int circleSize = 18;
        auto outsideRect = juce::Rectangle<int>(0, (tb.getHeight() - circleSize) / 2, circleSize, circleSize).toFloat().reduced(lineThickness, lineThickness);
        g.setColour(colour);
        g.drawRoundedRectangle(outsideRect, outsideRect.getHeight() / 2, lineThickness);

        auto insideRect = outsideRect.reduced(1, 1);
        g.setColour(colour.withAlpha(highlighted ? 0.2f : 0.f));
        g.fillRoundedRectangle(insideRect, insideRect.getHeight() / 2);

        const float textX = outsideRect.getRight() + 8.f;
        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(font_);
        g.drawText(tb.getButtonText(), static_cast<int>(textX), 0, static_cast<int>(static_cast<float>(tb.getWidth()) - textX), tb.getHeight(), juce::Justification::left);

        if (on)
        {
            auto circleRect = insideRect.reduced(2, 2);
            g.setColour(colour);
            g.fillRoundedRectangle(circleRect, circleRect.getHeight() / 2);
        }
    }

private:
    juce::Font font_;
};

class MelissaLookAndFeel_SelectorToggleButton : public juce::LookAndFeel_V4
{
public:
    ~MelissaLookAndFeel_SelectorToggleButton() override {}

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        using namespace juce;

        float alpha = 0.2f;
        if (tb.getToggleState())
        {
            alpha = 0.8f;
        }
        else if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            alpha = 0.6f;
        }
        g.setColour(MelissaUISettings::getTextColour(alpha));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(tb.getButtonText(), 8, 0, tb.getWidth() - 8, tb.getHeight(), Justification::centredLeft);

        if (tb.getToggleState())
        {
            g.setColour(MelissaUISettings::getAccentColour());
            g.fillRoundedRectangle(0.f, 0.f, 4.f, static_cast<float>(tb.getHeight()) - 4.f, 2.f);
        }
    }
};

class MelissaLookAndFeel_StemToggleButton : public juce::LookAndFeel_V4
{
public:
    ~MelissaLookAndFeel_StemToggleButton() override {}

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        using namespace juce;

        const float height = static_cast<float>(tb.getHeight());
        const auto localBounds = tb.getLocalBounds().toFloat();
        const bool highlightedOrDown = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        const auto isDark = MelissaUISettings::isDarkMode;

        if (tb.getToggleState())
        {
            g.setColour(MelissaUISettings::getAccentColour(highlightedOrDown ? 1.f : 0.6f));
            g.fillRoundedRectangle(localBounds, height / 2.f);
        }
        else if (highlightedOrDown)
        {
            g.setColour(isDark ? Colours::white.withAlpha(0.2f) : Colours::black.withAlpha(0.2f));
            g.fillRoundedRectangle(localBounds, height / 2.f);
        }
        else
        {
            g.setColour(isDark ? Colours::white.withAlpha(0.1f) : Colours::black.withAlpha(0.1f));
            g.fillRoundedRectangle(localBounds, height / 2.f);
        }

        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
    }
};

class MelissaLookAndFeel_CrossFader : public juce::LookAndFeel_V4
{
public:
    ~MelissaLookAndFeel_CrossFader() override {}

    void drawLinearSlider(juce::Graphics& g, int x, int /*y*/, int width, int /*height*/, float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/, const juce::Slider::SliderStyle style, juce::Slider &s) override
    {
        using namespace juce;

        if (style != Slider::LinearHorizontal) return;

        const auto cb = s.getLocalBounds().toFloat();
        constexpr float faderThickness = 8.f;

        g.setColour(MelissaUISettings::getSubColour());
        g.fillRoundedRectangle(0.f, (cb.getHeight() - lineThickness) / 2.f, cb.getWidth(), lineThickness, lineThickness / 2.f);

        const float ratio = (sliderPos - static_cast<float>(x)) / static_cast<float>(width);
        const float faderXPos = (cb.getWidth() - faderThickness) * ratio;
        g.setColour(MelissaUISettings::getAccentColour());
        g.fillRoundedRectangle(faderXPos, 0.f, faderThickness, cb.getHeight(), faderThickness / 2.f);
    }
};

class MelissaLookAndFeel_SimpleTextButton : public juce::LookAndFeel_V4
{
public:
    MelissaLookAndFeel_SimpleTextButton()
    {

    }

    ~MelissaLookAndFeel_SimpleTextButton() override {}

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}

    void drawButtonText(juce::Graphics& g, juce::TextButton& tb, bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) override
    {
        using namespace juce;
        g.setColour(MelissaUISettings::getTextColour(shouldDrawButtonAsHighlighted ? 0.8f : 0.4f));
        g.setFont(font_);
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
    }

    void setFont(juce::Font font) { font_ = font; }
    juce::Font getFont() const { return font_; }

private:
    juce::Font font_;
};

class MelissaLookAndFeel_SimpleTextEditor : public MelissaLookAndFeel
{
public:
    void fillTextEditorBackground(juce::Graphics&, int, int, juce::TextEditor&) override {}
    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}
};

class MelissaLookAndFeel_MenuButton : public juce::LookAndFeel_V4
{
public:
    MelissaLookAndFeel_MenuButton()
    {
    }

    virtual ~MelissaLookAndFeel_MenuButton() override { }

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& /*backgroundColour*/, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            g.setColour(MelissaUISettings::getAccentColour().withAlpha(0.8f));
            g.fillRoundedRectangle(b.getLocalBounds().toFloat(), 4.f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        using namespace juce;
        const bool highlighted = (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown);
        g.setColour(MelissaUISettings::getTextColour(highlighted ? 1.f : 0.8f));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(tb.getButtonText(), 10, 0, tb.getWidth() - 20, tb.getHeight(), Justification::left);
    }
};
