//
//  MelissaWaveformToolbarComponent.cpp
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#include "MelissaWaveformToolbarComponent.h"
#include "MelissaUISettings.h"
#include "MelissaDataSource.h"
#include "MelissaUtility.h"
#include "BinaryData.h"

using namespace juce;

namespace
{
constexpr int kMargin = 10;
constexpr int kSpacing = 20;
constexpr int kSliderWidth = 140;
constexpr int kSliderHeight = 30;
constexpr int kButtonWidth = 60;
constexpr int kButtonHeight = 30;
constexpr int kToggleWidth = 40;
constexpr int kToggleHeight = 20;
}

MelissaWaveformToolbarComponent::MelissaWaveformToolbarComponent()
{
    auto* dataSource = MelissaDataSource::getInstance();
    dataSource->addListener(this);

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
    setVisible(true);

    zoomSlider_->setValue(dataSource->getWaveformZoom(), dontSendNotification);
    followToggleButton_->setToggleState(dataSource->getWaveformFollow(), dontSendNotification);
    autoSnapToggleButton_->setToggleState(dataSource->getWaveformSnap(), dontSendNotification);
}

MelissaWaveformToolbarComponent::~MelissaWaveformToolbarComponent()
{
    auto* dataSource = MelissaDataSource::getInstance();
    dataSource->removeListener(this);

    zoomSlider_->setLookAndFeel(nullptr);
    followToggleButton_->setLookAndFeel(nullptr);
    autoSnapToggleButton_->setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void MelissaWaveformToolbarComponent::paint(Graphics& g)
{
    constexpr int kRound = 6;
    const auto colour = MelissaUISettings::getMainColour();

    g.setColour(colour);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), kRound);
}

void MelissaWaveformToolbarComponent::resized()
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

void MelissaWaveformToolbarComponent::waveformZoomChanged(float zoomValue)
{
    zoomSlider_->setValue(zoomValue, dontSendNotification);
}

void MelissaWaveformToolbarComponent::waveformFollowChanged(bool followPlayingPosition)
{
    followToggleButton_->setToggleState(followPlayingPosition, dontSendNotification);
}

void MelissaWaveformToolbarComponent::waveformSnapChanged(bool snapToBeats)
{
    autoSnapToggleButton_->setToggleState(snapToBeats, dontSendNotification);
}
