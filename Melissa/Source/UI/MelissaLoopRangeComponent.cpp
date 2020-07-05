//
//  MelissaLoopRangeComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaLoopRangeComponent.h"

namespace
{
constexpr int kEdgeWidth = 4;
};

MelissaLoopRangeComponent::MelissaLoopRangeComponent() :
aRatio_(0.f), bRatio_(1.f),
mouseOnLoopStartEdge_(false), mouseOnLoopEndEdge_(false),
draggingLoopStart_(true),
mouseStatus_(kMouseStatus_None)
{
    model_ = MelissaModel::getInstance();
    model_->addListener(this);
}

void MelissaLoopRangeComponent::paint(Graphics& g)
{
    const auto width = getWidth();
    
    auto rect = getLocalBounds();
    rect.setX(aRatio_ * width);
    rect.setWidth((bRatio_ - aRatio_) * width);
    
    g.setColour(Colours::white.withAlpha(0.2f));
    g.fillRect(rect);
    
    const auto loopStartRect = getLoopStartEdgeRect();
    g.setColour(Colour(MelissaUISettings::getAccentColour()).withAlpha(1.f));
    g.fillRect(loopStartRect);
    if (mouseOnLoopStartEdge_)
    {
        g.setColour(Colours::black.withAlpha(0.3f));
        g.fillRect(loopStartRect);
    }
    
    const auto loopEndRect = getLoopEndEdgeRect();
    g.setColour(Colour(MelissaUISettings::getAccentColour()).withAlpha(1.f));
    g.fillRect(loopEndRect);
    if (mouseOnLoopEndEdge_)
    {
        g.setColour(Colours::black.withAlpha(0.3f));
        g.fillRect(loopEndRect);
    }
}

void MelissaLoopRangeComponent::mouseDown(const MouseEvent& event)
{
    const auto mousePoint = juce::Point<float>(event.x, event.y);
    mouseOnLoopStartEdge_ = getLoopStartEdgeRect().contains(mousePoint);
    mouseOnLoopEndEdge_   = getLoopEndEdgeRect().contains(mousePoint);
    
    if (mouseOnLoopStartEdge_)
    {
        mouseStatus_ = kMouseStatus_DraggingStart;
    }
    else if (mouseOnLoopEndEdge_)
    {
        mouseStatus_ = kMouseStatus_DraggingEnd;
    }
}

void MelissaLoopRangeComponent::mouseUp(const MouseEvent& event)
{
    if (mouseStatus_ == kMouseStatus_None)
    {
        // clicked
        model_->setPlayingPosRatio(event.x / static_cast<float>(getWidth()));
    }
    else
    {
        model_->setLoopPosRatio(aRatio_, bRatio_);
    }
    mouseStatus_ = kMouseStatus_None;
}

void MelissaLoopRangeComponent::mouseMove(const MouseEvent& event)
{
    const auto mousePoint = juce::Point<float>(event.x, event.y);
    mouseOnLoopStartEdge_ = getLoopStartEdgeRect().contains(mousePoint);
    mouseOnLoopEndEdge_   = getLoopEndEdgeRect().contains(mousePoint);
    
    repaint();
}

void MelissaLoopRangeComponent::mouseDrag(const MouseEvent& event)
{
    const auto width = getWidth();
    const auto mousePosRatio = std::clamp(event.x / static_cast<float>(width), 0.f, 1.f);
    
    if (mouseStatus_ == kMouseStatus_Range)
    {
        if (draggingLoopStart_)
        {
            if (aRatio_ < mousePosRatio)
            {
                bRatio_ = mousePosRatio;
            }
            else
            {
                // flipped
                draggingLoopStart_ = false;
            }
        }
        else
        {
            if (mousePosRatio < bRatio_)
            {
                aRatio_ = mousePosRatio;
            }
            else
            {
                // flipped
                draggingLoopStart_ = true;
            }
        }
    }
    else if (mouseStatus_ == kMouseStatus_DraggingStart)
    {
        if (mousePosRatio < bRatio_)
        {
            aRatio_ = mousePosRatio;
        }
        else
        {
            // flipped
            mouseStatus_ = kMouseStatus_DraggingEnd;
        }
    }
    else if (mouseStatus_ == kMouseStatus_DraggingEnd)
    {
        if (aRatio_ < mousePosRatio)
        {
            bRatio_ = mousePosRatio;
        }
        else
        {
            // flipped
            mouseStatus_ = kMouseStatus_DraggingStart;
        }
    }
    else if (abs(event.getMouseDownX() - event.x) > 4)
    {
        mouseStatus_ = kMouseStatus_Range;
        aRatio_ = event.getMouseDownX() / static_cast<float>(getWidth());
        draggingLoopStart_ = true;
    }
    
    repaint();
}

void MelissaLoopRangeComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    aRatio_ = aRatio;
    bRatio_ = bRatio;
    repaint();
}

Rectangle<float> MelissaLoopRangeComponent::getLoopStartEdgeRect() const
{
    const auto width = getWidth();
    auto rect = getLocalBounds();
    auto leftEdge = rect.withWidth(kEdgeWidth);
    leftEdge.setX(aRatio_ * width);
    return leftEdge.toFloat();
}

Rectangle<float> MelissaLoopRangeComponent::getLoopEndEdgeRect() const
{
    const auto width = getWidth();
    auto rect = getLocalBounds();
    auto rightEdge = rect.withWidth(kEdgeWidth);
    rightEdge.setX(bRatio_ * width - kEdgeWidth);
    return rightEdge.toFloat();
}
