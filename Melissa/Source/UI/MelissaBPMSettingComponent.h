//
//  MelissaBPMSettingComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"
#include "MelissaModelListener.h"
#include "MelissaTapTempoButton.h"
#include "MelissaUtility.h"

class MelissaBPMSettingComponent : public Component, public MelissaModelListener
{
public:
    MelissaBPMSettingComponent() : wasMetronomeOn_(false)
    {
        model_ = MelissaModel::getInstance();
        model_->addListener(this);
        
        wasMetronomeOn_ = model_->getMetronomeSwitch();
        model_->setMetronomeSwitch(false);
        
        constexpr int textEditorWidth = 100;
        //constexpr int buttonWidth = 120;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        //constexpr int checkboxWidth = 30;
        
        circleToggleLaf_.setFont(Font(MelissaUISettings::getFontSizeMain()));
        
        setSize(380, margin + (controlHeight + margin) * 3);
        
        {
            auto l = std::make_unique<Label>();
            l->setText("BPM :", dontSendNotification);
            l->setFont(Font(MelissaUISettings::getFontSizeMain()));
            l->setBounds(margin, margin, MelissaUtility::getStringSize(l->getFont(), l->getText()).first, controlHeight);
            addAndMakeVisible(l.get());
            labels_.emplace_back(std::move(l));
        }
        
        {
            auto l = std::make_unique<Label>();
            l->setText("Tap tempo :", dontSendNotification);
            l->setFont(Font(MelissaUISettings::getFontSizeMain()));
            l->setBounds(margin, margin + (controlHeight + margin) * 1, MelissaUtility::getStringSize(l->getFont(), l->getText()).first, controlHeight);
            addAndMakeVisible(l.get());
            labels_.emplace_back(std::move(l));
        }
        
        bpmEditor_ = std::make_unique<TextEditor>();
        bpmEditor_->setFont(Font(MelissaUISettings::getFontSizeMain()));
        bpmEditor_->setJustification(Justification::centred);
        bpmEditor_->setBounds(getWidth() - margin - textEditorWidth, margin, textEditorWidth, controlHeight);
        bpmEditor_->setInputRestrictions(3, "0123456789");
        bpmEditor_->setText(String(model_->getBpm()), dontSendNotification);
        bpmEditor_->onReturnKey = [&]()
        {
            const auto bpm = bpmEditor_->getText().getIntValue();
            model_->setBpm(bpm);
        };
        
        tapTempoButton_ = std::make_unique<MelissaTapTempoButton>();
        tapTempoButton_->setFont(Font(MelissaUISettings::getFontSizeMain()));
        tapTempoButton_->setBounds(getWidth() - margin - textEditorWidth, margin * 2 + controlHeight, textEditorWidth, controlHeight);
        tapTempoButton_->correctWithPlaybackSpeed(true);
        addAndMakeVisible(tapTempoButton_.get());
        
        speedCheckBox_ = std::make_unique<ToggleButton>();
        speedCheckBox_->setButtonText(TRANS("Correct with the playback speed."));
        speedCheckBox_->setLookAndFeel(&circleToggleLaf_);
        speedCheckBox_->onClick = [&]()
        {
            tapTempoButton_->correctWithPlaybackSpeed(speedCheckBox_->getToggleState());
        };
        speedCheckBox_->setToggleState(true, dontSendNotification);
        speedCheckBox_->setBounds(margin, margin + (controlHeight + margin) * 2, getWidth() - margin * 2, controlHeight);
        addAndMakeVisible(speedCheckBox_.get());
        
        //bpmEditor_->setText();
        addAndMakeVisible(bpmEditor_.get());
        
    }
    
    ~MelissaBPMSettingComponent()
    {
        model_->setMetronomeSwitch(wasMetronomeOn_);
        speedCheckBox_->setLookAndFeel(nullptr);
        MelissaModel::getInstance()->removeListener(this);
    }
    
    void bpmChanged(float bpm) override
    {
        bpmEditor_->setText(String(bpm), dontSendNotification);
    }
    
private:
    MelissaModel* model_;
    std::vector<std::unique_ptr<Label>> labels_;
    std::unique_ptr<TextEditor> bpmEditor_;
    std::unique_ptr<ToggleButton> speedCheckBox_;
    std::unique_ptr<MelissaTapTempoButton> tapTempoButton_;
    MelissaLookAndFeel_CircleToggleButton circleToggleLaf_;
    
    bool wasMetronomeOn_;
};
