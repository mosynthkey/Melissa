//
//  MelissaScrollLabel.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <string>

class MelissaScrollLabel : public Component, public Timer
{
public:
    MelissaScrollLabel(const Font& font) : font_(font), text_(""), textWidth_(0), position_(0)
    {
    }
    
    void mouseDown(const MouseEvent& event) override
    {
        if (getWidth() <= textWidth_)
        {
            position_ = 0;

            startTimer(50);
        }
    }
    
    void paint(Graphics& g) override
    {
        g.setColour(Colours::white.withAlpha(0.8f));
        g.setFont(font_);
        g.drawText(text_, position_, 0, textWidth_, getHeight(), Justification::left);
    }
    
    void resized() override
    {
        setText(text_);
    }
    
    void setText(const String& text)
    {
        stopTimer();
        
        text_ = text;
        textWidth_ = font_.getStringWidth(text_);
        
        if (getWidth() <= textWidth_)
        {
            // scroll
            position_ = 0;
            textWidth_ += getWidth();
            startTimer(50);
        }
        else
        {
            // center
            position_ = (getWidth() - textWidth_) / 2;
        }
        
        repaint();
    }
    
    void timerCallback() override
    {
        const int w = getWidth();
        --position_;
        if (textWidth_ + position_ < w)
        {
            position_ = 0;
            stopTimer();
        }
        repaint();
    }
    
private:
    Font font_;
    String text_;
    int textWidth_;
    int position_;
};
