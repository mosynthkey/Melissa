//
//  MelissaWaveformMouseEventComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaWaveformMouseEventComponent.h"

using namespace juce;

void MelissaWaveformMouseEventComponent::addListener(MelissaWaveformMouseEventListener* listener)
{
    listeners_.emplace_back(listener);
}

void MelissaWaveformMouseEventComponent::removeListener(MelissaWaveformMouseEventListener* listener)
{
    for (size_t listenerIndex = 0; listenerIndex < listeners_.size(); ++listenerIndex)
    {
        if (listeners_[listenerIndex] == listener)
        {
            listeners_.erase(listeners_.begin() + listenerIndex);
            return;
        }
    }
}

void MelissaWaveformMouseEventComponent::MelissaWaveformMouseEventComponent::mouseDown(const MouseEvent& event)
{
    const float xRatio = std::clamp(event.x / static_cast<float>(getWidth()), 0.f, 1.f);
    for (auto&& l : listeners_) l->mouseDown(xRatio, event.mods.isLeftButtonDown());
}

void MelissaWaveformMouseEventComponent::MelissaWaveformMouseEventComponent::mouseUp(const MouseEvent& event)
{
    const float xRatio = std::clamp(event.x / static_cast<float>(getWidth()), 0.f, 1.f);
    for (auto&& l : listeners_) l->mouseUp(xRatio);
}

void MelissaWaveformMouseEventComponent::MelissaWaveformMouseEventComponent::mouseMove(const MouseEvent& event)
{
    const float xRatio = std::clamp(event.x / static_cast<float>(getWidth()), 0.f, 1.f);
    for (auto&& l : listeners_) l->mouseMove(xRatio);
}

void MelissaWaveformMouseEventComponent::MelissaWaveformMouseEventComponent::mouseDrag(const MouseEvent& event)
{
    const float xRatio = std::clamp(event.x / static_cast<float>(getWidth()), 0.f, 1.f);
    for (auto&& l : listeners_) l->mouseDrag(xRatio);
}

void MelissaWaveformMouseEventComponent::mouseEnter(const MouseEvent& event)
{
    const float xRatio = std::clamp(event.x / static_cast<float>(getWidth()), 0.f, 1.f);
    for (auto&& l : listeners_) l->mouseEnter(xRatio);
}

void MelissaWaveformMouseEventComponent::mouseExit(const MouseEvent& event)
{
    const float xRatio = std::clamp(event.x / static_cast<float>(getWidth()), 0.f, 1.f);
    for (auto&& l : listeners_) l->mouseExit(xRatio);
}
