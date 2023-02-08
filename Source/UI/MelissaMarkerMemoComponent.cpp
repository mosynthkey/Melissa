//
//  MelissaMarkerMemoComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <vector>
#include "MelissaMarkerMemoComponent.h"
#include "MelissaModel.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

using namespace juce;

MelissaMarkerMemoComponent::MelissaMarkerMemoComponent()
{
    dataSource_ = MelissaDataSource::getInstance();
    dataSource_->addListener(this);
}

void MelissaMarkerMemoComponent::paint(Graphics& g)
{
    std::vector<MelissaDataSource::Song::Marker> markers;
    dataSource_->getMarkers(markers);
    
    markerLabelInfo_.clear();
    
    for (int markerIndex = 0; markerIndex < markers.size(); ++markerIndex)
    {
        const auto& m = markers[markerIndex];
        if (m.memo_.isEmpty()) continue;
        
        MelissaMarkerLabelInfo info;
        info.memo_ = m.memo_;
        info.colour_ = Colour::fromRGB(m.colourR_, m.colourG_, m.colourB_);
        info.width_ = MelissaUtility::getStringSize(font_, m.memo_).first;
        info.originalWidth_ = info.width_;
        info.posRatio_ = m.position_;
        info.x_ = m.position_ * getWidth() - info.width_ / 2;
        info.shorten_ = false;
        info.markerIndex_ = markerIndex;
        markerLabelInfo_.emplace_back(info);
    }
    if (markerLabelInfo_.size() == 0) return;

    const size_t numOfLabels = markerLabelInfo_.size();
    for (size_t markerIndex = 0; markerIndex < numOfLabels; ++markerIndex)
    {
        auto& markerInfo = markerLabelInfo_[markerIndex];
        const int x_min = (markerIndex == 0) ? 0 : (markerLabelInfo_[markerIndex - 1].x_ + markerLabelInfo_[markerIndex - 1].width_);
        const int x_max = ((markerIndex == numOfLabels - 1) ? 1.f : (markerInfo.posRatio_ + markerLabelInfo_[markerIndex + 1].posRatio_) / 2.f) * getWidth();
        const int w_max = x_max - x_min;
        
        if (w_max <= markerInfo.width_)
        {
            markerInfo.x_ = x_min;
            markerInfo.width_ = w_max;
            markerInfo.shorten_ = true;
        }
        else if (markerInfo.x_ < x_min)
        {
            markerInfo.x_ = x_min;
        }
        else if (x_max <= markerInfo.x_ + markerInfo.width_)
        {
            markerInfo.x_ = x_max - markerInfo.width_;
        }
        else
        {
            markerInfo.x_ = markerInfo.posRatio_ * getWidth() - markerInfo.width_ / 2;
        }
        
        if (1 <= markerIndex && markerLabelInfo_[markerIndex - 1].shorten_)
        {
            auto width = markerInfo.x_ - markerLabelInfo_[markerIndex - 1].x_;
            if (markerLabelInfo_[markerIndex - 1].originalWidth_ < width) width = markerLabelInfo_[markerIndex - 1].originalWidth_;
            markerLabelInfo_[markerIndex - 1].width_ = width;
        }
    }

    const auto h = getHeight();
    constexpr auto lineHeight = 4;
    for (auto&& info : markerLabelInfo_)
    {
        g.setFont(font_);
        g.setColour(MelissaUISettings::getTextColour());
        g.drawText(info.memo_, info.x_, 0, info.width_, h - lineHeight, Justification::centred);
        
        g.setColour(info.colour_);
        g.fillRoundedRectangle(info.x_, h - lineHeight, info.width_, lineHeight, lineHeight / 2);
    }
}

void MelissaMarkerMemoComponent::mouseDown(const MouseEvent& event)
{
    const int x = event.getMouseDownX();
    for (size_t infoIndex = 0; infoIndex < markerLabelInfo_.size(); ++infoIndex)
    {
        const auto& info = markerLabelInfo_[infoIndex];
        if (info.x_ <= x && x <= info.x_ + info.width_)
        {
            if (listener_ != nullptr) listener_->markerClicked(info.markerIndex_, event.mods.isShiftDown());
        }
    }
}
