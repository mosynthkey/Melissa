//
//  MelissaToolbarComponent.cpp
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#include "MelissaToolbarComponent.h"
#include "MelissaDataSource.h"
#include "MelissaUtility.h"

using namespace juce;

namespace
{
constexpr int kRound = 6;
constexpr int kTitleMargin = 10;
constexpr int kSeparatorWidth = 1;
constexpr int kSeparatorMargin = 10;
}

MelissaToolbarComponent::MelissaToolbarComponent(const String& title)
    : title_(title), titleWidth_(0)
{
    auto* dataSource = MelissaDataSource::getInstance();
    const auto font = dataSource->getFont(MelissaDataSource::Global::kFontSize_Main);
    titleWidth_ = MelissaUtility::getStringSize(font, title_).first;
}

void MelissaToolbarComponent::paint(Graphics& g)
{
    const auto colour = MelissaUISettings::getMainColour();

    // body
    g.setColour(colour);
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), kRound);

    // title
    auto* dataSource = MelissaDataSource::getInstance();
    g.setColour(MelissaUISettings::getTextColour());
    g.setFont(dataSource->getFont(MelissaDataSource::Global::kFontSize_Main));
    g.drawText(title_, kTitleMargin, 0, titleWidth_, getHeight(), Justification::centredLeft);

    // separator
    const int separatorX = kTitleMargin + titleWidth_ + kSeparatorMargin;
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRect(separatorX, 8, kSeparatorWidth, getHeight() - 16);
}

void MelissaToolbarComponent::resized()
{
    // Content layout is handled by subclasses or parent
}

Rectangle<int> MelissaToolbarComponent::getContentBounds() const
{
    const int contentX = kTitleMargin + titleWidth_ + kSeparatorMargin + kSeparatorWidth + kSeparatorMargin;
    return Rectangle<int>(contentX, 0, getWidth() - contentX - kTitleMargin, getHeight());
}
