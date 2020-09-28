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
    else if (message.isNoteOn())
    {
        return processControlMessage(String("NoteOn ") + String(message.getNoteNumber()), 1.f);
    }
    
    return false;
}

bool MelissaShortcutManager::processControlMessage(const String& controlMessage, float value)
{
    printf("%s - %f\n", controlMessage.toRawUTF8(), value);
    command_->excuteCommand(MelissaDataSource::getInstance()->getAssignedShortcut(controlMessage) , value);
    return true;
}
