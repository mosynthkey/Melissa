#pragma once

#include <tuple>

class MelissaColourScheme
{
public:
    static std::pair<uint32_t, uint32_t> BackGroundGradationColour()
    {
        return { 0xFF252F41, 0xFF1F2024 };
    }
    
    static uint32_t MainColour()
    {
        return 0xffDDEAFF;
    }
};
