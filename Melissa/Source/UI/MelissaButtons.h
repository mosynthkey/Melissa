#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaModel.h"
#include "MelissaModelListener.h"
#include "MelissaUISettings.h"

class MelissaMenuButton : public Button
{
public:
    MelissaMenuButton() : Button("") { }
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

class MelissaAddButton : public Button
{
public:
    MelissaAddButton() : Button("") { }
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        constexpr int lineThickness = 2;
        
        const bool highlighed = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(highlighed ? 1.f : 0.6f));
        
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

class MelissaPlayPauseButton : public Button, public MelissaModelListener
{
public:
    MelissaPlayPauseButton(const String& name = "") :
    Button(name), drawPlayIcon_(true)
    {
        MelissaModel::getInstance()->addListener(this);
    }
    
    void playbackStatusChanged(PlaybackStatus status) override
    {
        drawPlayIcon_ = (status != kPlaybackStatus_Playing);
        repaint();
    }
    
private:
    void paintButton (Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& b = getLocalBounds();
        
        constexpr int t = 2; // thickness
        const int w = b.getWidth();
        const int h = b.getHeight();
        const int triW = w * 9.f / 23.f;
        const int triH = h * 3.f / 7.f;
        
        const bool on = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, on ? 0.8f : 0.4f));
        g.drawEllipse(t / 2, t / 2, w - t - 1, h - t - 1, t);
        
        if (drawPlayIcon_)
        {
            const int x0 = (w - triW) * 4.f / 7.f;
            const int y0 = (h - triH) / 2;
            
            Path path;
            path.addTriangle (x0, y0, x0, y0 + triH, x0 + triW, h / 2);
            g.fillPath(path);
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
};

class CloseButton : public Button
{
public:
    CloseButton() : Button("")  { }
    ~CloseButton() { }
    
    void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        
        const int size = getWidth();
        g.setColour(Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(0, 0, size, size, size / 5);
        
        if (highlighted)
        {
            g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(0.2f));
            g.fillRoundedRectangle(0, 0, size, size, size / 6);
        }
        
        const int margin = 4;
        g.setColour(Colours::white.withAlpha(0.8f));
        g.drawLine(margin, margin, size - margin, size - margin);
        g.drawLine(margin, size - margin, size - margin, margin);
        
    }
};

class BackgroundButton : public Button
{
public:
    BackgroundButton() : Button("")  { }
    ~BackgroundButton() { }
    void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override { }
};
