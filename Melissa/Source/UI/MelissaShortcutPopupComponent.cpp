//
//  MelissaShortcutPopupComponent.cpp
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#include "MelissaCommand.h"
#include "MelissaDataSource.h"
#include "MelissaShortcutPopupComponent.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

MelissaShortcutPopupComponent::MelissaShortcutPopupComponent()
{
    MelissaShortcutManager::getInstance()->addListener(this);
}

MelissaShortcutPopupComponent::~MelissaShortcutPopupComponent()
{
    MelissaShortcutManager::getInstance()->removeListener(this);
}

void MelissaShortcutPopupComponent::show(const String& text)
{
    const auto assignedShortcut = MelissaDataSource::getInstance()->getAssignedShortcut(text);
    if (assignedShortcut.isEmpty()) return;
    
    text_ = text + String(" : ") + MelissaCommand::getInstance()->getCommandDescription(assignedShortcut);
    
    repaint();
    
    animator_.cancelAnimation(this, false);
    animator_.fadeIn(this, 0);
    setVisible(true);
    stopTimer();
    startTimer(2000);
}

void MelissaShortcutPopupComponent::paint(Graphics& g)
{
    g.fillAll(Colours::transparentWhite);
    
    const int textWidth = MelissaUtility::getStringSize(Font(MelissaUISettings::getFontSizeSub()), text_).first;
    const int x = (getWidth() - textWidth) / 2;
    
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(x, 0, textWidth, 30, 6);
    
    g.setFont(MelissaUISettings::getFontSizeSub());
    g.setColour(MelissaUISettings::getTextColour());
    g.drawText(text_, x, 0, textWidth, 30, 6, Justification::centred);
}

void MelissaShortcutPopupComponent::timerCallback()
{
    animator_.cancelAnimation(this, false);
    animator_.fadeOut(this, 500);
}

void MelissaShortcutPopupComponent::controlMessageReceived(const String& controlMessage)
{
    show(controlMessage);
}
