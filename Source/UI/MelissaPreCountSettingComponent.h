//
//  MelissaPreCountSettingComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDefinitions.h"
#include "MelissaLookAndFeel.h"
#include "MelissaModelListener.h"
#include "MelissaUtility.h"

class MelissaPreCountSettingComponent : public juce::Component, public MelissaModelListener
{
public:
    MelissaPreCountSettingComponent()
    {
        using namespace juce;
        
        model_ = MelissaModel::getInstance();
        model_->addListener(this);
        
        constexpr int textEditorWidth = 100;
        constexpr int labelWidth = 50;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        constexpr int y0 = margin + (controlHeight + margin) * 0;
        constexpr int y1 = margin + (controlHeight + margin) * 1 + margin;
        constexpr int y2 = margin + (controlHeight + margin) * 2 + margin;
        
        circleToggleLaf_.setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
        
        setSize(380, margin + (controlHeight + margin) * 3 + margin);
        
        auto createAndAddCheckBox = [&](const String& buttonText)
        {
            constexpr int kRadioGroup_PreCountSettings = 20000;
            auto c = std::make_unique<ToggleButton>();
            c->setButtonText(buttonText);
            c->setLookAndFeel(&circleToggleLaf_);
            c->setRadioGroupId(kRadioGroup_PreCountSettings);
            addAndMakeVisible(c.get());
            return c;
        };
        
        const int checkBoxWidth = getWidth() - margin * 4 - textEditorWidth - labelWidth;
        secCheckBox_ = createAndAddCheckBox(TRANS("precount_specify_sec"));
        secCheckBox_->setToggleState(true, dontSendNotification);
        secCheckBox_->setBounds(margin, y0, checkBoxWidth, controlHeight);
        
        countCheckBox_ = createAndAddCheckBox(TRANS("precount_specify_bpm"));
        countCheckBox_->setBounds(margin, y1, checkBoxWidth, controlHeight);
        
        auto createAndAddTextEditor = [&]()
        {
            auto e = std::make_unique<TextEditor>();
            e = std::make_unique<TextEditor>();
            e->applyColourToAllText(MelissaUISettings::getTextColour());
            e->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
            e->setJustification(Justification::centred);
            e->setInputRestrictions(3, "0123456789");
            e->onReturnKey = [&]()
            {
            };
            addAndMakeVisible(e.get());
            return e;
        };
        
        secEditor_ = createAndAddTextEditor();
        secEditor_->setBounds(secCheckBox_->getRight() + margin, y0, textEditorWidth, controlHeight);
        
        bpmEditor_ = createAndAddTextEditor();
        bpmEditor_->setBounds(countCheckBox_->getRight() + margin, y1, textEditorWidth, controlHeight);
        
        countEditor_ = createAndAddTextEditor();
        countEditor_->setBounds(countCheckBox_->getRight() + margin, y2, textEditorWidth, controlHeight);
        
        auto createAndAddLabel = [&](const String& text)
        {
            auto l = std::make_unique<Label>();
            l->setText(text, dontSendNotification);
            l->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
            addAndMakeVisible(l.get());
            return l;
        };
        
        secLabel_ = createAndAddLabel(TRANS("precount_sec"));
        secLabel_->setBounds(secEditor_->getRight() + margin, y0, labelWidth, controlHeight);
        
        bpmLabel_ = createAndAddLabel(TRANS("precount_bpm"));
        bpmLabel_->setBounds(countEditor_->getRight() + margin, y1, labelWidth, controlHeight);
        
        countLabel_ = createAndAddLabel(TRANS("precount_count"));
        countLabel_->setBounds(countEditor_->getRight() + margin, y2, labelWidth, controlHeight);
    }
    
    ~MelissaPreCountSettingComponent()
    {
        secCheckBox_->setLookAndFeel(nullptr);
        countCheckBox_->setLookAndFeel(nullptr);
        MelissaModel::getInstance()->removeListener(this);
    }
    
private:
    MelissaModel* model_;
    
    std::unique_ptr<juce::ToggleButton> secCheckBox_;
    std::unique_ptr<juce::ToggleButton> countCheckBox_;
    
    std::unique_ptr<juce::Label> secLabel_;
    std::unique_ptr<juce::Label> bpmLabel_;
    std::unique_ptr<juce::Label> countLabel_;
    
    std::unique_ptr<juce::TextEditor> secEditor_;
    std::unique_ptr<juce::TextEditor> bpmEditor_;
    std::unique_ptr<juce::TextEditor> countEditor_;
    
    MelissaLookAndFeel_CircleToggleButton circleToggleLaf_;
};
