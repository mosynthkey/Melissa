//
//  MelissaBottomControlComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaBottomControlComponent.h"
#include "MelissaModalDialog.h"
#include "MelissaOptionDialog.h"
#include "MelissaUpdateChecker.h"

MelissaBottomControlComponent::MelissaBottomControlComponent()
{
    updateButton_ = std::make_unique<TextButton>();
    updateButton_->setButtonText(TRANS("update_available"));
    updateButton_->setLookAndFeel(&laf_);
    updateButton_->onClick = []()
    {
        MelissaUpdateChecker::showUpdateDialog();
    };
    addChildComponent(updateButton_.get());
    
    startTimer(0);
}

void MelissaBottomControlComponent::paint(Graphics& g)
{
    g.setColour(MelissaUISettings::getSubColour());
    g.fillAll();
}

void MelissaBottomControlComponent::resized()
{
    const int margin = 10;
    const int buttonWidth = 180;
    updateButton_->setBounds(getWidth() - buttonWidth - margin, 0, buttonWidth, getHeight());
}

void MelissaBottomControlComponent::timerCallback()
{
    stopTimer();
    const auto status = MelissaUpdateChecker::getUpdateStatus();
    updateButton_->setVisible(status == MelissaUpdateChecker::kUpdateStatus_UpdateExists);
}
