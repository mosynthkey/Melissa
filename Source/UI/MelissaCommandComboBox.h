//
//  MelissaCommandComboBox.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"

class MelissaCommandComboBox : public juce::ComboBox
{    
public:
    MelissaCommandComboBox();
    ~MelissaCommandComboBox();
    void select(const juce::String& command);
    void setShortcut(const juce::String& shortcut) { shortcut_ = shortcut; }
    juce::String getSelectedCommand() const { return selectedCommand_; }

    std::function<void(const juce::String& command)> onSelectedCommandChanged_;
    
private:
    MelissaLookAndFeel laf_;
    juce::PopupMenu popupMenu_;
    int noAssignMenuId_;
    juce::String selectedCommand_;
    juce::String shortcut_;
    std::vector<Listener*> listeners_;
    
    void showPopup() override;
    juce::String getSelectedCommandWithItemId(int itemId) const;
    void update();
};

