#pragma once

#include <sstream>
#include <string>

class MelissaUtility
{
public:
    static std::string getFormattedTime(float msec)
    {
        const int32_t minute = msec / (60.f * 1000.f);
        msec -= minute * 60 * 1000;
        const int32_t sec = msec / 1000.f;
        msec -= sec * 1000;
        
        std::stringstream ss;
        ss << minute << ":" << sec << "." << std::setw(4) << std::setfill('0') << msec;
        
        return ss.str();
    }
};
