//
//  MelissaStemDetailComponent.cpp
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaDataSource.h"
#include "MelissaStemDetailComponent.h"
#include "MelissaUISettings.h"


namespace
{
const char* partNames[MelissaStemDetailComponent::kNumParts] = { "Vocal", "Piano", "Bass", "Drums", "Others" };
constexpr int titleWidth = 52;
constexpr int kXMargin = 5;
}

MelissaStemDetailComponent::MelissaStemDetailComponent()
{
    resetButton_ = std::make_unique<MelissaRoundButton>("Reset");
    resetButton_->setTooltip(TRANS("reset_part_detail"));
    resetButton_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    resetButton_->onClick =  [this]()
    {
        auto model = MelissaModel::getInstance();
        for (int part = kCustomPartVolume_Vocal; part <= kCustomPartVolume_Others; ++part)
        {
            model->setCustomPartVolume(static_cast<CustomPartVolume>(part), 0.f);
        }
    };
    addAndMakeVisible(resetButton_.get());
    
    for (int partIndex = 0; partIndex < kNumParts; ++partIndex)
    {
        auto k = std::make_unique<Slider>(Slider::RotaryVerticalDrag, Slider::NoTextBox);
        k->setTooltip(TRANS(String::fromUTF8(partNames[partIndex]).toLowerCase() + "_custom_volume"));
        k->setRange(-1.f, 1.0f);
        k->setDoubleClickReturnValue(true, 0.f);
        k->setValue(0.f);
        k->onValueChange = [&, this, partIndex]()
        {
            const auto value = partKnobs_[partIndex]->getValue();
            MelissaModel::getInstance()->setCustomPartVolume(static_cast<CustomPartVolume>(partIndex), value);
        };
        addAndMakeVisible(k.get());
        partKnobs_[partIndex] = std::move(k);
    }
    
    MelissaModel::getInstance()->addListener(this);
}

void MelissaStemDetailComponent::resized()
{
    updatePosition();
    constexpr int buttonWidth = 52;
    resetButton_->setBounds(kXMargin + (titleWidth / 2) - (buttonWidth / 2), 34, buttonWidth, 22);
    constexpr int knobSize = 36;
    for (int partIndex = 0; partIndex < kNumParts; ++partIndex)
    {
        const int cx = xCenterList_[partIndex];
        partKnobs_[partIndex]->setBounds(cx - knobSize / 2, 24, knobSize, knobSize);
    }
}

void MelissaStemDetailComponent::paint(Graphics& g)
{
    auto dataSource = MelissaDataSource::getInstance();
    
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 15);
    
    g.setColour(Colours::white);
    
    g.setFont(dataSource->getFont(MelissaDataSource::Global::kFontSize_Sub));
    g.drawText("Part", kXMargin, 5, titleWidth, 20, Justification::centred);
    
    g.setFont(dataSource->getFont(MelissaDataSource::Global::kFontSize_Small));
    for (int partIndex = 0; partIndex < kNumParts; ++partIndex)
    {
        const int cx = xCenterList_[partIndex];
        g.drawText(partNames[partIndex], cx - labelWidth_ / 2, 5, labelWidth_, 20, Justification::centred);
    }
}

void MelissaStemDetailComponent::updatePosition()
{
    const float totalControlWidth = getWidth() - 30 - kXMargin;
    const float partNameWidth = 50;
    const float widthUnit = (totalControlWidth - titleWidth) / (partNameWidth * kNumParts);
    
    labelWidth_ = static_cast<int>(widthUnit * partNameWidth);
    int x = kXMargin + titleWidth;
    
    for (int partIndex = 0; partIndex < kNumParts; ++partIndex)
    {
        xCenterList_[partIndex] = x + (labelWidth_ / 2);
        x += labelWidth_;
    }
}

void MelissaStemDetailComponent::customPartVolumeChanged(CustomPartVolume part, float volume)
{
    const int partIndex = static_cast<int>(part);
    partKnobs_[partIndex]->setValue(volume);
}
