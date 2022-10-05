//
//  MelissaUISettings.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <tuple>

class MelissaUISettings
{
public:
    static Colour getTextColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFFFFFF).withAlpha(alpha) : Colour(0xFF000000).withAlpha(alpha);
    }
    
    static Colour getWaveformColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFFD9E8FF).withAlpha(alpha) : Colour(0xFF89B8FF).withAlpha(alpha);
    }
    
    static Colour getMainColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFF171820).withAlpha(alpha) : Colour(0xFFFFFFFF).withAlpha(alpha);
    }
    
    static Colour getSubColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFF323440).withAlpha(alpha) : Colour(0xFFE6E8EC).withAlpha(alpha);
    }
    
    static Colour getAccentColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFF73ACE5).withAlpha(alpha) : Colour(0xFF7DB2FF).withAlpha(alpha);
    }
    
    static inline bool isDarkMode = "true";
};
