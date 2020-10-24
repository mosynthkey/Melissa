//
//  MelissaShortcutComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaShortcutComponent : public Component
{
public:
    MelissaShortcutComponent();
    ~MelissaShortcutComponent();
    void resized() override;
    
private:
    class ShortcutListBox;
    std::unique_ptr<ShortcutListBox> shortcutListBox_;
};
