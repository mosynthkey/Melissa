#include "MelissaBottomControlComponent.h"

enum
{
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
    g.setColour(Colour(MelissaColourScheme::MainColour()).withAlpha(0.06f));
    g.fillAll();
}

void MelissaBottomControlComponent::resized()
{
    tooltipLabel_->setBounds(kXMargin, 0, getWidth() - kXMargin * 2, getHeight());
    settingsButton_->setBounds(getWidth() - kSettingsButtonWidth - kXMargin, 0, kSettingsButtonWidth, getHeight());
}
