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
    static juce::Colour getTextColour(float alpha = 1.f)
    {
        return isDarkMode ? juce::Colour(0xFFFFFF).withAlpha(alpha) : juce::Colour(0xFF000000).withAlpha(alpha);
    }
    
    static juce::Colour getWaveformColour(float alpha = 1.f)
    {
        return isDarkMode ? juce::Colour(0xFFD9E8FF).withAlpha(alpha) : juce::Colour(0xFF89B8FF).withAlpha(alpha);
    }
    
    static juce::Colour getMainColour(float alpha = 1.f)
    {
        return isDarkMode ? juce::Colour(0xFF12131A).withAlpha(alpha) : juce::Colour(0xFFFFFFFF).withAlpha(alpha);
    }
    
    static juce::Colour getSubColour(float alpha = 1.f)
    {
        return isDarkMode ? juce::Colour(0xFF282C37).withAlpha(alpha) : juce::Colour(0xFFE6E8EC).withAlpha(alpha);
    }
    
    static juce::Colour getAccentColour(float alpha = 1.f)
    {
#ifdef MELISSA_USE_STEM_SEPARATION
        return isDarkMode ? juce::Colour(0xFF80BFFF).withAlpha(alpha) : juce::Colour(0xFF7DB2FF).withAlpha(alpha);
#else
        return isDarkMode ? juce::Colour(0xFF80B3FF).withAlpha(alpha) : juce::Colour(0xFF80B3FF).withAlpha(alpha);
#endif
    }
    
    static inline bool isDarkMode = "true";
};
