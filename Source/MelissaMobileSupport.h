//
//  MelissaMobileSupport.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaMobileSupport
{
public:
    MelissaMobileSupport() {}
    
    static File importFile(const URL& fileUrl);
    
};
