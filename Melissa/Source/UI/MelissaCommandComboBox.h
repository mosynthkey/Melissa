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

    std::function<void(const String& command)> onSelectedCommandChanged_;
    
private:
    MelissaLookAndFeel laf_;
    PopupMenu popupMenu;
    int noAssignMenuId_;
    std::vector<Listener*> listeners_;
    
    void showPopup() override;
    String getSelectedCommand(int itemId) const;
    void update();
};

