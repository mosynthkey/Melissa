//
//  MelissaDoubleClickEditLabel.h
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"

class MelissaDoubleClickEditLabel : public juce::Label
{
public:
    MelissaDoubleClickEditLabel(juce::TableListBoxModel* owner, int rowNumber, int columnId) : owner_(owner), rowNumber_(rowNumber), columnId_(columnId)
    {
        setLookAndFeel(&laf_);
        setEditable(false, true);
        setWantsKeyboardFocus(false);
    }
    
    ~MelissaDoubleClickEditLabel()
    {
        setLookAndFeel(nullptr);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        owner_->cellClicked(rowNumber_, columnId_, event);
    }
    
    void setRowNumber(int rowNumber)
    {
        rowNumber_ = rowNumber;
    }
    
    int getRowNumber() const { return rowNumber_; }
    
    void setColumnId(int columnId)
    {
        columnId_ = columnId;
    }
    
    int getColumnId() const { return columnId_; }
    
private:
    juce::TableListBoxModel* owner_;
    int rowNumber_;
    int columnId_;
    
    MelissaLookAndFeel_Memo laf_;
};
