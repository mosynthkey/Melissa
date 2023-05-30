//
//  MelissaMobileMenuComponent.cpp
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaMobileMenuComponent.h"
#include "MelissaUISettings.h"

MelissaMobileMenuComponent::MelissaMobileMenuComponent()
{
    
}

MelissaMobileMenuComponent::~MelissaMobileMenuComponent()
{
    
}

void MelissaMobileMenuComponent::resized()
{
    
}

void MelissaMobileMenuComponent::paint(juce::Graphics& g)
{
    g.fillAll(MelissaUISettings::getMainColour());
}
