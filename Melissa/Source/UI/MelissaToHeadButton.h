//
//  MelissaToHeadButton.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaToHeadButton : public Button
{
public:
    MelissaToHeadButton(const String& name = "") :
    Button(name)
    {
        setOpaque(false);
    }
    
private:
    void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& b = getLocalBounds();
        
        const int w = b.getWidth();
        const int h = b.getHeight();
        
        const bool on = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, on ? 0.8f : 0.4f));
        
        g.fillRect(0, 0, w / 10, h);
        
        Path path;
        path.addTriangle (w, 0, w, h, w / 10, h / 2);
        g.fillPath(path);
        
    }
};

