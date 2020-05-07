//
//  MelissaSpeedTrainingProgressComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaSpeedTrainingProgressComponent : public Component
{
public:
    MelissaSpeedTrainingProgressComponent();
    
    void setFont(const Font& font);
    void setMinRatio(float ratio);
    void setMaxRatio(float ratio);
    void setCurrentRatioAndLabelText(float ratio, const String& labelText);
    
    // Component
    void paint(Graphics& g) override;
    
private:
    Font font_;
    float minRatio_;
    float maxRatio_;
    float currentRatio_;
    String labelText_;
};
