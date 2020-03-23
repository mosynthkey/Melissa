#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaHost
{
public:
    virtual ~MelissaHost() {};
    virtual void updatePracticeList(const Array<var>& list) = 0;
    virtual void createPlaylist(const String& name) = 0;
    virtual bool loadFile(const String& filePath) = 0;
    
    virtual void closeTutorial() = 0;
};
