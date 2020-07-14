//
//  MelissaMarkerMemoComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <vector>
#include "MelissaMarkerMemoComponent.h"
#include "MelissaModel.h"
#include "MelissaUtility.h"

MelissaMarkerMemoComponent::MelissaMarkerMemoComponent()
{
    dataSource_ = MelissaDataSource::getInstance();
    dataSource_->addListener(this);
}

void MelissaMarkerMemoComponent::paint(Graphics& g)
{
    struct MelissaMarkerLabelInfo
    {
        int x_, width_;
        float posRatio_;
        String memo_;
        Colour colour_;
    };
    
    std::vector<MelissaDataSource::Song::Marker> markers;
    dataSource_->getMarkers(markers);
    
    std::vector<MelissaMarkerLabelInfo> markerLabelInfo;
    float totalWidth = 0;
    for (auto&& m : markers)
    {
        if (m.memo_.isEmpty()) continue;
        
        MelissaMarkerLabelInfo info;
        info.memo_ = m.memo_;
        info.colour_ = Colour::fromRGB(m.colourR_, m.colourG_, m.colourB_);
        info.width_ = MelissaUtility::getStringSize(font_, m.memo_).first;
        info.posRatio_ = m.position_;
        totalWidth += info.width_;
        markerLabelInfo.emplace_back(info);
    }
    if (markerLabelInfo.size() == 0) return;
    
    const size_t centerIndex = markerLabelInfo.size() / 2;
    
    float widthRatio = getWidth() / totalWidth;
    if (widthRatio > 1.f) widthRatio = 1.f;
    
    for (size_t leftIndex = 0; leftIndex <= centerIndex; ++leftIndex)
    {
        auto& info = markerLabelInfo[leftIndex];
        info.width_ *= widthRatio;
        info.x_ = (info.posRatio_ * getWidth()) - info.width_ / 2;
        if (info.x_ < 0) info.x_ = 0;
        if (1 <= leftIndex)
        {
            auto& prev = markerLabelInfo[leftIndex - 1];
            if (info.x_ < prev.x_ + prev.width_) info.x_ = prev.x_ + prev.width_;
        }
    }
    
    for (size_t rightIndex = markerLabelInfo.size() - 1; centerIndex < rightIndex; --rightIndex)
    {
        auto& info = markerLabelInfo[rightIndex];
        info.width_ *= widthRatio;
        info.x_ = (info.posRatio_ * getWidth()) - info.width_ / 2;
        if (getWidth() <= info.x_ + info.width_) info.x_ = getWidth() - info.width_;
        if (rightIndex < markerLabelInfo.size() - 1)
        {
            auto& next = markerLabelInfo[rightIndex + 1];
            if (next.x_ < info.x_ + info.width_) info.x_ = next.x_ - info.width_;
        }
    }
    
    /*
    for (size_t markerIndex = 0; markerIndex < markerLabelInfo.size() - 1; ++markerIndex)
    {
        const auto x0 = markerLabelInfo[markerIndex].x_ + markerLabelInfo[markerIndex].width_;
        const auto x1  = markerLabelInfo[markerIndex + 1].x_;
        if (x1 < x0)
        {
            const auto center = (x0 + x1) / 2;
            const auto lengthToShorten = x0 - center;
            markerLabelInfo[markerIndex].width_ -= lengthToShorten;
            
            markerLabelInfo[markerIndex + 1].x_ += lengthToShorten;
            markerLabelInfo[markerIndex + 1].width_ -= lengthToShorten;
        }
    }
     */

    const auto h = getHeight();
    constexpr auto lineHeight = 4;
    for (auto&& info : markerLabelInfo)
    {
        g.setFont(font_);
        g.setColour(Colours::white.withAlpha(0.8f));
        g.drawText(info.memo_, info.x_, 0, info.width_, h - lineHeight, Justification::centred);
        
        g.setColour(info.colour_);
        g.fillRoundedRectangle(info.x_, h - lineHeight, info.width_, lineHeight, lineHeight / 2);
    }
}

