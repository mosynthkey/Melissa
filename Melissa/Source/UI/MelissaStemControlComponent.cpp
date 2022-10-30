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

static const String stemNames[] = { "Inst.", "Vo.", "Pf.", "Bass", "Drums", "Others" };

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
    /*
    if (result == kStemProviderResult_Interrupted)
    {
        status_ = MelissaStemProvider::getInstance()->getStemProviderStatus();
    }
     */
    
    updateAndArrangeControls();
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
    
    int totalNameLength = 0;
    x = allButton_->getRight() + kMargin;
    for (auto& name : stemNames) totalNameLength += name.length();
    const float widthUnit = static_cast<float>((getWidth() - x) - kMargin - kMargin * (kNumStemTypes - 1)) / totalNameLength;
    for (int stemTypeIndex = 0; stemTypeIndex < kNumStemTypes; ++stemTypeIndex)
    {
        int buttonWidth = static_cast<int>(widthUnit * stemNames[stemTypeIndex].length());
        stemSwitchButtons_[stemTypeIndex]->setBounds(x, yMargin, buttonWidth, kButtonHeight);
        x += (buttonWidth + kMargin);
    }
    
    for (auto& b : stemSwitchButtons_) b->setVisible(status_ == kStemProviderStatus_Available);
    createStemsButton_->setVisible(status_ != kStemProviderStatus_Available);
    if (status_ != kStemProviderStatus_Available) allButton_->setToggleState(true, dontSendNotification);
    
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
