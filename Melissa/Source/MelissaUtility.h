#pragma once

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

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
        ss << minute << ":" << sec << "." << static_cast<int32_t>(msec) ;
        
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
    
    static std::string getFormattedPitch(int32_t pitch)
    {
        if (pitch > 0)
        {
            return "# " + std::to_string(pitch);
        }
        else if (pitch == 0)
        {
            return "Original";
        }
        else
        {
            return "b " + std::to_string(pitch * -1);
        }
    }
};
