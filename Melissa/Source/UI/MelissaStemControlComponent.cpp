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

enum
{
    kMelissaStemButtonGroup = 4000,
};

static const String stemNames[] = { "Inst.", "Vo.", "Piano", "Bass", "Drums", "Others" };

class MelissaStemControlComponent::ProgressBar : public Component, public Timer
{
public:
    ProgressBar() : progress_(0), count_(0)
    {
        startTimerHz(kFps);
    }
    
    void paint(Graphics& g) override
    {
        const auto isDark = MelissaUISettings::isDarkMode;
        
        g.setColour(isDark ? Colours::white.withAlpha(0.1f) : Colours::black.withAlpha(0.1f));
        g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), getHeight() / 2.f);
        
        const float alpha = (sin(2 * M_PI * count_ / static_cast<float>(kFps * kSpeed)) + 1.f) / 2.f;
        g.setColour(MelissaUISettings::getAccentColour(alpha * 0.5f + 0.5f));
        g.fillRoundedRectangle(0, 0, getWidth() * progress_, getHeight(), getHeight() / 2.f);
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
    
    void setProgress(float progress)
    {
        progress_ = progress;
        repaint();
    }
    
    void reset()
    {
        progress_ = 0.f;
        repaint();
    }
    
private:
    float progress_;
    int count_;
    static const inline int kFps = 20;
    static const inline int kSpeed = 2;
};

MelissaStemControlComponent::MelissaStemControlComponent() : status_(kStemProviderStatus_Ready)
{
    MelissaStemProvider::getInstance()->addListener(this);
    auto model = MelissaModel::getInstance();
    model->addListener(this);
    
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
    
    createStemsButton_ = std::make_unique<TextButton>();
    createStemsButton_->setLookAndFeel(&simpleTextButtonLaf_);
    createStemsButton_->onClick = [&]()
    {
        if (MelissaStemProvider::getInstance()->isThreadRunning())
        {
            const std::vector<String> options = { TRANS("ok"), TRANS("cancel") };
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("cancel_creating_stems"), options, [&](size_t yesno) {
                if (yesno == 1 /* no */ ) return;
                
                MelissaStemProvider::getInstance()->signalThreadShouldExit();
                createStemsButton_->setButtonText(TRANS("cancel_separating"));
                createStemsButton_->setEnabled(false);
            });
            MelissaModalDialog::show(dialog, TRANS("separation_of_music"));
        }
        else
        {
            const std::vector<String> options = { TRANS("ok"), TRANS("cancel") };
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("before_creating_stems"), options, [&](size_t yesno) {
                if (yesno == 1 /* no */ ) return;
                
                MelissaStemProvider::getInstance()->requestStems(File(MelissaDataSource::getInstance()->getCurrentSongFilePath()));
            });
            MelissaModalDialog::show(dialog, TRANS("separation_of_music"));
        }
    };
    addAndMakeVisible(createStemsButton_.get());
    
    progressBar_ = std::make_unique<ProgressBar>();
    addAndMakeVisible(progressBar_.get());
    
    allButton_ = createAndAddTextButton("All");
    allButton_->setToggleState(true, dontSendNotification);
    allButton_->onClick = [&, model]() { model->setPlayPart(kStemType_All); };
    for (int stemTypeIndex = 0; stemTypeIndex < kNumStemTypes; ++stemTypeIndex)
    {
        stemSwitchButtons_[stemTypeIndex] = createAndAddTextButton(stemNames[stemTypeIndex]);
        stemSwitchButtons_[stemTypeIndex]->setVisible(false);
        stemSwitchButtons_[stemTypeIndex]->onClick = [&, stemTypeIndex, model]() { model->setPlayPart(static_cast<StemType>(stemTypeIndex)); };
    }
    
    updateAndArrangeControls();
}

MelissaStemControlComponent::~MelissaStemControlComponent()
{
    createStemsButton_->setLookAndFeel(nullptr);
    allButton_->setLookAndFeel(nullptr);
    for (auto& b : stemSwitchButtons_) b->setLookAndFeel(nullptr);
}

void MelissaStemControlComponent::resized()
{
    updateAndArrangeControls();
}

void MelissaStemControlComponent::paint(Graphics& g)
{
    const auto b = getLocalBounds();
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(b.toFloat(), b.getHeight() / 2);
}

void MelissaStemControlComponent::playPartChanged(StemType playPart)
{
    toggleStems(playPart);
}

void MelissaStemControlComponent::stemProviderStatusChanged(StemProviderStatus status)
{
    status_ = status;
    updateAndArrangeControls();
}

void MelissaStemControlComponent::stemProviderResultReported(StemProviderResult result)
{
    updateAndArrangeControls();
    progressBar_->reset();
}

void MelissaStemControlComponent::stemProviderProgressReported(float progress)
{
    progressBar_->setProgress(progress);
}

void MelissaStemControlComponent::updateAndArrangeControls()
{
    constexpr int kMargin = 4;
    constexpr int yMargin = 4;
    constexpr int kButtonHeight = 30 - yMargin * 2;
    
    int x = kMargin;
    allButton_->setBounds(x, yMargin, 40, kButtonHeight);
    const int createButtonOrStatusWidth = getWidth() - kMargin * 3 - allButton_->getWidth();
    
    x += (allButton_->getWidth() + kMargin);
    createStemsButton_->setBounds(x, yMargin, createButtonOrStatusWidth, kButtonHeight);
    constexpr int kProgressBarMargin = 10;
    progressBar_->setBounds(x + kProgressBarMargin, createStemsButton_->getBottom() - 2, createButtonOrStatusWidth - kProgressBarMargin * 2, 2);
    
    Font font = MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub);
    int totalNameLength = 0;
    x = allButton_->getRight() + kMargin;
    for (auto& name : stemNames) totalNameLength += font.getStringWidth(name);
    const float widthUnit = static_cast<float>((getWidth() - x) - kMargin - kMargin * (kNumStemTypes - 1)) / totalNameLength;
    for (int stemTypeIndex = 0; stemTypeIndex < kNumStemTypes; ++stemTypeIndex)
    {
        int buttonWidth = static_cast<int>(widthUnit * font.getStringWidth(stemNames[stemTypeIndex]));
        stemSwitchButtons_[stemTypeIndex]->setBounds(x, yMargin, buttonWidth, kButtonHeight);
        x += (buttonWidth + kMargin);
    }
    
    for (auto& b : stemSwitchButtons_) b->setVisible(status_ == kStemProviderStatus_Available);
    createStemsButton_->setVisible(status_ != kStemProviderStatus_Available);
    if (status_ != kStemProviderStatus_Available) allButton_->setToggleState(true, dontSendNotification);
    progressBar_->setVisible(status_ == kStemProviderStatus_Processing);
    
    if (status_ == kStemProviderStatus_Ready)
    {
        createStemsButton_->setButtonText(TRANS("click_to_separate"));
        createStemsButton_->setEnabled(true);
    }
    else if (status_ == kStemProviderStatus_NotAvailable)
    {
        createStemsButton_->setButtonText(TRANS("couldnt_separate"));
        createStemsButton_->setEnabled(false);
    }
    else if (status_ == kStemProviderStatus_Processing)
    {
        createStemsButton_->setButtonText(TRANS("separating_click_to_cancel"));
        createStemsButton_->setEnabled(true);
    }
}

void MelissaStemControlComponent::toggleStems(int stemIndex)
{
    if (status_ == kStemProviderStatus_Available)
    {
        allButton_->setToggleState(stemIndex == kStemType_All, dontSendNotification);
        for (int buttonIndex = 0; buttonIndex < kNumStemTypes; ++buttonIndex)
        {
            stemSwitchButtons_[buttonIndex]->setToggleState(buttonIndex == stemIndex, dontSendNotification);
        }
    }
    else
    {
        allButton_->setToggleState(true, dontSendNotification);
        for (int buttonIndex = 0; buttonIndex < kNumStemTypes; ++buttonIndex)
        {
            stemSwitchButtons_[buttonIndex]->setToggleState(buttonIndex == stemIndex, dontSendNotification);
        }
    }
}
