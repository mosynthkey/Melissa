//
//  MelissaBeatRulerComponent.cpp
//  Melissa
//
//  Copyright(c) 2024 Masaki Ono
//

#include "MelissaBeatRulerComponent.h"
#include "MelissaUISettings.h"

MelissaBeatRulerComponent::MelissaBeatRulerComponent() : audioLengthSeconds_(0.0f)
{
    
}

void MelissaBeatRulerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Fill background with sub colour
    g.fillAll(MelissaUISettings::getSubColour());
    
    // Draw horizontal line at bottom
    g.setColour(MelissaUISettings::getTextColour(0.3f));
    g.drawHorizontalLine(bounds.getBottom() - 1, 0.0f, static_cast<float>(bounds.getWidth()));
    
    // Draw onset/downbeat position lines (fallback to beats if no downbeats)
    if (beatResult_.isValid && audioLengthSeconds_ > 0.0f)
    {
        g.setColour(MelissaUISettings::getAccentColour(0.7f));
        
        // Use downbeats if available, otherwise fallback to beats
        const auto& positions = !beatResult_.downbeatPositions.empty() ? 
                               beatResult_.downbeatPositions : beatResult_.beatPositions;
        
        for (const float pos : positions)
        {
            // Convert position (seconds) to pixel position
            float xPos = (pos / audioLengthSeconds_) * bounds.getWidth();
            
            if (xPos >= 0.0f && xPos <= bounds.getWidth())
            {
                g.drawVerticalLine(static_cast<int>(xPos), 0.0f, static_cast<float>(bounds.getHeight()));
            }
        }
    }
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