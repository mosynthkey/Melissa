//
//  MelissaSoundEngineDialog.h
//  Melissa
//
//  Copyright(c) 2026 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDefinitions.h"
#include "MelissaLookAndFeel.h"
#include "MelissaModelListener.h"
#include "MelissaUtility.h"

class MelissaSoundEngineDialog : public juce::Component, public MelissaModelListener
{
public:
    MelissaSoundEngineDialog()
    {
        using namespace juce;

        model_ = MelissaModel::getInstance();
        model_->addListener(this);

        circleToggleLaf_.setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));

        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        constexpr int width = 300;

        setSize(width, margin + (controlHeight + margin) * 2);

        bungeeButton_ = std::make_unique<ToggleButton>(TRANS("sound_engine_bungee"));
        bungeeButton_->setLookAndFeel(&circleToggleLaf_);
        bungeeButton_->setRadioGroupId(kRadioGroup_SoundEngine);
        bungeeButton_->setBounds(margin, margin, width - margin * 2, controlHeight);
        bungeeButton_->onClick = [this]()
        {
            if (bungeeButton_->getToggleState())
                model_->setStretcherType(kStretcher_Bungee);
        };
        addAndMakeVisible(bungeeButton_.get());

        soundTouchButton_ = std::make_unique<ToggleButton>(TRANS("sound_engine_soundtouch"));
        soundTouchButton_->setLookAndFeel(&circleToggleLaf_);
        soundTouchButton_->setRadioGroupId(kRadioGroup_SoundEngine);
        soundTouchButton_->setBounds(margin, margin + controlHeight + margin, width - margin * 2, controlHeight);
        soundTouchButton_->onClick = [this]()
        {
            if (soundTouchButton_->getToggleState())
                model_->setStretcherType(kStretcher_SoundTouch);
        };
        addAndMakeVisible(soundTouchButton_.get());

        updateToggleState();
    }

    ~MelissaSoundEngineDialog() override
    {
        bungeeButton_->setLookAndFeel(nullptr);
        soundTouchButton_->setLookAndFeel(nullptr);
        model_->removeListener(this);
    }

    void stretcherTypeChanged(StretcherType type) override
    {
        updateToggleState();
    }

private:
    static constexpr int kRadioGroup_SoundEngine = 30000;

    void updateToggleState()
    {
        const bool isBungee = (model_->getStretcherType() == kStretcher_Bungee);
        bungeeButton_->setToggleState(isBungee, juce::dontSendNotification);
        soundTouchButton_->setToggleState(!isBungee, juce::dontSendNotification);
    }

    MelissaModel* model_;
    std::unique_ptr<juce::ToggleButton> bungeeButton_;
    std::unique_ptr<juce::ToggleButton> soundTouchButton_;
    MelissaLookAndFeel_CircleToggleButton circleToggleLaf_;
};
