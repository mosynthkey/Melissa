//
//  MelissaButtons.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaModel.h"
#include "MelissaModelListener.h"
#include "MelissaUISettings.h"

class MelissaMenuButton : public juce::Button
{
public:
    MelissaMenuButton() : juce::Button(""), showBudge_(true)
    {
        menuDrawable_ = juce::Drawable::createFromImageData(BinaryData::melissa_header_svg, BinaryData::melissa_header_svgSize);
    }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        menuDrawable_->drawAt(g, 0, 0, (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown) ? 1.f : 0.8f);
        
        if (showBudge_)
        {
            g.setColour(juce::Colours::red);
            g.fillEllipse(12, 12, 16, 16);
        }
    }
    
    void setBudgeVisibility(bool show)
    {
        showBudge_ = show;
        repaint();
    }
    
private:
    bool showBudge_;
    std::unique_ptr<juce::Drawable> menuDrawable_;
};

class MelissaAddButton : public juce::Button
{
public:
    MelissaAddButton() : juce::Button("") { }
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        constexpr int lineThickness = 2;
        
        const bool highlighed = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(MelissaUISettings::getMainColour().withAlpha(highlighed ? 1.f : 0.6f));
        
        const int w = getWidth();
        const int h = getHeight();
        const int size = w;
        const int lineLength = size * 2 / 5;
        g.drawRoundedRectangle(1, 1, size - 2, size - 2, (size - 2) / 2, 1);
        g.fillRect((w - lineLength) / 2, (h - lineThickness) / 2, lineLength, lineThickness);
        const int hHalf = (lineLength - lineThickness) / 2;
        g.fillRect((w - lineThickness) / 2, (h - lineLength) / 2, lineThickness, hHalf);
        g.fillRect((w - lineThickness) / 2, (h - lineLength) / 2 + lineThickness + hHalf, lineThickness, hHalf);
    }
};

class MelissaPlayPauseButton : public juce::Button, public MelissaModelListener
{
public:
    MelissaPlayPauseButton(const juce::String& name = "") :
    juce::Button(name), drawPlayIcon_(true)
    {
        playDrawable_ = juce::Drawable::createFromImageData(BinaryData::play_button_svg, BinaryData::play_button_svgSize);
        MelissaModel::getInstance()->addListener(this);
    }
    
    void playbackStatusChanged(PlaybackStatus status) override
    {
        drawPlayIcon_ = (status != kPlaybackStatus_Playing);
        repaint();
    }
    
private:
    void paintButton (juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
#ifdef JUCE_IOS
        g.fillAll(MelissaUISettings::getMainColour());
#endif
        const auto& b = getLocalBounds();
        
        const int w = b.getWidth();
        const int h = b.getHeight();
        const int triW = w * 9.f / 23.f;
        const int triH = h * 3.f / 7.f;
        
        const bool on = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        
        g.setColour(MelissaUISettings::getTextColour(on ? 1.f : 0.8f));
        
        if (drawPlayIcon_)
        {
            playDrawable_->drawAt(g, 0, 0, on ? 1.f : 0.8f);
        }
        else
        {
            const int w0 = w / 10;
            const int l0 = h * 2 / 5;
            
            g.fillRect(w / 2 - w0 * 1.5, (h - l0) / 2, w0, l0);
            g.fillRect(w / 2 + w0 * 0.5, (h - l0) / 2, w0, l0);
        }
    }
    
    bool drawPlayIcon_;
    std::unique_ptr<juce::Drawable> playDrawable_;
};

class CloseButton : public juce::Button
{
public:
    CloseButton() : juce::Button("")  { }
    ~CloseButton() { }
    
    void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        
        const int size = getWidth();
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(0, 0, size, size, size / 5);
        
        if (highlighted)
        {
            g.setColour(MelissaUISettings::getMainColour().withAlpha(0.2f));
            g.fillRoundedRectangle(0, 0, size, size, size / 6);
        }
        
        const int margin = 4;
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawLine(margin, margin, size - margin, size - margin);
        g.drawLine(margin, size - margin, size - margin, margin);
        
    }
};

class BackgroundButton : public juce::Button
{
public:
    BackgroundButton() : juce::Button("")  { }
    ~BackgroundButton() { }
    void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override { }
};

class MelissaRoundButton : public juce::Button
{
public:
    MelissaRoundButton(const juce::String& title) : juce::Button(""), title_(title)
    {
        // default
        normalColour_ = MelissaUISettings::getAccentColour(0.6f);
        highlightedColour_ = downColour_ = MelissaUISettings::getAccentColour(1.f);
    }
    
    void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Colour colour = normalColour_;
        if (shouldDrawButtonAsDown)
        {
            colour = downColour_;
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            colour = highlightedColour_;
        }
        
        g.setColour(colour);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), getHeight() / 2);
        
        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(font_);
        g.drawText(title_, 0, 0, getWidth(), getHeight(), juce::Justification::centred);
    }

    void setFont(juce::Font font)
    {
        font_ = font;
        repaint();
    }
    
    void setColour(juce::Colour normalColour, juce::Colour highlightedColour, juce::Colour downColour)
    {
        normalColour_ = normalColour;
        highlightedColour_ = highlightedColour;
        downColour_ = downColour;
        repaint();
    }
    
private:
    juce::String title_;
    juce::Font font_;
    juce::Colour normalColour_, highlightedColour_, downColour_;
};

class MelissaAudioDeviceButton : public juce::Button
{
public:
    MelissaAudioDeviceButton() : juce::Button(""), name_()
    {
        using namespace juce;
        normal_ = Drawable::createFromImageData(BinaryData::speaker_svg, BinaryData::speaker_svgSize);
        normal_->replaceColour(Colours::white, MelissaUISettings::getTextColour(0.8f));
        
        highlighted_ = Drawable::createFromImageData(BinaryData::speaker_svg, BinaryData::speaker_svgSize);
        highlighted_->replaceColour(Colours::white, MelissaUISettings::getTextColour());
    }
    
    ~MelissaAudioDeviceButton() { }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            highlighted_->drawAt(g, 10, 10, 1.f);
        }
        else
        {
            normal_->drawAt(g, 10, 10, 1.f);
        }
        
        g.setColour(MelissaUISettings::getTextColour((shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown) ? 1.f : 0.8f));
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(name_, 30, 0, getWidth() - 30, getHeight(), juce::Justification::centred, false);
    }
    
    void setAudioDeviceName(const juce::String& name)
    {
        name_ = name;
        repaint();
    }
    
private:
    juce::String name_;
    std::unique_ptr<juce::Drawable> normal_;
    std::unique_ptr<juce::Drawable> highlighted_;
};

class MelissaIconTextButton : public juce::Button
{
public:
    MelissaIconTextButton(const void* imageData, const size_t imageNumBytes, const juce::Colour& colour, const juce::String& text)  : juce::Button("")
    {
        image_ = juce::Drawable::createFromImageData(imageData, imageNumBytes);
        image_->replaceColour(juce::Colours::black, colour);
        
        text_ = text;
    }
    
    ~MelissaIconTextButton() {}
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        constexpr int xSize = 24;
        image_->drawAt(g, (getWidth() - xSize) / 2, 0, 1.f);
        
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Small));
        g.setColour(MelissaUISettings::getTextColour());
        g.drawText(text_, 0, getHeight() - 16, getWidth(), 16, juce::Justification::centred);
    }
    
    void setText(const juce::String& text)
    {
        text_ = text;
        repaint();
    }
    
private:
    std::unique_ptr<juce::Drawable> image_;
    juce::String text_;
};

class MelissaControlButton : public juce::Button
{
public:
    MelissaControlButton()  : juce::Button("")
    {
    }
    
    ~MelissaControlButton() {}
    
    void setText(const juce::String& text)
    {
        text_ = text;
    }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool isOn = (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown);
        g.setColour(MelissaUISettings::getMainColour());
        g.fillRoundedRectangle(0.f, 0.f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 4);
        
        if (isOn)
        {
            g.setColour(MelissaUISettings::getAccentColour(0.5f));
            g.fillRoundedRectangle(0.f, 0.f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 4);
        }
        
        g.setColour(MelissaUISettings::getSubColour());
        g.drawRoundedRectangle(0.f, 0.f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 4, 4);
        
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Small));
        g.setColour(MelissaUISettings::getTextColour());
        //g.drawText(text_, getLocalBounds().reduced(10, 10), juce::Justification::centredBottom);
        g.drawMultiLineText(text_, 10, getHeight() / 2, getWidth() - 20, juce::Justification::centred);
    }
    
private:
    juce::String text_;
};
