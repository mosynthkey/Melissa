//
//  MelissaStemSeparationSelectComponent.cpp
//  Melissa
//

#include "MelissaDataSource.h"
#include "MelissaStemSeparationSelectComponent.h"
#include "MelissaStemProvider.h"
#include "MelissaModalDialog.h"

MelissaStemSeparationSelectComponent::MelissaStemSeparationSelectComponent()
{
    explanationLabel_.setText(TRANS("before_creating_stems"), juce::dontSendNotification);
    addAndMakeVisible(explanationLabel_);

    optionButtons_[kOption_Demucs] = std::make_unique<juce::TextButton>();
    optionButtons_[kOption_Demucs]->setButtonText(TRANS("separation_demucs_detail"));
    optionButtons_[kOption_Demucs]->onClick = [this]() 
    {
        MelissaStemProvider::getInstance()->requestStems(MelissaDataSource::getInstance()->getCurrentSongFilePath(), kSeparatorType_Demucs);
        MelissaModalDialog::close();
    };
    addAndMakeVisible(optionButtons_[kOption_Demucs].get());

    optionButtons_[kOption_Spleeter] = std::make_unique<juce::TextButton>();
    optionButtons_[kOption_Spleeter]->setButtonText(TRANS("separation_spleeter_detail"));
    optionButtons_[kOption_Spleeter]->onClick = [this]() 
    {
        MelissaStemProvider::getInstance()->requestStems(MelissaDataSource::getInstance()->getCurrentSongFilePath(), kSeparatorType_Spleeter);
        MelissaModalDialog::close();
    };
    addAndMakeVisible(optionButtons_[kOption_Spleeter].get());
    
    cancelButton_ = std::make_unique<juce::TextButton>("Cancel");
    cancelButton_->onClick = [this]()
    {
        MelissaModalDialog::close();
    };
    addAndMakeVisible(cancelButton_.get());
}

MelissaStemSeparationSelectComponent::~MelissaStemSeparationSelectComponent()
{
    for (int optionIndex = 0; optionIndex < kNumOptions; ++optionIndex)
    {
        optionButtons_[optionIndex]->setLookAndFeel(nullptr);
    }
}

void MelissaStemSeparationSelectComponent::resized()
{
    const int margin = 10;
    const int explanationLabelHeight = 40;
    const int buttonWidth = getWidth() - margin * 2;
    const int buttonHeight = 60;
    const int cancelButtonWidth = 100;
    const int cancelButtonHeight = 30;

    explanationLabel_.setBounds(margin, margin, getWidth() - margin * 2, explanationLabelHeight);
    
    for (int optionIndex = 0; optionIndex < kNumOptions; ++optionIndex)
    {
        optionButtons_[optionIndex]->setBounds((getWidth() - buttonWidth) / 2, explanationLabelHeight + margin + optionIndex * (buttonHeight + margin), buttonWidth, buttonHeight);
    }
    
    cancelButton_->setBounds(getWidth() - cancelButtonWidth - margin, 
                            getHeight() - cancelButtonHeight - margin,
                            cancelButtonWidth, cancelButtonHeight);
}
