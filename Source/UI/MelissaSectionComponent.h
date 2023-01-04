//
//  MelissaSectionComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"

class MelissaSectionComponent : public Component
{
public:
    MelissaSectionComponent(const String& title = "");
    
    // Compoent
    void paint(Graphics& g) override;
    
private:
    String title_;
};
