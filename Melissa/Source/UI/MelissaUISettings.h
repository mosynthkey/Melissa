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
    static std::pair<uint32_t, uint32_t> getBackGroundGradationColour()
    {
        return { 0xFF1D2534, 0xFF090C10 };
    }
    
    static uint32_t getMainColour()
    {
        return 0xffddeaff;
    }
    
    static uint32_t getAccentColour()
    {
        return 0xff87BBFF;
    }
    
    static uint32_t getDialogBackgoundColour()
    {
        return 0xFF262930;
    }
    
    static uint32_t getTitleBarColour()
    {
        return 0xff38404D;
    }
    
    static uint32_t getComponentBackgroundColour()
    {
        return 0xFF39404C;
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
    
    static bool isJa;
    static bool isMac;

private:
    static String fontName_;
    static bool useJapaneseFont_;

    static String japaneseFontName_;
    static String latinFontName_;
};
