//
//  MelissaButtonWithProgressBar.h
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"
#include "MelissaDataSource.h"

class MelissaButtonWithProgressBar : public juce::Component, public juce::Timer
{
public:
    MelissaButtonWithProgressBar(const juce::String& buttonText = "");
    ~MelissaButtonWithProgressBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    void timerCallback() override;

    void setButtonText(const juce::String& text);
    juce::String getButtonText() const { return buttonText_; }
    void setEnabled(bool enabled);
    bool isEnabled() const { return isEnabled_; }
    void setTooltip(const juce::String& tooltip) { tooltipText_ = tooltip; }
    juce::String getTooltip() const { return tooltipText_; }

    void setCallback(std::function<void()> callback) { onClick_ = callback; }

    // Progress bar control
    void setProgress(float progress); // 0.0 to 1.0
    void startAnimation();
    void stopAnimation();
    void showProgressBar(bool show);

private:
    juce::String buttonText_;
    juce::String tooltipText_;
    bool isMouseOver_;
    bool isMouseDown_;
    bool isEnabled_;
    bool isAnimating_;
    bool showProgressBar_;

    float progress_;
    int animationPhase_;
    std::function<void()> onClick_;

    static constexpr int kFPS = 30;
    static constexpr float kAnimationSpeed = 2.0f;
    static constexpr int kProgressBarHeight = 3;
    static constexpr int kProgressBarMargin = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaButtonWithProgressBar)
};
