#include "MelissaBottomControlComponent.h"

enum
{
    kTop = 20,
    kXMargin = 20,
    kSettingsButtonWidth = 80,
};

MelissaBottomControlComponent::MelissaBottomControlComponent(MelissaHost* host) : host_(host)
{
    tooltipLabel_ = std::make_unique<Label>();
    tooltipLabel_->setText("Tooltip", dontSendNotification);
    addAndMakeVisible(tooltipLabel_.get());
    
    settingsButton_ = std::make_unique<MelissaSimpleTextButton>("Settings");
    settingsButton_->onClick = [&]()
    {
        host_->showPreferencesDialog();
    };
    addAndMakeVisible(settingsButton_.get());
}

void MelissaBottomControlComponent::setTooltipText(const String& tooltipText)
{
    tooltipLabel_->setText(tooltipText, dontSendNotification);
}

void MelissaBottomControlComponent::paint(Graphics& g)
{
    const int gradMargin = kTop;
    const int w = getWidth();
    

}

void MelissaBottomControlComponent::resized()
{
    const int height = getHeight() - kTop;
    tooltipLabel_->setBounds(kXMargin, kTop, getWidth() - kXMargin * 2, height);
    settingsButton_->setBounds(getWidth() - kSettingsButtonWidth - kXMargin, kTop, kSettingsButtonWidth, height);
}
