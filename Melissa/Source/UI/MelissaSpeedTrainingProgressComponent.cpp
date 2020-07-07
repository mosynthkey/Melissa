//
//  MelissaSpeedTrainingProgressComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaSpeedTrainingProgressComponent.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

MelissaSpeedTrainingProgressComponent::MelissaSpeedTrainingProgressComponent() :
model_(MelissaModel::getInstance()),
prevPlayingPosRatio_(0.f)
{
    model_->addListener(this);
    startTimer(1000 / 10);
}

void MelissaSpeedTrainingProgressComponent::speedIncStartChanged(int speedIncStart)
{
    repaint();
}

void MelissaSpeedTrainingProgressComponent::speedIncGoalChanged(int speedIncGoal)
{
    repaint();
}

void MelissaSpeedTrainingProgressComponent::paint(Graphics& g)
{
    constexpr int lineHeight = 4;
    constexpr int triangleWidth = 10;
    constexpr int triangleHeight = 10;
    
    const auto width = getWidth();
    
    g.setColour(Colours::white.withAlpha(0.2f));
    g.fillRoundedRectangle(triangleWidth / 2, 0, width - triangleWidth, lineHeight, lineHeight / lineHeight);
    
    printf("%d / %d\n", (model_->getCurrentSpeed() - model_->getSpeedIncStart()), (model_->getSpeedIncGoal() - model_->getSpeedIncStart()));
    const auto currentRatio = std::clamp((model_->getCurrentSpeed() - model_->getSpeedIncStart()) / static_cast<float>(model_->getSpeedIncGoal() - model_->getSpeedIncStart()), 0.f, 1.f);
    printf("currentRatio = %f\n", currentRatio);
    
    Path path;
    const int triangleCenter = currentRatio * (width - triangleWidth) + triangleWidth / 2;
    path.startNewSubPath(triangleCenter, 0);
    path.lineTo(triangleCenter - triangleWidth / 2, triangleHeight);
    path.lineTo(triangleCenter + triangleWidth / 2, triangleHeight);
    
    g.setColour(Colour(MelissaUISettings::getAccentColour()));
    g.fillPath(path);
    
    const auto currentSpeedStr = String(model_->getCurrentSpeed()) + "%";
    const auto strSize = MelissaUtility::getStringSize(font_, currentSpeedStr);
    
    auto strX = std::clamp(triangleCenter - strSize.first / 2, 0, width - strSize.first);
    g.setColour(Colours::white.withAlpha(0.8f));
    g.setFont(font_);
    g.drawText(currentSpeedStr, strX, triangleHeight + 2, strSize.first, strSize.second, Justification::centred);
}

void MelissaSpeedTrainingProgressComponent::timerCallback()
{
    const auto currentRatio = model_->getPlayingPosRatio();
    if (currentRatio < prevPlayingPosRatio_) repaint();
    prevPlayingPosRatio_ = currentRatio;
}

