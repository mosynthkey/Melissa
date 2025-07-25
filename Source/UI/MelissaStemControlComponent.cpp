//
//  MelissaStemControlComponent.cpp
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#include "MelissaDataSource.h"
#include "MelissaOptionDialog.h"
#include "MelissaModalDialog.h"
#include "MelissaPopupMessageComponent.h"
#include "MelissaStemControlComponent.h"

using namespace juce;

enum
{
    kMelissaStemButtonGroup = 4000,
};



namespace
{
static const String stemNames[] = { "Inst.", "Vocal", "Piano", "Guitar", "Bass", "Drums", "Others" };
static const String errorMessages[] =
{
    "", // kStemProviderResult_Success,
    "stem_err_failed_to_read_source_file", // kStemProviderResult_FailedToReadSourceFile,
    "stem_err_failed_to_initialize", // kStemProviderResult_FailedToInitialize,
    "stem_err_failed_to_split", // kStemProviderResult_FailedToSplit,
    "stem_err_failed_to_export", // kStemProviderResult_FailedToExport,
    "stem_err_failed_to_export", // kStemProviderResult_Interrupted,
    "stem_err_notsupported", // kStemProviderResult_NotSupported,
    "stem_err_unknown", // kStemProviderResult_UnknownError
};

constexpr int titleWidth = 52;
constexpr int kControlX = 40;
enum { kGroup_SoloMix = 10000 };
}

class MelissaStemControlComponent::ProgressBar : public Component, public Timer
{
public:
    ProgressBar() : progress_(0), estimatedTime_(0), startTime_(0), count_(0), 
                   progressPercentage_(0), progressMessage_("")
    {
        startTimerHz(kFps);
    }
    
    void paint(Graphics& g) override
    {
        const auto isDark = MelissaUISettings::isDarkMode;
        
        float progress = 0.f;
        
        // Use progress percentage if available, otherwise use estimated time
        if (progressPercentage_ > 0)
        {
            progress = progressPercentage_ / 100.0f;
        }
        else if (estimatedTime_ != 0)
        {
            progress = (clock() - startTime_) / estimatedTime_;
            if (progress > 1.f) progress = 1.f;
            if (progress < progress_) progress = progress_;
        }
        
        g.setColour(isDark ? Colours::white.withAlpha(0.1f) : Colours::black.withAlpha(0.1f));
        g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), getHeight() / 2.f);
        
        const float alpha = (sin(2 * M_PI * count_ / static_cast<float>(kFps * kSpeed)) + 1.f) / 2.f;
        g.setColour(MelissaUISettings::getAccentColour(alpha * 0.5f + 0.5f));
        g.fillRoundedRectangle(0, 0, getWidth() * progress, getHeight(), getHeight() / 2.f);
        
        progress_ = progress;
    }
    
    void visibilityChanged() override
    {
        if (isVisible() && !isTimerRunning())
        {
            startTimerHz(kFps);
        }
        else if (!isVisible())
        {
            stopTimer();
        }
    }
    
    void timerCallback() override
    {
        ++count_;
        if (count_ >= (kFps * kSpeed)) count_ = 0;
        
        repaint();
    }
    
    void setEstimatedTime(float estimatedTime)
    {
        estimatedTime_ = estimatedTime;
    }
    
    void setProgress(float progressPercentage, const juce::String& message)
    {
        progressPercentage_ = progressPercentage;
        progressMessage_ = message;
    }
    
    void startProgress()
    {
        startTime_ = clock();
        estimatedTime_ = 0;
        progressPercentage_ = 0;
        progressMessage_ = "";
    }
    
    void reset()
    {
        progress_ = 0.f;
        progressPercentage_ = 0;
        progressMessage_ = "";
        repaint();
    }
    
    juce::String getMessage() const { return progressMessage_; }
    
private:
    float progress_, estimatedTime_;
    float progressPercentage_;
    juce::String progressMessage_;
    clock_t startTime_;
    int count_;
    static const inline int kFps = 20;
    static const inline int kSpeed = 2;
};

MelissaStemControlComponent::AnimatedStemButton::AnimatedStemButton() :
    isMouseOver_(false),
    isMouseDown_(false),
    isEnabled_(true),
    isAnimating_(false),
    animationPhase_(0)
{
    startTimer(1000 / kFPS);
}

MelissaStemControlComponent::AnimatedStemButton::~AnimatedStemButton()
{
    stopTimer();
}

void MelissaStemControlComponent::AnimatedStemButton::paint(juce::Graphics& g)
{
    using namespace juce;
    
    // Background
    const auto bounds = getLocalBounds().toFloat();
    const bool isDark = MelissaUISettings::isDarkMode;
    
    g.setColour(juce::Colour(MelissaUISettings::getSubColour()));
    g.fillRoundedRectangle(bounds, getHeight() / 2);
    
    if (!isEnabled_)
    {
        // When disabled
        g.setColour(MelissaUISettings::getTextColour(0.5f));
    }
    else if (isMouseDown_)
    {
        // When pressed
        g.setColour(MelissaUISettings::getAccentColour(0.2f));
        g.fillRoundedRectangle(bounds, getHeight() / 2);
        g.setColour(MelissaUISettings::getTextColour(1.0f));
    }
    else if (isMouseOver_)
    {
        // When hovered
        g.setColour(MelissaUISettings::getAccentColour(0.1f));
        g.fillRoundedRectangle(bounds, getHeight() / 2);
        g.setColour(MelissaUISettings::getTextColour(0.9f));
    }
    else
    {
        // Normal state
        g.setColour(MelissaUISettings::getTextColour(0.8f));
    }
    
    // Animate when processing
    if (isAnimating_)
    {
        // Create gradient animation
        const float gradientPos = (sin(static_cast<float>(animationPhase_) / (kFPS * kAnimationSpeed) * 2.f) + 1.f) / 2.f;
        
        ColourGradient gradient;
        
        // Create moving gradient
        gradient.point1 = Point<float>(bounds.getX() + bounds.getWidth() * (gradientPos - 0.5f), bounds.getY());
        gradient.point2 = Point<float>(bounds.getX() + bounds.getWidth() * (gradientPos + 0.5f), bounds.getBottom());
        
        gradient.addColour(0.0, MelissaUISettings::getAccentColour(0.1f));
        gradient.addColour(0.5, MelissaUISettings::getAccentColour(0.4f));
        gradient.addColour(1.0, MelissaUISettings::getAccentColour(0.1f));
        
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, getHeight() / 2);
    }
    
    // Draw text
    g.setColour(MelissaUISettings::getTextColour(0.8f));
    g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    g.drawText(buttonText_, 0, 0, getWidth(), getHeight(), juce::Justification::centred);
}

void MelissaStemControlComponent::AnimatedStemButton::resized()
{
    // Nothing to do
}

void MelissaStemControlComponent::AnimatedStemButton::mouseDown(const juce::MouseEvent& e)
{
    if (!isEnabled_) return;
    
    isMouseDown_ = true;
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::mouseUp(const juce::MouseEvent& e)
{
    if (!isEnabled_) return;
    
    isMouseDown_ = false;
    
    if (isMouseOver_ && onClick != nullptr)
    {
        onClick();
    }
    
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::mouseEnter(const juce::MouseEvent& e)
{
    isMouseOver_ = true;
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::mouseExit(const juce::MouseEvent& e)
{
    isMouseOver_ = false;
    isMouseDown_ = false;
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::timerCallback()
{
    if (isAnimating_)
    {
        ++animationPhase_;
        repaint();
    }
}

void MelissaStemControlComponent::AnimatedStemButton::setButtonText(const juce::String& text)
{
    buttonText_ = text;
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::setEnabled(bool enabled)
{
    isEnabled_ = enabled;
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::startAnimation()
{
    isAnimating_ = true;
    animationPhase_ = 0;
    repaint();
}

void MelissaStemControlComponent::AnimatedStemButton::stopAnimation()
{
    isAnimating_ = false;
    repaint();
}

MelissaStemControlComponent::MelissaStemControlComponent() : mode_(kMode_NoStems), status_(kStemProviderStatus_Ready)
{
    auto model = MelissaModel::getInstance();
    
    // Solo/Mix Toggle Buttons
    mixButton_ = std::make_unique<ToggleButton>();
    mixButton_->setButtonText("Mix");
    mixButton_->setTooltip(TRANS("stem_mix_button"));
    mixButton_->setRadioGroupId(kGroup_SoloMix);
    mixButton_->setToggleState(false, dontSendNotification);
    mixButton_->onClick = [&, model]()
    {
        mode_ = kMode_Mix;
        model->setPlayPart(kPlayPart_Custom);
        updateAndArrangeControls();
    };
    addAndMakeVisible(mixButton_.get());
    
    soloButton_ = std::make_unique<ToggleButton>();
    soloButton_->setButtonText("Solo");
    soloButton_->setTooltip(TRANS("stem_solo_button"));
    soloButton_->setRadioGroupId(kGroup_SoloMix);
    soloButton_->setToggleState(true, dontSendNotification);
    soloButton_->onClick = [&, model]()
    {
        mode_ = kMode_Solo;
        model->setPlayPart(kPlayPart_All);
        updateAndArrangeControls();
    };
    addAndMakeVisible(soloButton_.get());
    
    // Custom animated stem button
    createStemsButton_ = std::make_unique<AnimatedStemButton>();
    createStemsButton_->setCallback([&]()
    {
        if (MelissaStemProvider::getInstance()->isThreadRunning())
        {
            const std::vector<String> options = { TRANS("ok"), TRANS("cancel") };
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("cancel_creating_stems"), options, [&](size_t yesno) {
                if (yesno == 1 /* cancel */ ) return;
                
                MelissaStemProvider::getInstance()->signalThreadShouldExit();
                createStemsButton_->setButtonText(TRANS("cancel_separating"));
                createStemsButton_->setEnabled(false);
            });
            MelissaModalDialog::show(dialog, TRANS("separation_of_music"));
        }
        else
        {
            auto component = std::make_shared<MelissaStemSeparationSelectComponent>();
            component->setSize(580, 280);
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("separation_of_music"));
        }
    });
    addAndMakeVisible(createStemsButton_.get());
    
    progressBar_ = std::make_unique<ProgressBar>();
    addAndMakeVisible(progressBar_.get());
    
    // Solo Buttons
    auto createAndAddTextButton = [&](const String& text)
    {
        auto b = std::make_unique<ToggleButton>(text);
        b->setTooltip(TRANS("stems_" + text.toLowerCase()));
        b->setButtonText(text);
        b->setRadioGroupId(kMelissaStemButtonGroup);
        b->setClickingTogglesState(true);
        b->setLookAndFeel(&stemToggleButtonLaf_);
        addAndMakeVisible(b.get());
        return b;
    };
    
    allButton_ = createAndAddTextButton("All");
    allButton_->setToggleState(true, dontSendNotification);
    allButton_->onClick = [&, model]() { model->setPlayPart(kPlayPart_All); };
    for (int buttonIndex = 0; buttonIndex < kNumStemSoloButtons; ++buttonIndex)
    {
        stemSwitchButtons_[buttonIndex] = createAndAddTextButton(stemNames[buttonIndex]);
        stemSwitchButtons_[buttonIndex]->setVisible(false);
        stemSwitchButtons_[buttonIndex]->onClick = [&, buttonIndex, model]() {
            model->setPlayPart(static_cast<PlayPart>(buttonIndex + kPlayPart_Instruments));
        };
    }
    
    // Mix Knobs
    for (int partIndex = 0; partIndex < kNumMixKnobs; ++partIndex)
    {
        auto k = std::make_unique<Slider>(Slider::RotaryVerticalDrag, Slider::NoTextBox);
        k->setTooltip(TRANS((stemNames[partIndex + 1]).toLowerCase() + "_custom_volume"));
        k->setRange(-1.f, 1.0f);
        k->setDoubleClickReturnValue(true, 0.f);
        k->setValue(0.f);
        k->onValueChange = [&, this, partIndex]()
        {
            const auto value = partKnobs_[partIndex]->getValue();
            MelissaModel::getInstance()->setCustomPartVolume(static_cast<CustomPartVolume>(partIndex), value);
        };
        addAndMakeVisible(k.get());
        partKnobs_[partIndex] = std::move(k);
    }
    
    updateAndArrangeControls();
    
    MelissaStemProvider::getInstance()->addListener(this);
    model->addListener(this);
}

MelissaStemControlComponent::~MelissaStemControlComponent()
{
    allButton_->setLookAndFeel(nullptr);
    for (auto& b : stemSwitchButtons_) b->setLookAndFeel(nullptr);
}

void MelissaStemControlComponent::resized()
{
    updateAndArrangeControls();
}

void MelissaStemControlComponent::paint(Graphics& g)
{
    auto dataSource = MelissaDataSource::getInstance();
    
    g.setColour(MelissaUISettings::getTextColour());
    
    g.setFont(dataSource->getFont(MelissaDataSource::Global::kFontSize_Sub));
    g.drawText("Part", 0, 5, titleWidth, 20, Justification::centredLeft);
    
    const int labelWidth = 50;
    
    const bool isStemAvailable = (status_ == kStemProviderStatus_Available_Full || status_ == kStemProviderStatus_Available_NoGuitar);
    if (!isStemAvailable)
    {
        g.setColour(MelissaUISettings::getSubColour());
        g.fillRoundedRectangle(kControlX, 30, getWidth() - kControlX, 30, 15);
    }
    else if (isStemAvailable && mode_ == kMode_Mix)
    {
        g.setFont(dataSource->getFont(MelissaDataSource::Global::kFontSize_Small));
        const int knobMargin = (getWidth() - kControlX) / kNumMixKnobs;
        int cx = kControlX + knobMargin / 2;
        for (int partIndex = 0; partIndex < kNumMixKnobs; ++partIndex)
        {
            const auto stemAvailable = MelissaDataSource::getInstance()->isStemAvailable(static_cast<PlayPart>(partIndex));
            g.setColour(MelissaUISettings::getTextColour().withAlpha(stemAvailable ? 1.f : 0.2f));
            g.drawText(stemNames[partIndex + 1], cx - labelWidth / 2, 5, labelWidth, 20, Justification::centred);
            cx += knobMargin;
        }
    }
}

void MelissaStemControlComponent::playPartChanged(PlayPart playPart)
{
    toggleStems(playPart);
}

void MelissaStemControlComponent::customPartVolumeChanged(CustomPartVolume part, float volume)
{
    const int partIndex = static_cast<int>(part);
    partKnobs_[partIndex]->setValue(volume);
}

void MelissaStemControlComponent::stemProviderStatusChanged(StemProviderStatus status)
{
    status_ = status;
    updateAndArrangeControls();
    
    if (status == kStemProviderStatus_Processing)
    {
        progressBar_->startProgress();
        createStemsButton_->startAnimation();
    }
}

void MelissaStemControlComponent::stemProviderResultReported(StemProviderResult result)
{
    updateAndArrangeControls();
    progressBar_->reset();
    createStemsButton_->stopAnimation();
}

void MelissaStemControlComponent::stemProviderEstimatedTimeReported(float estimatedTime)
{
    progressBar_->setEstimatedTime(estimatedTime);
}

void MelissaStemControlComponent::stemProviderProgressReported(float progressPercentage, const juce::String& message)
{
    progressBar_->setProgress(progressPercentage, message);
}

void MelissaStemControlComponent::updateAndArrangeControls()
{
    if (status_ == kStemProviderStatus_Ready)
    {
        createStemsButton_->setButtonText(TRANS("click_to_separate"));
        createStemsButton_->setEnabled(true);
        createStemsButton_->stopAnimation();
    }
    else if (status_ == kStemProviderStatus_NotAvailable)
    {
        auto result = MelissaStemProvider::getInstance()->getStemProviderResult();
        createStemsButton_->setButtonText(TRANS(errorMessages[result]));
        createStemsButton_->setEnabled(false);
        createStemsButton_->stopAnimation();
    }
    else if (status_ == kStemProviderStatus_Processing)
    {
        createStemsButton_->setButtonText(TRANS("separating_click_to_cancel"));
        createStemsButton_->setEnabled(true);
        createStemsButton_->startAnimation();
    }
    
    const bool isAvailable = (status_ == kStemProviderStatus_Available_Full || status_ == kStemProviderStatus_Available_NoGuitar);
    createStemsButton_->setVisible(!isAvailable);
    progressBar_->setVisible(status_ == kStemProviderStatus_Processing);
    
    allButton_->setVisible(isAvailable && mode_ == kMode_Solo);
    for (auto& b : stemSwitchButtons_) b->setVisible(isAvailable && mode_ == kMode_Solo);
    for (auto&k : partKnobs_) k->setVisible(isAvailable && mode_ == kMode_Mix);
    
    constexpr int kXMargin = 4;
    constexpr int kYMargin = 2;
    constexpr int kButtonHeight = 24;
    
    // Toggle Buttons
    soloButton_->setBounds(0, 26, 40, 16);
    mixButton_ ->setBounds(0, 44, 40, 16);
    
    // Message Button
    const int createButtonOrStatusWidth = getWidth() - kControlX;
    createStemsButton_->setBounds(kControlX, 30, createButtonOrStatusWidth, 30);
    
    // Solo Buttons
    const int kSoloButtonWidth = (getWidth() - kControlX - kXMargin * 3 - 10) / 4;
    
    int x = 10;
    for (int buttonIndex = 0; buttonIndex < (kNumStemSoloButtons + 1); ++buttonIndex)
    {
        const int column = buttonIndex % 4;
        const int row = buttonIndex / 4;
        
        const int buttonX = kControlX + kXMargin + column * (kSoloButtonWidth + kXMargin);
        const int buttonY = x + (kYMargin + kButtonHeight) * row;
        
        if (buttonIndex == 0)
        {
            allButton_->setBounds(buttonX, buttonY, kSoloButtonWidth, kButtonHeight);
        }
        else
        {
            const int stemTypeIndex = buttonIndex - 1;
            stemSwitchButtons_[stemTypeIndex]->setBounds(buttonX, buttonY, kSoloButtonWidth, kButtonHeight);
            
            if (1 /* vocals */ <= stemTypeIndex)
            {
                const auto stemAvailalbe = MelissaDataSource::getInstance()->isStemAvailable(static_cast<PlayPart>(stemTypeIndex - 1));
                stemSwitchButtons_[stemTypeIndex]->setEnabled(stemAvailalbe);
                stemSwitchButtons_[stemTypeIndex]->setAlpha(stemAvailalbe ? 1 : 0.2f);
            }
        }
    }
    
    x = kControlX;
    constexpr int kProgressBarMargin = 10;
    progressBar_->setBounds(x + kProgressBarMargin, createStemsButton_->getBottom() - 6, createButtonOrStatusWidth - kProgressBarMargin * 2, 2);
    
    // Mix Knobs
    constexpr int knobSize = 36;
    const int knobMargin = (getWidth() - kControlX) / kNumMixKnobs;
    x = kControlX + knobMargin / 2;
    for (int partIndex = 0; partIndex < kNumMixKnobs; ++partIndex)
    {
        partKnobs_[partIndex]->setBounds(x - knobSize / 2, 24, knobSize, knobSize);
        partKnobs_[partIndex]->setEnabled(MelissaDataSource::getInstance()->isStemAvailable(static_cast<PlayPart>(partIndex)));
        x += knobMargin;
    }
    
    repaint();
}

void MelissaStemControlComponent::toggleStems(PlayPart playPart)
{
    if (status_ == kStemProviderStatus_Available_Full || status_ == kStemProviderStatus_Available_NoGuitar)
    {
        mixButton_->setToggleState(playPart == kPlayPart_Custom, dontSendNotification);
        soloButton_->setToggleState(playPart != kPlayPart_Custom, dontSendNotification);
        
        if (playPart == kPlayPart_Custom)
        {
            mode_ = kMode_Mix;
        }
        else
        {
            mode_ = kMode_Solo;
            allButton_->setToggleState(playPart == kPlayPart_All, dontSendNotification);
            for (int buttonIndex = 0; buttonIndex < kNumStemSoloButtons; ++buttonIndex)
            {
                stemSwitchButtons_[buttonIndex]->setToggleState(playPart == (kPlayPart_Instruments + buttonIndex), dontSendNotification);
            }
        }
    }
    else
    {
        allButton_->setToggleState(false, dontSendNotification);
        for (int buttonIndex = 0; buttonIndex < kNumStemSoloButtons; ++buttonIndex)
        {
            stemSwitchButtons_[buttonIndex]->setToggleState(false, dontSendNotification);
        }
    }
    
    updateAndArrangeControls();
}
