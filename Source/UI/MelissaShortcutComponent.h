//
//  MelissaShortcutComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "MelissaCommandComboBox.h"
#include "MelissaDataSource.h"
#include "MelissaShortcutManager.h"

class MelissaShortcutComponent : public juce::Component,
                                 public MelissaDataSourceListener,
                                 public MelissaShortcutListener
{
public:
    MelissaShortcutComponent();
    ~MelissaShortcutComponent();
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void controlMessageReceived(const juce::String& controlMessage) override;
    
private:
    std::unique_ptr<juce::Label> commandLabel_;
    std::unique_ptr<MelissaCommandComboBox> assignCombobox_;
    std::unique_ptr<juce::TextButton> resetButton_;
    std::unique_ptr<juce::TextButton> resetAllButton_;
    
    class ShortcutListBox;
    std::unique_ptr<ShortcutListBox> shortcutListBox_;
    
    void initAssignBox();
    void shortcutUpdated() override;
    
    MelissaShortcutManager* shortcutManager_;
    
    int registerEditY_;
    juce::Rectangle<int> listRect_;
};
