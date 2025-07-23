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
    constexpr float lineWidth = 1.0f;
    
    // Draw horizontal line at bottom
    g.setColour(MelissaUISettings::getAccentColour());
    g.fillRect(0, bounds.getBottom() - 1, bounds.getWidth(), 1);
    
    if (!beatResult_.isValid || audioLengthSeconds_ <= 0.0f)
        return;
    
    // Calculate average pixel spacing between beats to determine if we should draw all beats
    constexpr float kMinPixelSpacing = 20.0f; // Minimum spacing to draw all beats
    bool shouldDrawAllBeats = false;
    
    if (!beatResult_.beatPositions.empty() && beatResult_.beatPositions.size() > 1)
    {
        // Calculate average beat interval in pixels
        float totalDuration = beatResult_.beatPositions.back() - beatResult_.beatPositions.front();
        float avgBeatInterval = totalDuration / (beatResult_.beatPositions.size() - 1);
        float avgPixelSpacing = (avgBeatInterval / audioLengthSeconds_) * bounds.getWidth();
        
        shouldDrawAllBeats = avgPixelSpacing >= kMinPixelSpacing;
    }
    
    if (shouldDrawAllBeats && !beatResult_.beatPositions.empty())
    {
        // Draw all beats with different heights for downbeats vs regular beats
        for (size_t i = 0; i < beatResult_.beatPositions.size(); ++i)
        {
            float pos = beatResult_.beatPositions[i];
            float xPos = (pos / audioLengthSeconds_) * bounds.getWidth();
            
            if (xPos >= 0.0f && xPos <= bounds.getWidth())
            {
                
                
                // Check if this beat is a downbeat
                bool isDownbeat = false;
                if (i < beatResult_.beatCounts.size())
                {
                    isDownbeat = (beatResult_.beatCounts[i] == 1);
                }
                
                // Different line heights for downbeats vs regular beats
                float lineHeight = isDownbeat ? bounds.getHeight() * 0.8f : bounds.getHeight() * 0.5f;
                float yStart = bounds.getHeight() - lineHeight;

                g.fillRect(xPos - lineWidth / 2.f, static_cast<float>(bounds.getHeight() - lineHeight), lineWidth, lineHeight);
            }
        }
    }
    else if (!beatResult_.downbeatPositions.empty())
    {
        // Draw only downbeats when spacing is too tight
        float lineHeight = bounds.getHeight() * 0.8f;

        for (const float pos : beatResult_.downbeatPositions)
        {
            float xPos = (pos / audioLengthSeconds_) * bounds.getWidth();
            
            if (xPos >= 0.0f && xPos <= bounds.getWidth())
            {
                g.fillRect(xPos - lineWidth / 2.f, static_cast<float>(bounds.getHeight() - lineHeight), lineWidth, lineHeight);
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
