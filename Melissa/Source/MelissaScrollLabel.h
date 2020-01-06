#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <string>

class MelissaScrollLabel : public Component, public Timer
{
public:
    MelissaScrollLabel(const Font& font) : font_(font), text_(""), textWidth_(0), position_(0)
    {
    }
    
    void paint(Graphics& g) override
    {
        g.setColour(Colours::white.withAlpha(0.8f));
        g.setFont(font_);
        g.drawText(text_, position_, 0, textWidth_, getHeight(), Justification::left);
    }
    
    void setText(const std::string& text)
    {
        stopTimer();
        
        text_ = text;
        textWidth_ = font_.getStringWidth(text_);
        
        if (getWidth() < textWidth_)
        {
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
            position_ = w;
        }
        repaint();
    }
    
private:
    Font font_;
    std::string text_;
    int textWidth_;
    int position_;
};
