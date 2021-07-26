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
        return isDarkMode ? Colour(0xFF181823).withAlpha(alpha) : Colour(0xFFFFFFFF).withAlpha(alpha);
    }
    
    static Colour getSubColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFF303340).withAlpha(alpha) : Colour(0xFFE6E8EC).withAlpha(alpha);
    }
    
    static Colour getAccentColour(float alpha = 1.f)
    {
        return isDarkMode ? Colour(0xFF7DB2FF).withAlpha(alpha) : Colour(0xFF7DB2FF).withAlpha(alpha);
    }
    
    static int getFontSizeMain()
    {
        if (isMac)
        {
            return 16;
        }
        else
        {
            return 21;
        }
    }
    
    static int getFontSizeSub()
    {
        return getFontSizeMain() - 2;
    }
    
    static int getFontSizeSmall()
    {
        return getFontSizeMain() - 5;
    }
    
    static String getFontName()
    {
        if (fontName_.isNotEmpty()) return fontName_;

        useJapaneseFont_ = isJapaneseFontAvailable();
        fontName_ = useJapaneseFont_ ? japaneseFontName_ : latinFontName_;
        
        return fontName_;
    }

    static bool isJapaneseFontAvailable()
    {
        Array<Font> availableFonts;
        Font::findFonts(availableFonts);

        for (auto&& font : availableFonts)
        {
            if (font.getTypefaceName() == japaneseFontName_) return true;
        }

        return false;
    }
    
    static bool isDarkMode;
    static bool isJa;
    static bool isMac;

private:
    static String fontName_;
    static bool useJapaneseFont_;

    static String japaneseFontName_;
    static String latinFontName_;
};
