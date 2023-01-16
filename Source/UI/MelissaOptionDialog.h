//
//  MelissaOptionDialog.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaModalDialog.h"
#include "MelissaUtility.h"

class MelissaOptionDialog : public Component
{
public:
    MelissaOptionDialog(const String& labelString, const std::vector<String>& options, std::function<void(size_t index)> onClick) : onClick_(onClick)
    {
        constexpr int buttonWidth = 120;
        constexpr int controlHeight = 30;
        constexpr int margin = 10;
        const size_t numOfControls = options.size();
        auto labelSize = MelissaUtility::getStringSize(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)), labelString);
        const int minimumWidth = static_cast<int>(buttonWidth * numOfControls + margin * (numOfControls - 1));
        if (labelSize.first < minimumWidth) labelSize.first = minimumWidth;
        const int width = labelSize.first + margin * 2;
        const int height = margin * 3 + controlHeight + labelSize.second;
        
        setSize(width, height);
        
        label_ = std::make_unique<Label>();
        label_->setFont(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main)));
        label_->setText(labelString, dontSendNotification);
        label_->setBounds(margin, margin, labelSize.first, labelSize.second);
        addAndMakeVisible(label_.get());
        
        int xPos = width - (margin + buttonWidth) * static_cast<int>(numOfControls);
        for (size_t button_i = 0; button_i < numOfControls; ++button_i)
        {
            auto button = std::make_unique<TextButton>();
            button->setBounds(xPos, margin * 2 + labelSize.second, buttonWidth, controlHeight);
            button->setButtonText(options[button_i]);
            button->onClick = [&, button_i]() {
                if (onClick_ == nullptr) return;
                onClick_(button_i);
                MelissaModalDialog::close();
            };
            addAndMakeVisible(button.get());
            optionButtons_.emplace_back(std::move(button));
            
            xPos += (margin + buttonWidth);
        }
    }
    
private:
    std::unique_ptr<Label> label_;
    std::vector<std::unique_ptr<TextButton>> optionButtons_;
    std::function<void(size_t)> onClick_;
};
