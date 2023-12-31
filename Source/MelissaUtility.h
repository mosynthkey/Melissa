//
//  MelissaUtility.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include "../JuceLibraryCode/JuceHeader.h"

class MelissaUtility
{
public:
    static std::string getFormattedTimeMSec(float msec)
    {
        const int32_t minute = msec / (60.f * 1000.f);
        msec -= minute * 60 * 1000;
        const int32_t sec = msec / 1000.f;
        msec -= sec * 1000;
        
        std::stringstream ss;
        ss << minute << ":" << std::setw(2) << std::setfill('0') << sec << "." << std::setw(1) << std::setfill('0') << static_cast<int32_t>(msec / 100.f);
        
        return ss.str();
    }
    
    static std::string getFormattedTimeSec(float sec)
    {
        const int32_t minute = sec / 60.f;
        sec -= static_cast<int32_t>(60 * minute);
        
        std::stringstream ss;
        ss << minute << ":" << std::setw(2) << std::setfill('0') << static_cast<int32_t>(sec);
        
        return ss.str();
    }
    
    static std::string getFormattedPitch(float pitch)
    {
        const int pitchInt = static_cast<int>(std::round(pitch * 10));
        const int sign = (pitchInt > 0) ? 1 : (pitchInt == 0 ? 0 : -1);
        
        const int pitchFixed = std::abs(pitchInt) / 10;
        const int pitchFrac  = std::abs(pitchInt) % 10;
        
        const std::string fracString = (pitchFrac == 0) ? "" : ("." + std::to_string(pitchFrac));
        
        if (sign == 0)
        {
#ifdef JUCE_IOS
            return "0";
#else
            return "Original";
#endif
        }
        else if (sign > 0)
        {
            return "# " + std::to_string(pitchFixed) + fracString;
        }
        else
        {
            return "b " + std::to_string(pitchFixed) + fracString;
        }
    }
    
    static juce::StringArray splitString(const juce::String& string)
    {
        juce::StringArray array;
        array.addTokens(string, "\n", "\"");
        return array;
    }
    
    static std::pair<int, int> getStringSize(const juce::Font& font, const juce::String& string)
    {
        int maxWidth = 0;
        const auto stringArray = MelissaUtility::splitString(string);
        for (int i = 0; i < stringArray.size(); ++i)
        {
            const auto width = font.getStringWidth(stringArray[i]);
            maxWidth = fmax(maxWidth, width);
        }
        return std::make_pair(maxWidth + 10, stringArray.size() * font.getHeight());
    };
    
    static void fillRoundRectangle(juce::Graphics& g, int x0, int y0, int w, int h, int r0, int r1, int r2, int r3)
    {
        const int x1 = x0 + r0;
        const int x2 = (x0 + w) - r1;
        const int x3 = (x0 + w) - r2;
        const int x4 = x0 + r3;
        const int y1 = y0 + r0;
        const int y2 = y0 + r1;
        const int y3 = y0 + h - r2;
        const int y4 = y0 + h - r3;
        
        juce::Path path;
        path.startNewSubPath(x1, y0);
        path.lineTo(x2, y0);
        path.addArc(x2 - r1, y2 - r1, r1 * 2, r1 * 2, 0, M_PI / 2);
        path.lineTo(x0 + w, y3);
        path.addArc(x3 - r2, y0 + h - r2 * 2, r2 * 2, r2 * 2, M_PI / 2, M_PI);
        path.lineTo(x4, y0 + h);
        path.addArc(x0, y4 - r3, r3 * 2, r3 * 2, M_PI, M_PI / 2 * 3);
        path.lineTo(x0, y1);
        path.addArc(x0, y0, r0 * 2, r0 * 2, M_PI / 2 * 3, M_PI * 2);
        g.fillPath(path);
    }
};
