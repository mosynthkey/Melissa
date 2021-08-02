//
//  MelissaShortcutPopupComponent.h
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaShortcutManager.h"

class MelissaShortcutPopupComponent : public Component, public Timer, public MelissaShortcutListener
{
public:
    MelissaShortcutPopupComponent();
    ~MelissaShortcutPopupComponent();
    void show(const String& text);
    
private:
    void paint(Graphics& g) override;
    void timerCallback() override;
    void controlMessageReceived(const String& controlMessage) override;
    String text_;
    
    ComponentAnimator animator_;
};
