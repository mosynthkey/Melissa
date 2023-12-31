//
//  MelissaBeepGenerator.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include <stddef.h>

class MelissaBeepGenerator
{
public:
    MelissaBeepGenerator();
    void setOutputSampleRate(int sampleRate) { sampleRate_ = sampleRate; };
    float render();
    void trigger(float pitch);
    
private:
    int   sampleRate_;
    float pitch_;
    float osc_;
    float amp_;
};
