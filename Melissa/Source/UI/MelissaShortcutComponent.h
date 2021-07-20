//
//  MelissaShortcutComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaCommandComboBox.h"
#include "MelissaShortcutManager.h"

class MelissaShortcutComponent : public Component,
                                 public MelissaShortcutListener
{
public:
    MelissaShortcutComponent();
    ~MelissaShortcutComponent();
    void resized() override;
    void paint(Graphics& g) override;
    
    void controlMessageReceived(const String& controlMessage) override;
    
private:
    std::unique_ptr<Label> commandLabel_;
    std::unique_ptr<MelissaCommandComboBox> assignCombobox_;
    std::unique_ptr<TextButton> resetButton_;
    std::unique_ptr<TextButton> resetAllButton_;
    
    class ShortcutListBox;
    std::unique_ptr<ShortcutListBox> shortcutListBox_;
    
    void initAssignBox();
    
    MelissaShortcutManager* shortcutManager_;
    
    int registerEditY_;
    Rectangle<int> listRect_;
};
