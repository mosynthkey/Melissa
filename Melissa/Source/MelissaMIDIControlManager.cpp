//
//  MelissaMIDIControlManager.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaMIDIControlManager.h"

MelissaMIDIControlManager::MelissaMIDIControlManager()
{
    
}

MelissaMIDIControlManager::~MelissaMIDIControlManager()
{
    
}

void MelissaMIDIControlManager::registFromVar(var settings)
{
    
}

var MelissaMIDIControlManager::getSettingsAsVar()
{
    return var();
}

void MelissaMIDIControlManager::processMIDIMessage(const MidiMessage& message)
{
    auto desc = message.getDescription();
    if (desc != "f8") printf("%s\n", desc.toRawUTF8());
}
