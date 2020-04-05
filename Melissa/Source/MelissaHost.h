//
//  MelissaHost.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaHost
{
public:
    virtual ~MelissaHost() {};
    virtual void closeTutorial() = 0;
};
