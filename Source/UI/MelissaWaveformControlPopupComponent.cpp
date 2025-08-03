//
//  MelissaWaveformControlPopupComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaWaveformControlPopupComponent.h"
#include "MelissaUISettings.h"
#include "MelissaDataSource.h"
#include "BinaryData.h"
#include <iostream>

using namespace juce;

MelissaWaveformControlPopupComponent::MelissaWaveformControlPopupComponent()
    : isPopupVisible_(false)
{
    auto* dataSource = MelissaDataSource::getInstance();
    dataSource->addListener(this);
    closeButtonDrawable_ = Drawable::createFromImageData(BinaryData::clear_svg, BinaryData::clear_svgSize);
    closeButtonHighlightedDrawable_ = Drawable::createFromImageData(BinaryData::clear_svg, BinaryData::clear_svgSize);
    closeButtonDrawable_->replaceColour(Colours::white, MelissaUISettings::getTextColour(0.4f));
    closeButtonHighlightedDrawable_->replaceColour(Colours::white, MelissaUISettings::getTextColour(0.8f));
    
    closeButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    closeButton_->setTooltip("Close");
    closeButton_->setImages(closeButtonDrawable_.get(), closeButtonHighlightedDrawable_.get());
    closeButton_->onClick = [this]()
    {
        if (onCloseClicked)
            onCloseClicked();
        hidePopup();
    };
    addAndMakeVisible(closeButton_.get());

    waveformLabel_ = std::make_unique<Label>("waveformLabel", "Waveform");
    waveformLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    waveformLabel_->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(waveformLabel_.get());

    zoomLabel_ = std::make_unique<Label>("zoomLabel", "Zoom");
    zoomLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    zoomLabel_->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(zoomLabel_.get());

    followLabel_ = std::make_unique<Label>("followLabel", "Follow");
    followLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    followLabel_->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(followLabel_.get());

    autoLabel_ = std::make_unique<Label>("autoLabel", "Auto");
    autoLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    autoLabel_->setJustificationType(Justification::centredLeft);
    addAndMakeVisible(autoLabel_.get());

    zoomSlider_ = std::make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
    zoomSlider_->setRange(1.0f, 20.0f);
    zoomSlider_->setDoubleClickReturnValue(true, 1.0f);
    zoomSlider_->setValue(1.0f);
    zoomSlider_->setLookAndFeel(&zoomSliderLaf_);
    zoomSlider_->onValueChange = [this]()
    {
        auto* dataSource = MelissaDataSource::getInstance();
        dataSource->setWaveformZoom(static_cast<float>(zoomSlider_->getValue()));
    };
    addAndMakeVisible(zoomSlider_.get());

    zoomResetButton_ = std::make_unique<TextButton>("1x");
    zoomResetButton_->setTooltip("Reset Zoom to 1x");
    zoomResetButton_->onClick = [this]()
    {
        auto* dataSource = MelissaDataSource::getInstance();
        dataSource->setWaveformZoom(1.0f);
    };
    addAndMakeVisible(zoomResetButton_.get());

    loopSnapButton_ = std::make_unique<TextButton>("Snap");
    loopSnapButton_->setTooltip("Snap Loop to Downbeats");
    loopSnapButton_->onClick = [this]()
    {
        if (onLoopSnapClicked)
            onLoopSnapClicked();
    };
    addAndMakeVisible(loopSnapButton_.get());

    followToggleButton_ = std::make_unique<ToggleButton>("");
    followToggleButton_->setTooltip("Follow Playing Position");
    followToggleButton_->setToggleState(false, dontSendNotification);
    followToggleButton_->onClick = [this]()
    {
        auto* dataSource = MelissaDataSource::getInstance();
        dataSource->setWaveformFollow(followToggleButton_->getToggleState());
    };
    addAndMakeVisible(followToggleButton_.get());

    autoSnapToggleButton_ = std::make_unique<ToggleButton>("");
    autoSnapToggleButton_->setTooltip("Snap to Beats while Dragging");
    autoSnapToggleButton_->setToggleState(false, dontSendNotification);
    autoSnapToggleButton_->onClick = [this]()
    {
        auto* dataSource = MelissaDataSource::getInstance();
        dataSource->setWaveformSnap(autoSnapToggleButton_->getToggleState());
    };
    addAndMakeVisible(autoSnapToggleButton_.get());

    setLookAndFeel(&laf_);
    setVisible(false);
    
    zoomSlider_->setValue(dataSource->getWaveformZoom(), dontSendNotification);
    followToggleButton_->setToggleState(dataSource->getWaveformFollow(), dontSendNotification);
    autoSnapToggleButton_->setToggleState(dataSource->getWaveformSnap(), dontSendNotification);
}

MelissaWaveformControlPopupComponent::~MelissaWaveformControlPopupComponent()
{
    auto* dataSource = MelissaDataSource::getInstance();
    dataSource->removeListener(this);
    
    zoomSlider_->setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void MelissaWaveformControlPopupComponent::paint(Graphics& g)
{
    const bool isDark = MelissaUISettings::isDarkMode;
    const auto fillColor = isDark ? MelissaUISettings::getMainColour() : MelissaUISettings::getSubColour();
    const auto borderColor = isDark ? MelissaUISettings::getSubColour() : MelissaUISettings::getMainColour();

    // Transparent background (no fillAll)
    
    // Draw rounded rectangle with fill and border
    const auto bounds = getLocalBounds().toFloat();
    const float cornerRadius = 10.0f;
    
    // Fill the rounded rectangle
    g.setColour(fillColor);
    g.fillRoundedRectangle(bounds, cornerRadius);
    
    // Draw the border
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds, cornerRadius, 2.0f);
}

void MelissaWaveformControlPopupComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    
    const int spacing = 8;
    const int labelWidth = 60;
    const int sliderWidth = 140;
    const int buttonWidth = 50;
    const int toggleWidth = 40;
    const int toggleHeight = 20;
    const int closeButtonSize = 18;
    
    // Cancel (Close) button on the left
    auto closeArea = bounds.removeFromLeft(closeButtonSize);
    closeButton_->setBounds(closeArea.getCentreX() - closeButtonSize/2, closeArea.getCentreY() - closeButtonSize/2, closeButtonSize, closeButtonSize);
    
    bounds.removeFromLeft(spacing);
    
    // Waveform label
    auto waveformLabelArea = bounds.removeFromLeft(labelWidth);
    waveformLabel_->setBounds(waveformLabelArea);
    
    bounds.removeFromLeft(spacing);
    
    // Zoom label
    auto zoomLabelArea = bounds.removeFromLeft(labelWidth);
    zoomLabel_->setBounds(zoomLabelArea);
    
    bounds.removeFromLeft(spacing);
    
    // 1x (Reset) button
    auto resetArea = bounds.removeFromLeft(buttonWidth);
    zoomResetButton_->setBounds(resetArea);
    
    bounds.removeFromLeft(spacing);
    
    // Zoom slider
    auto sliderArea = bounds.removeFromLeft(sliderWidth);
    zoomSlider_->setBounds(sliderArea);
    
    bounds.removeFromLeft(spacing);
    
    // Follow label
    auto followLabelArea = bounds.removeFromLeft(labelWidth);
    followLabel_->setBounds(followLabelArea);
    
    bounds.removeFromLeft(spacing / 2);
    
    // Follow toggle (40x20)
    auto followArea = bounds.removeFromLeft(toggleWidth);
    followToggleButton_->setBounds(followArea.getX(), followArea.getCentreY() - toggleHeight/2, toggleWidth, toggleHeight);
    
    bounds.removeFromLeft(spacing);
    
    // Snap button (no label needed)
    auto snapArea = bounds.removeFromLeft(buttonWidth);
    loopSnapButton_->setBounds(snapArea);
    
    bounds.removeFromLeft(spacing);
    
    // Auto label
    auto autoLabelArea = bounds.removeFromLeft(labelWidth);
    autoLabel_->setBounds(autoLabelArea);
    
    bounds.removeFromLeft(spacing / 2);
    
    // Auto snap toggle (40x20)
    auto autoSnapArea = bounds.removeFromLeft(toggleWidth);
    autoSnapToggleButton_->setBounds(autoSnapArea.getX(), autoSnapArea.getCentreY() - toggleHeight/2, toggleWidth, toggleHeight);
}

void MelissaWaveformControlPopupComponent::showPopup(Component* parent, Rectangle<int> buttonBounds)
{
    if (!parent) 
    {
        return;
    }
    
    const int popupWidth = 800;
    const int popupHeight = 40;
    
    auto parentBounds = parent->getLocalBounds();
    auto popupBounds = Rectangle<int>(
        parentBounds.getCentreX() - popupWidth / 2,
        60,
        popupWidth,
        popupHeight
    );
    
    setBounds(popupBounds);
    setVisible(true);
    setAlpha(0.0f);  // Start invisible
    isPopupVisible_ = true;
    toFront(true);
    
    // Animate fade-in only (no position/size change)
    animator_.animateComponent(this, popupBounds, 1.0f, 200, false, 1.0, 0.0);
}

void MelissaWaveformControlPopupComponent::hidePopup()
{
    if (!isPopupVisible_) return;
    
    isPopupVisible_ = false;
    
    // Animate fade-out only (no position/size change)
    auto currentBounds = getBounds();
    
    animator_.animateComponent(this, currentBounds, 0.0f, 150, false, 1.0, 0.0);
    
    // Hide the component after animation completes
    juce::Timer::callAfterDelay(150, [this]() {
        setVisible(false);
        setAlpha(1.0f);  // Reset alpha for next show
    });
}

void MelissaWaveformControlPopupComponent::syncWithControls(float zoomValue, bool autoSnapState, bool followState)
{
    zoomSlider_->setValue(zoomValue, dontSendNotification);
    autoSnapToggleButton_->setToggleState(autoSnapState, dontSendNotification);
    followToggleButton_->setToggleState(followState, dontSendNotification);
}

float MelissaWaveformControlPopupComponent::getCurrentZoomValue() const
{
    return static_cast<float>(zoomSlider_->getValue());
}

bool MelissaWaveformControlPopupComponent::getAutoSnapState() const
{
    return autoSnapToggleButton_->getToggleState();
}

bool MelissaWaveformControlPopupComponent::getFollowState() const
{
    return followToggleButton_->getToggleState();
}

void MelissaWaveformControlPopupComponent::waveformZoomChanged(float zoomValue)
{
    zoomSlider_->setValue(zoomValue, dontSendNotification);
}

void MelissaWaveformControlPopupComponent::waveformFollowChanged(bool followPlayingPosition)
{
    followToggleButton_->setToggleState(followPlayingPosition, dontSendNotification);
}

void MelissaWaveformControlPopupComponent::waveformSnapChanged(bool snapToBeats)
{
    autoSnapToggleButton_->setToggleState(snapToBeats, dontSendNotification);
}
