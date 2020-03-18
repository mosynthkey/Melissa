#include "MelissaModalDialog.h"

enum
{
    kMargin = 4,
    kCloseButtonSize = 20,
};

MelissaModalDialog::MelissaModalDialog(MelissaHost* host, std::shared_ptr<Component>& component, const String& title, bool closeOnClickingOutside) :
host_(host),
component_(component),
closeOnClickingOutside_(closeOnClickingOutside)
{
    backgroundButton_ = std::make_unique<BackgroundButton>();
    backgroundButton_->onClick = [&]() { if (closeOnClickingOutside_) host_->closeModalDialog(); };
    addAndMakeVisible(backgroundButton_.get());
    
    titleLabel_ = std::make_unique<Label>();
    titleLabel_->setJustificationType(Justification::centred);
    titleLabel_->setFont(Font(22));
    titleLabel_->setText(title, dontSendNotification);
    addAndMakeVisible(titleLabel_.get());
    
    closeButton_ = std::make_unique<CloseButton>();
    closeButton_->setSize(kCloseButtonSize, kCloseButtonSize);
    closeButton_->onClick = [&]() { host_->closeModalDialog(); };
    addAndMakeVisible(closeButton_.get());
    
    addAndMakeVisible(component.get());
}

void MelissaModalDialog::paint(Graphics& g)
{
    g.fillAll(Colours::black.withAlpha(0.8f));
    
    const int dialogWidth  = kMargin + component_->getWidth()  + kMargin;
    const int dialogHeight = kMargin + kCloseButtonSize + kMargin + component_->getHeight() + kMargin;
    
    g.setColour(Colour(MelissaColourScheme::DialogBackgoundColour()));
    g.fillRoundedRectangle((getWidth() - dialogWidth) / 2, (getHeight() - dialogHeight) / 2, dialogWidth, dialogHeight, 4);
}

void MelissaModalDialog::resized()
{
    const int dialogWidth  = kMargin + component_->getWidth()  + kMargin;
    const int dialogHeight = kMargin + kCloseButtonSize + kMargin + component_->getHeight() + kMargin;
    
    const int x = (getWidth() - dialogWidth) / 2;
    const int y = (getHeight() - dialogHeight) / 2;
    
    backgroundButton_->setBounds(0, 0, getWidth(), getHeight());
    closeButton_->setBounds(x + kMargin, y + kMargin, kCloseButtonSize, kCloseButtonSize);
    titleLabel_->setBounds(x + kMargin, y + kMargin, dialogWidth - kMargin * 2, kCloseButtonSize);
    component_->setTopLeftPosition(closeButton_->getX(), closeButton_->getBottom() + kMargin);
}
