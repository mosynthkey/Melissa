//
//  MelissaSectionComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaSectionComponent.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

MelissaSectionComponent::MelissaSectionComponent(const String& title) :
title_(title)
{
}

void MelissaSectionComponent::paint(Graphics& g)
{
    constexpr int kRound = 6;
    const auto colour = MelissaUISettings::getMainColour();
    
    // body
    g.setColour(colour);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), kRound);
    
    // title
    g.setColour(Colours::white);
    g.setFont(MelissaUISettings::getFontSizeMain());
    g.drawText(title_, 0, 0, getWidth(), 30, Justification::centred);
    
    // line
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRect(10, 30, getWidth() - 20, 1);
}
