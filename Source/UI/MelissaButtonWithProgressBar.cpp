//
//  MelissaButtonWithProgressBar.cpp
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#include "MelissaButtonWithProgressBar.h"

using namespace juce;

MelissaButtonWithProgressBar::MelissaButtonWithProgressBar(const String& buttonText)
    : buttonText_(buttonText),
      isMouseOver_(false),
      isMouseDown_(false),
      isEnabled_(true),
      isAnimating_(false),
      showProgressBar_(false),
      progress_(0.0f),
      animationPhase_(0)
{
    startTimer(1000 / kFPS);
}

MelissaButtonWithProgressBar::~MelissaButtonWithProgressBar()
{
    stopTimer();
}

void MelissaButtonWithProgressBar::paint(Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float cornerRadius = bounds.getHeight() / 2.f;

    // Background
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(bounds, cornerRadius);

    if (!isEnabled_)
    {
        // Disabled state - just draw with reduced alpha
    }
    else if (isMouseDown_)
    {
        g.setColour(MelissaUISettings::getAccentColour(0.2f));
        g.fillRoundedRectangle(bounds, cornerRadius);
    }
    else if (isMouseOver_)
    {
        g.setColour(MelissaUISettings::getAccentColour(0.1f));
        g.fillRoundedRectangle(bounds, cornerRadius);
    }

    // Animation when processing
    if (isAnimating_)
    {
        const float gradientPos = (sin(static_cast<float>(animationPhase_) / (kFPS * kAnimationSpeed) * 2.f * MathConstants<float>::pi) + 1.f) / 2.f;

        ColourGradient gradient;
        gradient.point1 = Point<float>(bounds.getX() + bounds.getWidth() * (gradientPos - 0.5f), bounds.getY());
        gradient.point2 = Point<float>(bounds.getX() + bounds.getWidth() * (gradientPos + 0.5f), bounds.getBottom());

        gradient.addColour(0.0, MelissaUISettings::getAccentColour(0.1f));
        gradient.addColour(0.5, MelissaUISettings::getAccentColour(0.4f));
        gradient.addColour(1.0, MelissaUISettings::getAccentColour(0.1f));

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, cornerRadius);
    }

    // Draw text
    float textAlpha = isEnabled_ ? 0.8f : 0.4f;
    if (isMouseDown_) textAlpha = 1.0f;
    else if (isMouseOver_) textAlpha = 0.9f;

    g.setColour(MelissaUISettings::getTextColour(textAlpha));
    g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    g.drawText(buttonText_, 0, 0, getWidth(), getHeight(), Justification::centred);

    // Draw progress bar at the bottom
    if (showProgressBar_ && progress_ > 0.0f)
    {
        const float progressBarY = bounds.getHeight() - kProgressBarHeight - 2;
        const float progressBarWidth = bounds.getWidth() - kProgressBarMargin * 2;

        // Background of progress bar
        g.setColour(Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(kProgressBarMargin, progressBarY, progressBarWidth, kProgressBarHeight, kProgressBarHeight / 2.f);

        // Progress fill with animation
        const float alpha = (sin(2.f * MathConstants<float>::pi * animationPhase_ / static_cast<float>(kFPS * kAnimationSpeed)) + 1.f) / 2.f;
        g.setColour(MelissaUISettings::getAccentColour(alpha * 0.5f + 0.5f));
        g.fillRoundedRectangle(kProgressBarMargin, progressBarY, progressBarWidth * progress_, kProgressBarHeight, kProgressBarHeight / 2.f);
    }
}

void MelissaButtonWithProgressBar::resized()
{
    // Nothing specific to do
}

void MelissaButtonWithProgressBar::mouseDown(const MouseEvent& e)
{
    if (!isEnabled_) return;

    isMouseDown_ = true;
    repaint();
}

void MelissaButtonWithProgressBar::mouseUp(const MouseEvent& e)
{
    if (!isEnabled_) return;

    isMouseDown_ = false;

    if (isMouseOver_ && onClick_ != nullptr)
    {
        onClick_();
    }

    repaint();
}

void MelissaButtonWithProgressBar::mouseEnter(const MouseEvent& e)
{
    isMouseOver_ = true;
    repaint();
}

void MelissaButtonWithProgressBar::mouseExit(const MouseEvent& e)
{
    isMouseOver_ = false;
    isMouseDown_ = false;
    repaint();
}

void MelissaButtonWithProgressBar::timerCallback()
{
    if (isAnimating_ || showProgressBar_)
    {
        ++animationPhase_;
        if (animationPhase_ >= static_cast<int>(kFPS * kAnimationSpeed))
            animationPhase_ = 0;
        repaint();
    }
}

void MelissaButtonWithProgressBar::setButtonText(const String& text)
{
    buttonText_ = text;
    repaint();
}

void MelissaButtonWithProgressBar::setEnabled(bool enabled)
{
    isEnabled_ = enabled;
    repaint();
}

void MelissaButtonWithProgressBar::setProgress(float progress)
{
    progress_ = jlimit(0.0f, 1.0f, progress);
    repaint();
}

void MelissaButtonWithProgressBar::startAnimation()
{
    isAnimating_ = true;
    animationPhase_ = 0;
}

void MelissaButtonWithProgressBar::stopAnimation()
{
    isAnimating_ = false;
    animationPhase_ = 0;
    repaint();
}

void MelissaButtonWithProgressBar::showProgressBar(bool show)
{
    showProgressBar_ = show;
    if (!show)
    {
        progress_ = 0.0f;
    }
    repaint();
}
