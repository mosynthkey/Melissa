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
    const auto colour = Colour(MelissaUISettings::getMainColour());
    
    // title
    g.setColour(colour.withAlpha(0.12f));
    MelissaUtility::fillRoundRectangle(g, 0, 0, getWidth(), 30, 8, 8, 0, 0);
    g.setColour(Colours::white);
    g.setFont(MelissaUISettings::getFontSizeMain());
    g.drawText(title_, 0, 0, getWidth(), 30, Justification::centred);
    
    // body
    g.setColour(colour.withAlpha(0.06f));
    MelissaUtility::fillRoundRectangle(g, 0, 30, getWidth(), getHeight() - 30, 0, 0, 8, 8);
}
