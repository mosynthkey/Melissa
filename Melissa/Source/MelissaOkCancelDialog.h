#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUtility.h"

class MelissaOkCancelDialog : public Component
{
public:
    MelissaOkCancelDialog(MelissaHost* host, const String& labelString, std::function<void(void)> onClick) :
    host_(host), onClick_(onClick)
    {
        constexpr int buttonWidth = 100;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        auto labelSize = MelissaUtility::getStringSize(Font(22), labelString);
        const int minimumWidth = buttonWidth * 2 + margin;
        if (labelSize.first < minimumWidth)
        {
            labelSize.first = minimumWidth;
        }
        const int width = labelSize.first + margin * 2;
        const int height = margin * 3 + controlHeight + labelSize.second;
        
        setSize(width, height);
        
        label_ = std::make_unique<Label>();
        label_->setFont(Font(22));
        label_->setText(labelString, dontSendNotification);
        label_->setBounds(margin, margin, labelSize.first, labelSize.second);
        addAndMakeVisible(label_.get());
        
        okButton_ = std::make_unique<TextButton>();
        okButton_->setBounds(width - (margin + buttonWidth) * 2, margin * 2 + labelSize.second, buttonWidth, controlHeight);
        okButton_->setButtonText("OK");
        okButton_->onClick = [&]() {
            onClick_();
            host_->closeModalDialog();
        };
        addAndMakeVisible(okButton_.get());
        
        cancelButton_ = std::make_unique<TextButton>();
        cancelButton_->setBounds(width - (margin + buttonWidth), margin * 2 + labelSize.second, buttonWidth, controlHeight);
        cancelButton_->setButtonText(TRANS("cancel"));
        cancelButton_->onClick = [&]() { host_->closeModalDialog(); };
        addAndMakeVisible(cancelButton_.get());
    }
    
private:
    MelissaHost* host_;
    std::unique_ptr<Label> label_;
    std::unique_ptr<TextButton> okButton_;
    std::unique_ptr<TextButton> cancelButton_;
    std::function<void(void)> onClick_;
};
