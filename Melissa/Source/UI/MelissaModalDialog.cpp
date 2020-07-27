//
//  MelissaModalDialog.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaModalDialog.h"

enum
{
    kMargin = 4,
    kCloseButtonSize = 20,
};

MelissaDialog::MelissaDialog(std::shared_ptr<Component> contentComponent, const String& title, bool closeOnClickingOutside) :
contentComponent_(contentComponent),
closeOnClickingOutside_(closeOnClickingOutside)
{
    backgroundButton_ = std::make_unique<BackgroundButton>();
    backgroundButton_->onClick = [&]() { if (closeOnClickingOutside_) MelissaModalDialog::close(); };
    addAndMakeVisible(backgroundButton_.get());
    
    titleLabel_ = std::make_unique<Label>();
    titleLabel_->setJustificationType(Justification::centred);
    titleLabel_->setText(title, dontSendNotification);
    titleLabel_->setFont(Font(MelissaUISettings::getFontSizeMain()));
    addAndMakeVisible(titleLabel_.get());
    
    closeButton_ = std::make_unique<CloseButton>();
    closeButton_->setSize(kCloseButtonSize, kCloseButtonSize);
    closeButton_->onClick = [&]() { MelissaModalDialog::close(); };
    addAndMakeVisible(closeButton_.get());
    
    addAndMakeVisible(contentComponent.get());
}

void MelissaDialog::paint(Graphics& g)
{
    g.fillAll(Colours::black.withAlpha(0.8f));
    
    const int dialogWidth  = kMargin + contentComponent_->getWidth()  + kMargin;
    const int dialogHeight = kMargin + kCloseButtonSize + kMargin + contentComponent_->getHeight() + kMargin;
    
    g.setColour(Colour(MelissaUISettings::getDialogBackgoundColour()));
    g.fillRoundedRectangle((getWidth() - dialogWidth) / 2, (getHeight() - dialogHeight) / 2, dialogWidth, dialogHeight, 4);
}

void MelissaDialog::resized()
{
    const int dialogWidth  = kMargin + contentComponent_->getWidth()  + kMargin;
    const int dialogHeight = kMargin + kCloseButtonSize + kMargin + contentComponent_->getHeight() + kMargin;
    
    const int x = (getWidth() - dialogWidth) / 2;
    const int y = (getHeight() - dialogHeight) / 2;
    
    backgroundButton_->setBounds(0, 0, getWidth(), getHeight());
    closeButton_->setBounds(x + kMargin, y + kMargin, kCloseButtonSize, kCloseButtonSize);
    titleLabel_->setBounds(x + kMargin, y + kMargin, dialogWidth - kMargin * 2, kCloseButtonSize);
    contentComponent_->setTopLeftPosition(closeButton_->getX(), closeButton_->getBottom() + kMargin);
}

Component* MelissaModalDialog::parentComponent_ = nullptr;
std::unique_ptr<MelissaDialog> MelissaModalDialog::dialog_;

void MelissaModalDialog::show(std::shared_ptr<Component> component, const String& title, bool closeOnClickingOutside)
{
    dialog_ = std::make_unique<MelissaDialog>(component, title, closeOnClickingOutside);
    dialog_->setBounds(parentComponent_->getBounds());
    parentComponent_->addAndMakeVisible(dialog_.get());
}

void MelissaModalDialog::close()
{
    dialog_.reset(nullptr);
}

void MelissaModalDialog::resize()
{
    if (dialog_ != nullptr) dialog_->setSize(parentComponent_->getWidth(), parentComponent_->getHeight());
}
