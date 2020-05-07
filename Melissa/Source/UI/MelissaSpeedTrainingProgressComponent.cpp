//
//  MelissaSpeedTrainingProgressComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaSpeedTrainingProgressComponent.h"
#include "MelissaUtility.h"

MelissaSpeedTrainingProgressComponent::MelissaSpeedTrainingProgressComponent() :
minRatio_(0.f),
maxRatio_(1.f),
currentRatio_(0.f),
labelText_("")
{
    
}

void MelissaSpeedTrainingProgressComponent::setMinRatio(float ratio)
{
    if (ratio < 0.f || 1.f < ratio || maxRatio_ < ratio) return;
    minRatio_ = ratio;
    repaint();
}

void MelissaSpeedTrainingProgressComponent::setMaxRatio(float ratio)
{
    if (ratio < 0.f || 1.f < ratio || ratio < minRatio_) return;
    maxRatio_ = ratio;
    repaint();
}

void MelissaSpeedTrainingProgressComponent::setCurrentRatioAndLabelText(float ratio, const String& labelText)
{
    if (ratio < 0.f || 1.f < ratio || ratio < minRatio_ || maxRatio_ < ratio) return;
    currentRatio_ = ratio;
    labelText_ = labelText;
    repaint();
}

void MelissaSpeedTrainingProgressComponent::paint(Graphics& g)
{
    constexpr int lineHeight = 4;
    constexpr int triangleWidth = 4;
    constexpr int triangleHeight = 4;
    constexpr int marginX = 30;
    
    int strWidth, strHeight;
    std::tie(strWidth, strHeight) = MelissaUtility::getStringSize(font_, labelText_);
    const int strXPos = (getWidth() - marginX * 2) * currentRatio_ + marginX - strWidth / 2;
    g.setColour(Colours::white);
    g.drawText(labelText_, strXPos, 0, strWidth, strHeight, Justification::centred);
    
    
    
}
