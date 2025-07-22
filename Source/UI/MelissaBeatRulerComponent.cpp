//
//  MelissaBeatRulerComponent.cpp
//  Melissa
//
//  Copyright(c) 2024 Masaki Ono
//

#include "MelissaBeatRulerComponent.h"

MelissaBeatRulerComponent::MelissaBeatRulerComponent() : audioLengthSeconds_(0.0f)
{
    
}

void MelissaBeatRulerComponent::paint(juce::Graphics& g)
{
    // For now, just fill with red color as placeholder
    g.fillAll(juce::Colours::red.withAlpha(0.3f));
    
    // Add some debug text
    g.setColour(juce::Colours::black);
    g.setFont(12.0f);
    juce::String text = "Beat Ruler - Beats: " + juce::String(beatResult_.beatPositions.size());
    g.drawText(text, getLocalBounds(), juce::Justification::centredLeft);
}

void MelissaBeatRulerComponent::resized()
{
    // Nothing to resize for now
}

void MelissaBeatRulerComponent::setBeatResult(const MelissaBeatResult& result)
{
    beatResult_ = result;
    repaint();
}

void MelissaBeatRulerComponent::setAudioLength(float lengthInSeconds)
{
    audioLengthSeconds_ = lengthInSeconds;
    repaint();
}

void MelissaBeatRulerComponent::clearBeats()
{
    beatResult_ = MelissaBeatResult();
    repaint();
}