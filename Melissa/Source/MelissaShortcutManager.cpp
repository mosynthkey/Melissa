//
//  MelissaShortcutManager.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaShortcutManager.h"

MelissaShortcutManager MelissaShortcutManager::instance_;

MelissaShortcutManager::MelissaShortcutManager()
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

void MelissaShortcutManager::timerCallback()
{
    for (int noteNumber = 0; noteNumber < maxNoteNumber; ++noteNumber)
    {
        if (2 <= noteOnHistory[noteNumber])
        {
            processControlMessage(String("NoteOnDouble ") + String(noteNumber), 1.f);
        }
        else if (noteOnHistory[noteNumber] == 1)
        {
            processControlMessage(String("NoteOn ") + String(noteNumber), 1.f);
        }
        noteOnHistory[noteNumber] = 0;
    }
    
}

bool MelissaShortcutManager::processControlMessage(const String& controlMessage, float value)
{
    printf("%s - %f\n", controlMessage.toRawUTF8(), value);
    command_->excuteCommand(MelissaDataSource::getInstance()->getAssignedShortcut(controlMessage) , value);
    return true;
}
