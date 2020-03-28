#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <tuple>

class MelissaUISettings
{
public:
    static std::pair<uint32_t, uint32_t> BackGroundGradationColour()
    {
        return { 0xFF252F41, 0xFF1F2024 };
    }
    
    static uint32_t MainColour()
    {
        return 0xffddeaff;
    }
    
    static uint32_t DialogBackgoundColour()
    {
        return 0xFF1B1F25;
    }
    
    static int FontSizeMain()
    {
        if (isMac)
        {
            return isJa ? 16 : 17;
        }
        else
        {
            return isJa ? 22 : 22;
        }
    }
    
    static int FontSizeSub()
    {
        if (isMac)
        {
            return isJa ? 14 : 15;
        }
        else
        {
            return isJa ? 20 : 20;
        }
    }
    
    static String FontName()
    {
        if (isMac)
        {
            return isJa ? "Hiragino Kaku Gothic Pro" : "San Francisco";
        }
        else
        {
            return isJa ? "Meiryo UI" : "";
        }
    }
    
    static bool isJa;
    static bool isMac;
};
