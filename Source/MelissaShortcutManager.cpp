//
//  MelissaShortcutManager.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaShortcutManager.h"

using namespace juce;

MelissaShortcutManager MelissaShortcutManager::instance_;

MelissaShortcutManager::MelissaShortcutManager() : enable_(true)
{
    command_ = MelissaCommand::getInstance();
}

bool MelissaShortcutManager::processKeyboardMessage(const String& keyboardDescription)
{
    return processControlMessage(keyboardDescription, 1.f);
}

bool MelissaShortcutManager::processMIDIMessage(const MidiMessage& message)
{
    if (message.isController())
    {
        return processControlMessage(String("CC #") + String(message.getControllerNumber()), message.getControllerValue() / 127.f);
    }
    else if (message.isNoteOn() && message.getChannel() == 1)
    {
        if (2 <= noteOnHistory[message.getNoteNumber()])
        {
            startTimer(0);
        }
        else
        {
            startTimer(400);
            ++noteOnHistory[message.getNoteNumber()];
        }
        return true;
    }
    
    return false;
}

void MelissaShortcutManager::addListener(MelissaShortcutListener* listener)
{
    listeners_.emplace_back(listener);
}

void MelissaShortcutManager::removeListener(MelissaShortcutListener* listener)
{
    for (size_t listenerIndex = 0; listenerIndex < listeners_.size(); ++listenerIndex)
    {
        if (listeners_[listenerIndex] == listener)
        {
            listeners_.erase(listeners_.begin() + listenerIndex);
            return;
        }
    }
}

void MelissaShortcutManager::timerCallback()
{
    auto noteNumberToString = [](int noteNumber) {
        String noteName[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        const int noteNumberInOct = -2 + noteNumber / 12;
        return noteName[noteNumber % 12] + String(noteNumberInOct);
    };
    
    for (int noteNumber = 0; noteNumber < maxNoteNumber; ++noteNumber)
    {
        if (2 <= noteOnHistory[noteNumber])
        {
            processControlMessage(String("NoteOnDouble ") + noteNumberToString(noteNumber), 1.f);
        }
        else if (noteOnHistory[noteNumber] == 1)
        {
            processControlMessage(String("NoteOn ") + noteNumberToString(noteNumber), 1.f);
        }
        noteOnHistory[noteNumber] = 0;
    }
    
}

bool MelissaShortcutManager::processControlMessage(const String& controlMessage, float value)
{
    for (auto&& l : listeners_)
    {
        MessageManager::callAsync([&, l, controlMessage]() { l->controlMessageReceived(controlMessage); });
    }
    
    if (enable_) command_->excuteCommand(MelissaDataSource::getInstance()->getAssignedShortcut(controlMessage) , value);
    
    return true;
}
