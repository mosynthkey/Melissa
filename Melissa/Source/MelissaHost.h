#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaHost
{
public:
    virtual ~MelissaHost() {};
    virtual void setMelissaParameters(float aRatio, float bRatio, float speed, int32_t pitch) = 0;
    virtual void getMelissaParameters(float* aRatio, float* bRatio, float* speed, int32_t* pitch, int32_t* count) = 0;
    virtual void updatePracticeList(const Array<var>& list) = 0;
    virtual bool loadFile(const String& filePath) = 0;
};
