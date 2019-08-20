#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaPlayPauseButton : public Button
{
public:
    MelissaPlayPauseButton(const String& name = "") :
    Button(name), mode_(kMode_Play)
    {
        setOpaque(false);
    }
    
    enum Mode
    {
        kMode_Play,
        kMode_Pause
    } mode_;
    
    void setMode(Mode mode)
    {
        mode_ = mode;
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
        
        if (mode_ == kMode_Play)
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
};

