//
//  MelissaLevelMeter.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <cmath>

// Stereo level meter: two horizontal rounded bars (L top / R bottom) with peak hold.
class MelissaLevelMeter : public juce::Component, private juce::Timer
{
public:
    MelissaLevelMeter()
    {
        levelL_.store(0.f);
        levelR_.store(0.f);
        peakL_ = peakR_ = 0.f;
        peakHoldL_ = peakHoldR_ = 0;
        startTimerHz(30);
    }

    ~MelissaLevelMeter() override { stopTimer(); }

    // Call from audio thread — lock-free
    void pushSamples(const float* left, const float* right, int numSamples)
    {
        float rmsL = 0.f, rmsR = 0.f;
        for (int i = 0; i < numSamples; ++i)
        {
            rmsL += left[i]  * left[i];
            rmsR += right[i] * right[i];
        }
        rmsL = std::sqrt(rmsL / numSamples);
        rmsR = std::sqrt(rmsR / numSamples);

        auto blend = [](float cur, float next) {
            return next > cur ? next : cur * 0.85f + next * 0.15f;
        };
        levelL_.store(blend(levelL_.load(std::memory_order_relaxed), rmsL), std::memory_order_relaxed);
        levelR_.store(blend(levelR_.load(std::memory_order_relaxed), rmsR), std::memory_order_relaxed);
    }

    void paint(juce::Graphics& g) override
    {
        using namespace juce;
        const float w = static_cast<float>(getWidth());
        const float h = static_cast<float>(getHeight());
        const float barH   = (h - kGap) / 2.f;
        const float cornerR = barH / 2.f;

        auto drawBar = [&](float level, float peak, float y)
        {
            // background track
            g.setColour(Colour(0xff2a2a3a));
            g.fillRoundedRectangle(0.f, y, w, barH, cornerR);

            // level fill (left → right)
            const float fillW = level * w;
            if (fillW > 0.f)
            {
                auto col = level < 0.7f ? Colour(0xff4cd96a)
                         : level < 0.9f ? Colour(0xfff5c842)
                                        : Colour(0xffff4444);
                g.setColour(col);
                juce::Path clip;
                clip.addRoundedRectangle(0.f, y, w, barH, cornerR);
                g.saveState();
                g.reduceClipRegion(clip);
                g.fillRect(0.f, y, fillW, barH);
                g.restoreState();
            }

            // peak indicator (vertical line)
            if (peak > 0.01f)
            {
                const float px = peak * w - 1.f;
                g.setColour(Colour(0xccffffff));
                g.fillRect(px, y, 2.f, barH);
            }
        };

        drawBar(displayL_, peakL_, 0.f);
        drawBar(displayR_, peakR_, barH + kGap);
    }

private:
    static constexpr float kGap = 2.f;
    static constexpr int kPeakHoldFrames = 60;

    std::atomic<float> levelL_, levelR_;
    float displayL_ = 0.f, displayR_ = 0.f;
    float peakL_    = 0.f, peakR_    = 0.f;
    int   peakHoldL_ = 0,  peakHoldR_ = 0;

    void timerCallback() override
    {
        displayL_ = levelL_.load(std::memory_order_relaxed);
        displayR_ = levelR_.load(std::memory_order_relaxed);

        auto updatePeak = [&](float level, float& peak, int& hold)
        {
            if (level >= peak) { peak = level; hold = kPeakHoldFrames; }
            else if (hold > 0) { --hold; }
            else { peak *= 0.92f; }
        };
        updatePeak(displayL_, peakL_, peakHoldL_);
        updatePeak(displayR_, peakR_, peakHoldR_);

        repaint();
    }
};
