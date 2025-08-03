//
//  MelissaWaveformControlPopupComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaWaveformControlPopupComponent.h"
#include "MelissaUISettings.h"
#include "MelissaDataSource.h"
#include "MelissaUtility.h"
#include "BinaryData.h"
#include <iostream>

using namespace juce;

namespace
{
// Define margins as constexpr
constexpr int kMargin = 10;
constexpr int kSpacing = 20;
constexpr int kSliderWidth = 140;
constexpr int kSliderHeight = 30;
constexpr int kButtonWidth = 60;
constexpr int kButtonHeight = 30;
constexpr int kToggleWidth = 40;
constexpr int kToggleHeight = 20;
constexpr int kCloseButtonSize = 18;
}

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

    autoLabel_ = std::make_unique<Label>("autoLabel", "Auto Snap");
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
        auto model = MelissaModel::getInstance();
        model->snapLoopRangeToDownbeat();
    };
    addAndMakeVisible(loopSnapButton_.get());

    followToggleButton_ = std::make_unique<ToggleButton>("");
    followToggleButton_->setTooltip("Follow Playing Position");
    followToggleButton_->setToggleState(false, dontSendNotification);
    followToggleButton_->setLookAndFeel(&slideToggleButtonLaf_);
    followToggleButton_->onClick = [this]()
    {
        auto* dataSource = MelissaDataSource::getInstance();
        dataSource->setWaveformFollow(followToggleButton_->getToggleState());
    };
    addAndMakeVisible(followToggleButton_.get());

    autoSnapToggleButton_ = std::make_unique<ToggleButton>("");
    autoSnapToggleButton_->setTooltip("Snap to Beats while Dragging");
    autoSnapToggleButton_->setToggleState(false, dontSendNotification);
    autoSnapToggleButton_->setLookAndFeel(&slideToggleButtonLaf_);
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
    followToggleButton_->setLookAndFeel(nullptr);
    autoSnapToggleButton_->setLookAndFeel(nullptr);
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
    g.fillAll(fillColor);
    
    // Draw the border
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds.reduced(1.f), cornerRadius, 2.0f);
}

void MelissaWaveformControlPopupComponent::resized()
{
    // Calculate label widths using MelissaUtility
    auto* dataSource = MelissaDataSource::getInstance();
    const auto font = dataSource->getFont(MelissaDataSource::Global::kFontSize_Sub);
    const auto mainFont = dataSource->getFont(MelissaDataSource::Global::kFontSize_Main);
    
    const int kWaveformLabelWidth = MelissaUtility::getStringSize(mainFont, "Waveform").first;
    const int kZoomLabelWidth = MelissaUtility::getStringSize(font, "Zoom").first;
    const int kFollowLabelWidth = MelissaUtility::getStringSize(font, "Follow").first;
    const int kAutoLabelWidth = MelissaUtility::getStringSize(font, "Auto Snap").first;
    
    auto bounds = getLocalBounds().reduced(kMargin);
    
    // Cancel (Close) button on the left
    auto closeArea = bounds.removeFromLeft(kCloseButtonSize);
    closeButton_->setBounds(closeArea.getCentreX() - kCloseButtonSize/2, closeArea.getCentreY() - kCloseButtonSize/2, kCloseButtonSize, kCloseButtonSize);
    
    bounds.removeFromLeft(kSpacing);
    
    // Waveform label (wider to fit text)
    auto waveformLabelArea = bounds.removeFromLeft(kWaveformLabelWidth);
    waveformLabel_->setBounds(waveformLabelArea);
    
    bounds.removeFromLeft(kSpacing);
    
    // Zoom label
    auto zoomLabelArea = bounds.removeFromLeft(kZoomLabelWidth);
    zoomLabel_->setBounds(zoomLabelArea);
    
    bounds.removeFromLeft(kSpacing);
    
    // 1x (Reset) button (height 32)
    auto resetArea = bounds.removeFromLeft(kButtonWidth);
    zoomResetButton_->setBounds(resetArea.getX(), resetArea.getCentreY() - kButtonHeight/2, kButtonWidth, kButtonHeight);
    
    bounds.removeFromLeft(kSpacing);
    
    // Zoom slider (height 32)
    auto sliderArea = bounds.removeFromLeft(kSliderWidth);
    zoomSlider_->setBounds(sliderArea.getX(), sliderArea.getCentreY() - kSliderHeight/2, kSliderWidth, kSliderHeight);
    
    bounds.removeFromLeft(kSpacing);
    
    // Follow toggle (first)
    auto followArea = bounds.removeFromLeft(kToggleWidth);
    followToggleButton_->setBounds(followArea.getX(), followArea.getCentreY() - kToggleHeight/2, kToggleWidth, kToggleHeight);
    
    bounds.removeFromLeft(kSpacing / 2);
    
    // Follow label (after toggle)
    auto followLabelArea = bounds.removeFromLeft(kFollowLabelWidth);
    followLabel_->setBounds(followLabelArea);
    
    bounds.removeFromLeft(kSpacing);
    
    // Snap button (height 32)
    auto snapArea = bounds.removeFromLeft(kButtonWidth);
    loopSnapButton_->setBounds(snapArea.getX(), snapArea.getCentreY() - kButtonHeight/2, kButtonWidth, kButtonHeight);
    
    bounds.removeFromLeft(kSpacing);
    
    // Auto snap toggle (first)
    auto autoSnapArea = bounds.removeFromLeft(kToggleWidth);
    autoSnapToggleButton_->setBounds(autoSnapArea.getX(), autoSnapArea.getCentreY() - kToggleHeight/2, kToggleWidth, kToggleHeight);
    
    bounds.removeFromLeft(kSpacing / 2);
    
    // Auto label (after toggle)
    auto autoLabelArea = bounds.removeFromLeft(kAutoLabelWidth);
    autoLabel_->setBounds(autoLabelArea);
}

void MelissaWaveformControlPopupComponent::showPopup(Component* parent)
{
    if (!parent) 
    {
        return;
    }
    
    // Calculate label widths using MelissaUtility
    auto* dataSource = MelissaDataSource::getInstance();
    const auto font = dataSource->getFont(MelissaDataSource::Global::kFontSize_Sub);
    const auto mainFont = dataSource->getFont(MelissaDataSource::Global::kFontSize_Main);
    
    const int kWaveformLabelWidth = MelissaUtility::getStringSize(mainFont, "Waveform").first;
    const int kZoomLabelWidth = MelissaUtility::getStringSize(font, "Zoom").first;
    const int kFollowLabelWidth = MelissaUtility::getStringSize(font, "Follow").first;
    const int kAutoLabelWidth = MelissaUtility::getStringSize(font, "Auto Snap").first;
    
    const int popupWidth = (kMargin * 2) + kCloseButtonSize + (kSpacing * 8) + kWaveformLabelWidth + kZoomLabelWidth + kFollowLabelWidth + kAutoLabelWidth + kSliderWidth + (kButtonWidth * 2) + (kToggleWidth * 2) + (kSpacing / 2 * 2);
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
