//
//  MelissaSpleeterBridge.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaSpleeterBridge : public Thread
{
public:
    MelissaSpleeterBridge();
    ~MelissaSpleeterBridge() {}
    void requestStems();
    
    
private:
    void run() override;
    void createStems();
};
