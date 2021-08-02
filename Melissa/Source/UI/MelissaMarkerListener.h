//
//  MelissaMarkerListener.h
//  Melissa
//
//  Copyright(c) 2021 Masaki Ono
//

#pragma once

class MelissaMarkerListener
{
public:
    virtual ~MelissaMarkerListener() {};
    virtual void markerClicked(size_t markerIndex, bool isShiftKeyDown) {};
};
