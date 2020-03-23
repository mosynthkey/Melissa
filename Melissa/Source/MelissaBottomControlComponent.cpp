#include "MelissaBottomControlComponent.h"

MelissaBottomControlComponent::MelissaBottomControlComponent()
{
}

void MelissaBottomControlComponent::paint(Graphics& g)
{
    g.setColour(Colour(MelissaUISettings::MainColour()).withAlpha(0.06f));
    g.fillAll();
}

