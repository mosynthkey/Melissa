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
        MelissaStemProvider::getInstance()->requestStems(
            MelissaDataSource::getInstance()->getCurrentSongFilePath(), 
            kSeparatorType_Demucs,
            static_cast<StemOutputAudioFormat>(formatComboBox_->getSelectedItemIndex()));
        MelissaModalDialog::close();
    };
    addAndMakeVisible(optionButtons_[kOption_Demucs].get());

    optionButtons_[kOption_Spleeter] = std::make_unique<juce::TextButton>();
    optionButtons_[kOption_Spleeter]->setButtonText(TRANS("separation_spleeter_detail"));
    optionButtons_[kOption_Spleeter]->onClick = [this]() 
    {
        MelissaStemProvider::getInstance()->requestStems(
            MelissaDataSource::getInstance()->getCurrentSongFilePath(), 
            kSeparatorType_Spleeter,
            static_cast<StemOutputAudioFormat>(formatComboBox_->getSelectedItemIndex()));
        MelissaModalDialog::close();
    };
    addAndMakeVisible(optionButtons_[kOption_Spleeter].get());
    
    formatLabel_.setText(TRANS("format"), juce::dontSendNotification);
    addAndMakeVisible(formatLabel_);
    
    formatComboBox_ = std::make_unique<juce::ComboBox>();
    formatComboBox_->addItem(TRANS("ogg_format_description"), kStemOutputAudioFormat_Ogg + 1);
    formatComboBox_->addItem(TRANS("wav_format_description"), kStemOutputAudioFormat_Wav + 1);
    formatComboBox_->setSelectedItemIndex(kStemOutputAudioFormat_Ogg);
    addAndMakeVisible(formatComboBox_.get());
    
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
    formatComboBox_->setLookAndFeel(nullptr);
}

void MelissaStemSeparationSelectComponent::resized()
{
    const int margin = 10;
    const int explanationLabelHeight = 40;
    const int buttonWidth = getWidth() - margin * 2;
    const int buttonHeight = 60;
    const int cancelButtonWidth = 100;
    const int cancelButtonHeight = 30;
    const int formatLabelWidth = 120;
    const int formatComboBoxWidth = getWidth() - margin * 2 - formatLabelWidth;
    const int formatControlHeight = 30;

    explanationLabel_.setBounds(margin, margin, getWidth() - margin * 2, explanationLabelHeight);
    
    for (int optionIndex = 0; optionIndex < kNumOptions; ++optionIndex)
    {
        optionButtons_[optionIndex]->setBounds((getWidth() - buttonWidth) / 2, 
                                              explanationLabelHeight + margin + optionIndex * (buttonHeight + margin), 
                                              buttonWidth, buttonHeight);
    }
    
    const int formatYPos = explanationLabelHeight + margin + kNumOptions * (buttonHeight + margin);
    formatLabel_.setBounds(margin, formatYPos, formatLabelWidth, formatControlHeight);
    formatComboBox_->setBounds(margin + formatLabelWidth, formatYPos, formatComboBoxWidth, formatControlHeight);

    cancelButton_->setBounds(getWidth() - cancelButtonWidth - margin,
                             getHeight() - cancelButtonHeight - margin,
                             cancelButtonWidth, cancelButtonHeight);
}
