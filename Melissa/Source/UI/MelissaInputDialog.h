//
//  MelissaInputDialog.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"
#include "MelissaHost.h"
#include "MelissaModalDialog.h"

class MelissaInputDialog : public Component, public TextEditor::Listener
{
public:
    MelissaInputDialog(const String& labelString,  const String& defaultTextEditorString, std::function<void(const String& string)> onClick) : onClick_(onClick)
    {
        constexpr int textEditorWidth = 440;
        constexpr int buttonWidth = 120;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        
        constexpr int width = textEditorWidth + margin * 2;
        constexpr int height = margin * 4 + controlHeight * 3;
        
        setSize(width, height);
        
        label_ = std::make_unique<Label>();
        label_->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
        label_->setText(labelString, dontSendNotification);
        label_->setBounds(margin, margin, textEditorWidth, controlHeight);
        addAndMakeVisible(label_.get());
        
        textEditor_ = std::make_unique<TextEditor>();
        textEditor_->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
        textEditor_->setBounds(margin, margin * 2 + controlHeight, textEditorWidth, controlHeight);
        textEditor_->setText(defaultTextEditorString);
        textEditor_->applyColourToAllText(MelissaUISettings::getTextColour());
        textEditor_->addListener(this);
        addAndMakeVisible(textEditor_.get());
        
        okButton_ = std::make_unique<TextButton>();
        okButton_->setBounds(width - (margin + buttonWidth) * 2, margin * 3 + controlHeight * 2, buttonWidth, controlHeight);
        okButton_->setButtonText("OK");
        okButton_->onClick = [&]() {
            const String text = textEditor_->getText();
            onClick_(text);
            MelissaModalDialog::close();
        };
        addAndMakeVisible(okButton_.get());
        
        cancelButton_ = std::make_unique<TextButton>();
        cancelButton_->setBounds(width - (margin + buttonWidth), margin * 3 + controlHeight * 2, buttonWidth, controlHeight);
        cancelButton_->setButtonText(TRANS("cancel"));
        cancelButton_->onClick = [&]() { MelissaModalDialog::close(); };
        addAndMakeVisible(cancelButton_.get());
        
        setLookAndFeel(&laf_);
    }
    
    MelissaInputDialog(const String& labelString,  const String& defaultTextEditorString, std::function<void(const String& string)> onClick, const std::vector<String>& inputList) : onClick_(onClick)
    {
        const int numControlsHorizontal = 8;
        const int numControlsVertical = 3 + (static_cast<int>(inputList.size()) + numControlsHorizontal - 1) / numControlsHorizontal;
        
        constexpr int buttonWidth = 120;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        constexpr int inputButtonWidth = 120;
        
        constexpr int width = numControlsHorizontal * (inputButtonWidth + margin) + margin;
        const int textEditorWidth = width - margin * 2;
        
        const int height = margin * (numControlsVertical + 1) + controlHeight * numControlsVertical;
        
        setSize(width, height);
        
        label_ = std::make_unique<Label>();
        label_->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
        label_->setText(labelString, dontSendNotification);
        label_->setBounds(margin, margin, textEditorWidth, controlHeight);
        addAndMakeVisible(label_.get());
        
        textEditor_ = std::make_unique<TextEditor>();
        textEditor_->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
        textEditor_->setBounds(margin, margin * 2 + controlHeight, textEditorWidth, controlHeight);
        textEditor_->setText(defaultTextEditorString);
        textEditor_->applyColourToAllText(MelissaUISettings::getTextColour());
        textEditor_->addListener(this);
        addAndMakeVisible(textEditor_.get());
        
        int x = margin;
        int y = textEditor_->getBottom() + margin;
        
        
        for (int inputIndex = 0; inputIndex < inputList.size(); ++inputIndex)
        {
            const String buttonText = TRANS(inputList[inputIndex]);
            x = margin + (inputIndex % numControlsHorizontal) * (inputButtonWidth + margin);
            y = textEditor_->getBottom() + margin + (inputIndex / numControlsHorizontal) * (controlHeight + margin);
            
            auto b = std::make_unique<TextButton>();
            b->setBounds(x, y, inputButtonWidth, controlHeight);
            b->setButtonText(buttonText);
            b->onClick = [&, buttonText]() {
                const String text = textEditor_->getText().trim();
                textEditor_->setText(text + " " + buttonText);
            };
            addAndMakeVisible(b.get());
            inputButtons_.emplace_back(std::move(b));
        }
        
        y += (controlHeight + margin);
        
        okButton_ = std::make_unique<TextButton>();
        okButton_->setBounds(width - (margin + buttonWidth) * 2, y, buttonWidth, controlHeight);
        okButton_->setButtonText("OK");
        okButton_->onClick = [&]() {
            const String text = textEditor_->getText();
            onClick_(text);
            MelissaModalDialog::close();
        };
        addAndMakeVisible(okButton_.get());
        
        cancelButton_ = std::make_unique<TextButton>();
        cancelButton_->setBounds(width - (margin + buttonWidth), y, buttonWidth, controlHeight);
        cancelButton_->setButtonText(TRANS("cancel"));
        cancelButton_->onClick = [&]() { MelissaModalDialog::close(); };
        addAndMakeVisible(cancelButton_.get());
        
        setLookAndFeel(&laf_);
    }
    
    ~MelissaInputDialog()
    {
        setLookAndFeel(nullptr);
    }
    
    void textEditorReturnKeyPressed(TextEditor& editor) override
    {
        const String text = textEditor_->getText();
        onClick_(text);
        MelissaModalDialog::close();
    }
    
private:
    MelissaHost* host_;
    MelissaLookAndFeel laf_;
    std::unique_ptr<Label> label_;
    std::unique_ptr<TextEditor> textEditor_;
    std::unique_ptr<TextButton> okButton_;
    std::unique_ptr<TextButton> cancelButton_;
    std::vector<std::unique_ptr<TextButton>> inputButtons_;
    std::function<void(const String& string)> onClick_;
};
