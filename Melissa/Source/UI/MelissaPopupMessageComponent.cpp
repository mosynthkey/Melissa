//
//  MelissaPopupMessageComponent.cpp
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#include "MelissaCommand.h"
#include "MelissaDataSource.h"
#include "MelissaPopupMessageComponent.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

MelissaPopupMessageComponent::MelissaPopupMessageComponent()
{
}

MelissaPopupMessageComponent::~MelissaPopupMessageComponent()
{
}

void MelissaPopupMessageComponent::show(const String& text)
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

void MelissaPopupMessageComponent::paint(Graphics& g)
{
    g.fillAll(Colours::transparentWhite);
    
    const int textWidth = MelissaUtility::getStringSize(Font(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub)), text_).first;
    const int x = (getWidth() - textWidth) / 2;
    
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(x, 0, textWidth, 30, 6);
    
    g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    g.setColour(MelissaUISettings::getTextColour());
    g.drawText(text_, x, 0, textWidth, 30, 6, Justification::centred);
}

void MelissaPopupMessageComponent::timerCallback()
{
    animator_.cancelAnimation(this, false);
    animator_.fadeOut(this, 500);
}
