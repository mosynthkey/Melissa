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

class MelissaCommandComboBox : public ComboBox
{
public:
    MelissaCommandComboBox();
    ~MelissaCommandComboBox();
    void select(const String& command);
    void setShortcut(const String& shortcut) { shortcut_ = shortcut; }
    String getSelectedCommand() const { return selectedCommand_; }

    std::function<void(const String& command)> onSelectedCommandChanged_;
    
private:
    MelissaLookAndFeel laf_;
    PopupMenu popupMenu_;
    int noAssignMenuId_;
    String selectedCommand_;
    String shortcut_;
    std::vector<Listener*> listeners_;
    
    void showPopup() override;
    String getSelectedCommandWithItemId(int itemId) const;
    void update();
};

