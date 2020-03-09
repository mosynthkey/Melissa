#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaHost
{
public:
    virtual ~MelissaHost() {};
    virtual void setMelissaParameters(float aRatio, float bRatio, float speed) = 0;
    virtual void getMelissaParameters(float* aRatio, float* bRatio, float* speed) = 0;
    virtual void updatePracticeList(const Array<var>& list) = 0;
    virtual void createSetlist(const std::string& name) = 0;
    virtual bool loadFile(const String& filePath) = 0;
    
    virtual void showModalDialog(std::shared_ptr<Component> component, const std::string& title) = 0;
    virtual void showPreferencesDialog() = 0;
    virtual void closeModalDialog() = 0;
};
