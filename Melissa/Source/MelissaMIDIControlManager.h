#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <string>
#include <vector>

class MelissaMIDIControlManager
{
public:
    MelissaMIDIControlManager();
    ~MelissaMIDIControlManager();
    
    void registFromVar(var settings);
    var getSettingsAsVar();
    
    void processMIDIMessage(const MidiMessage& message);

private:
    struct Setting
    {
        std::string name;
        std::string key;
        enum Type
        {
            kType_bool,
            kType_ratio,
        } type;
        int cc;
    };
    
    std::vector<Setting> settings_;
    
};
