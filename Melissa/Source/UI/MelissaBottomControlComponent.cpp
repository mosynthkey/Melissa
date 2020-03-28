#include "MelissaBottomControlComponent.h"
#include "MelissaUpdateChecker.h"

MelissaBottomControlComponent::MelissaBottomControlComponent()
{
    updateButton_ = std::make_unique<TextButton>();
    updateButton_->setButtonText(TRANS("update_available"));
    updateButton_->setLookAndFeel(&lookAndFeel_);
    updateButton_->onClick = []()
    {
        if (NativeMessageBox::showYesNoBox(AlertWindow::NoIcon, TRANS("update"), TRANS("there_is_update")) == 1)
        {
            URL("https://github.com/mosynthkey/Melissa/releases").launchInDefaultBrowser();
        }
    };
    addChildComponent(updateButton_.get());
    
    startTimer(0);
}

void MelissaBottomControlComponent::paint(Graphics& g)
{
    g.setColour(Colour(MelissaUISettings::mainColour()).withAlpha(0.06f));
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
