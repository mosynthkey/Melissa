//
//  MelissaBeepGenerator.cpp
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaBeepGenerator.h"

MelissaBeepGenerator::MelissaBeepGenerator() : sampleRate_(0), pitch_(440), osc_(0.f), amp_(0.f)
{
}

float MelissaBeepGenerator::render()
{
    const float beep = osc_ * amp_;
    
    osc_ += 2.f / (sampleRate_ / pitch_);
    if (osc_ > 1.f) osc_ = -1.f;
    
    if (amp_ > 0.f)
    {
        amp_ -= 1.f / static_cast<float>(sampleRate_) * 20.f;
    }
    else if (amp_ < 0.f)
    {
        amp_ = 0.f;
    }
    
    return beep;
}

void MelissaBeepGenerator::trigger(float pitch)
{
    osc_ = 1.f;
    pitch_ = pitch;
    amp_ = 1.f;
}
