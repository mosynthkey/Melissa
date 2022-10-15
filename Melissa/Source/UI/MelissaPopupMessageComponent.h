//
//  MelissaPopupMessageComponent.h
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaShortcutManager.h"

class MelissaPopupMessageComponent : public Component, public Timer
{
public:
    MelissaPopupMessageComponent();
    ~MelissaPopupMessageComponent();
    void show(const String& text);
    
private:
    void paint(Graphics& g) override;
    void timerCallback() override;
    String text_;
    
    ComponentAnimator animator_;
};
