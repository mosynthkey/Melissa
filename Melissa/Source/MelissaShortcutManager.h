//
//  MelissaShortcutManager.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaCommand.h"
#include <string>
#include <vector>

class MelissaShortcutManager
{
public:
    // Singleton
    static MelissaShortcutManager* getInstance() { return &instance_; }
    MelissaShortcutManager(const MelissaShortcutManager&) = delete;
    MelissaShortcutManager& operator=(const MelissaShortcutManager&) = delete;
    MelissaShortcutManager(MelissaShortcutManager&&) = delete;
    MelissaShortcutManager& operator=(MelissaShortcutManager&&) = delete;
    
    bool processKeyboardMessage(const String& keyboardDescription);
    bool processMIDIMessage(const MidiMessage& message);
    
private:
    // Singleton
    MelissaShortcutManager();
    ~MelissaShortcutManager() {}
    static MelissaShortcutManager instance_;

    bool processControlMessage(const String& controlMessage, float value);
    
    MelissaCommand* command_;
    uint32 noteOnTimeCount[128];
};
