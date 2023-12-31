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

class MelissaShortcutListener
{
public:
    virtual ~MelissaShortcutListener() {};
    virtual void controlMessageReceived(const juce::String& controlMessage) = 0;
};

class MelissaShortcutManager : public juce::Timer
{    
public:
    // Singleton
    static MelissaShortcutManager* getInstance() { return &instance_; }
    MelissaShortcutManager(const MelissaShortcutManager&) = delete;
    MelissaShortcutManager& operator=(const MelissaShortcutManager&) = delete;
    MelissaShortcutManager(MelissaShortcutManager&&) = delete;
    MelissaShortcutManager& operator=(MelissaShortcutManager&&) = delete;
    
    void setEnable(bool enable) { enable_ = enable; }
    bool processKeyboardMessage(const juce::String& keyboardDescription);
    bool processMIDIMessage(const juce::MidiMessage& message);
    
    void addListener(MelissaShortcutListener* listener);
    void removeListener(MelissaShortcutListener* listener);
    
private:
    // Singleton
    MelissaShortcutManager();
    ~MelissaShortcutManager() {}
    static MelissaShortcutManager instance_;
    bool enable_;

    void timerCallback() override;
    bool processControlMessage(const juce::String& controlMessage, float value);
    
    MelissaCommand* command_;
    
    enum { maxNoteNumber = 128 };
    int noteOnHistory[maxNoteNumber];
    
    std::vector<MelissaShortcutListener*> listeners_;
};
