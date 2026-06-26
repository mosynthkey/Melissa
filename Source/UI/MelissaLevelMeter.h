//
//  MelissaLevelMeter.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include <JuceHeader.h>
#include "MelissaUISettings.h"
#include <atomic>
#include <cmath>
#include <functional>

// Combined stereo level meter + horizontal volume slider.
// - Two horizontal bars (L top / R bottom) show RMS level in accent colour.
// - A vertical thumb line shows the current volume value (0..1).
// - Drag left/right to change volume; double-click to reset to 1.0.
class MelissaLevelMeter : public juce::Component, private juce::Timer
{
public:
    std::function<void(float)> onValueChanged;

    MelissaLevelMeter()
    {
        levelL_.store(0.f);
        levelR_.store(0.f);
        startTimerHz(30);
    }

    ~MelissaLevelMeter() override { stopTimer(); }

    void setValue(float v, bool notify = false)
    {
        value_ = juce::jlimit(0.01f, 1.f, v);
        if (notify && onValueChanged) onValueChanged(value_);
        repaint();
    }
    float getValue() const { return value_; }

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
        g.fillAll(MelissaUISettings::getMainColour());

        const float w      = static_cast<float>(getWidth());
        const float h      = static_cast<float>(getHeight());
        // bars occupy the centre, leaving kBarMargin top and bottom for the thumb
        const float barTop = kBarMargin;
        const float barArea= h - kBarMargin * 2.f;
        const float barH   = (barArea - kGap) / 2.f;
        const float cornerR = barH / 2.f;
        const auto accent   = MelissaUISettings::getAccentColour();

        // bars inset by kBarMargin on all sides; thumb uses the full component rect
        const float barX = kBarMargin;
        const float barW = w - kBarMargin * 2.f;

        auto drawBar = [&](float level, float peak, float y)
        {
            g.setColour(Colour(MelissaUISettings::getSubColour()));
            g.fillRoundedRectangle(barX, y, barW, barH, cornerR);

            const float fillW = level * barW;
            if (fillW > 0.f)
            {
                Path clip;
                clip.addRoundedRectangle(barX, y, barW, barH, cornerR);
                g.saveState();
                g.reduceClipRegion(clip);
                g.setColour(accent.withAlpha(0.75f));
                g.fillRect(barX, y, fillW, barH);
                g.restoreState();
            }

            if (peak > 0.01f)
            {
                const float px = barX + peak * barW - 1.f;
                g.setColour(MelissaUISettings::getTextColour(0.6f));
                g.fillRect(px, y, 2.f, barH);
            }
        };

        drawBar(displayL_, peakL_, barTop);
        drawBar(displayR_, peakR_, barTop + barH + kGap);

        // Volume thumb
        const float tx = kBarMargin + value_ * barW;
        constexpr float thumbWidth = 8.f;
        g.setColour(accent.brighter(0.4f));
        g.fillRoundedRectangle(tx - thumbWidth / 2, 0.f, thumbWidth, h, thumbWidth / 2.f);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown())
            setValueFromX(e.x);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown())
            setValueFromX(e.x);
    }

    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        setValue(1.f, true);
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        setValue(juce::jlimit(0.01f, 1.f, value_ + wheel.deltaY * 0.05f), true);
    }

private:
    static constexpr float kGap       = 2.f;
    static constexpr float kBarMargin = 5.f;  // thumb extends this far above/below bars
    static constexpr int kPeakHoldFrames = 60;

    std::atomic<float> levelL_{0.f}, levelR_{0.f};
    float displayL_ = 0.f, displayR_ = 0.f;
    float peakL_    = 0.f, peakR_    = 0.f;
    int   peakHoldL_ = 0,  peakHoldR_ = 0;
    float value_ = 1.f;

    void setValueFromX(int x)
    {
        const float barW = getWidth() - kBarMargin * 2.f;
        const float v = juce::jlimit(0.01f, 1.f, (static_cast<float>(x) - kBarMargin) / barW);
        setValue(v, true);
    }

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
