#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaHost.h"

class MelissaInputDialog : public Component
{
public:
    MelissaInputDialog(MelissaHost* host, const String& labelString,  const String& defaultTextEditorString, std::function<void(const String& string)> onClick) :
    host_(host), onClick_(onClick)
    {
        constexpr int textEditorWidth = 440;
        constexpr int buttonWidth = 100;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        
        constexpr int width = textEditorWidth + margin * 2;
        constexpr int height = margin * 4 + controlHeight * 3;
        
        setSize(width, height);
        
        label_ = std::make_unique<Label>();
        label_->setFont(Font(22));
        label_->setText(labelString, dontSendNotification);
        label_->setBounds(margin, margin, textEditorWidth, controlHeight);
        addAndMakeVisible(label_.get());
        
        textEditor_ = std::make_unique<TextEditor>();
        textEditor_->setFont(Font(22));
        textEditor_->setBounds(margin, margin * 2 + controlHeight, textEditorWidth, controlHeight);
        textEditor_->setText(defaultTextEditorString);
        addAndMakeVisible(textEditor_.get());
        
        okButton_ = std::make_unique<TextButton>();
        okButton_->setBounds(width - (margin + buttonWidth) * 2, margin * 3 + controlHeight * 2, buttonWidth, controlHeight);
        okButton_->setButtonText("OK");
        okButton_->onClick = [&]() {
            const String text = textEditor_->getText().toStdString();
            onClick_(text);
        };
        addAndMakeVisible(okButton_.get());
        
        cancelButton_ = std::make_unique<TextButton>();
        cancelButton_->setBounds(width - (margin + buttonWidth), margin * 3 + controlHeight * 2, buttonWidth, controlHeight);
        cancelButton_->setButtonText(TRANS("cancel"));
        cancelButton_->onClick = [&]() {
            host_->closeModalDialog();
        };
        addAndMakeVisible(cancelButton_.get());
    }
    
private:
    MelissaHost* host_;
    std::unique_ptr<Label> label_;
    std::unique_ptr<TextEditor> textEditor_;
    std::unique_ptr<TextButton> okButton_;
    std::unique_ptr<TextButton> cancelButton_;
    std::function<void(const String& string)> onClick_;
};
