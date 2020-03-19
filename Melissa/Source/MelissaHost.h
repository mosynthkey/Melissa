#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaHost
{
public:
    virtual ~MelissaHost() {};
    virtual void updatePracticeList(const Array<var>& list) = 0;
    virtual void createSetlist(const String& name) = 0;
    virtual bool loadFile(const String& filePath) = 0;
    
    virtual void showModalDialog(std::shared_ptr<Component> component, const String& title) = 0;
    virtual void showPreferencesDialog() = 0;
    virtual void closeModalDialog() = 0;
    
    virtual void closeTutorial() = 0;
};
