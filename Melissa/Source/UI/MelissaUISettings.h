#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <tuple>

class MelissaUISettings
{
public:
    static std::pair<uint32_t, uint32_t> backGroundGradationColour()
    {
        return { 0xFF1D2534, 0xFF090C10 };
    }
    
    static uint32_t mainColour()
    {
        return 0xffddeaff;
    }
    
    static uint32_t dialogBackgoundColour()
    {
        return 0xFF1B1F25;
    }
    
    static int fontSizeMain()
    {
        if (isMac)
        {
            return isJa ? 16 : 17;
        }
        else
        {
            return isJa ? 21 : 21;
        }
    }
    
    static int fontSizeSub()
    {
        return fontSizeMain() - 2;
    }
    
    static int fontSizeSmall()
    {
        return fontSizeMain() - 5;
    }
    
    static String fontName()
    {
        if (isMac)
        {
            return isJa ? "Hiragino Kaku Gothic Pro" : "San Francisco";
        }
        else
        {
            return isJa ? "Meiryo UI" : "Segoe UI";
        }
    }
    
    static bool isJa;
    static bool isMac;
};
