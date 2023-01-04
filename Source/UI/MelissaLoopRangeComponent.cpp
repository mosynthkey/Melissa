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
mouseClickXRatio_(0.f),
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
    
    g.setColour(MelissaUISettings::getAccentColour(0.2f));
    g.fillRect(rect);
    
    const auto loopStartRect = getLoopStartEdgeRect();
    g.setColour(MelissaUISettings::getAccentColour(mouseOnLoopStartEdge_ ? 1.f : 0.6f));
    g.fillRect(loopStartRect);
    
    const auto loopEndRect = getLoopEndEdgeRect();
    g.setColour(MelissaUISettings::getAccentColour(mouseOnLoopEndEdge_ ? 1.f : 0.6f));
    g.fillRect(loopEndRect);
}

void MelissaLoopRangeComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    aRatio_ = aRatio;
    bRatio_ = bRatio;
    repaint();
}

void MelissaLoopRangeComponent::mouseDown(float xRatio, bool isLeft)
{
    const auto mousePoint = juce::Point<float>(xRatio * getWidth(), 0);
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
    
    mouseClickXRatio_ = xRatio;
}

void MelissaLoopRangeComponent::mouseUp(float xRatio)
{
    if (mouseStatus_ == kMouseStatus_None)
    {
        // clicked
        model_->setPlayingPosRatio(xRatio);
    }
    else
    {
        model_->setLoopPosRatio(aRatio_, bRatio_);
    }
    mouseStatus_ = kMouseStatus_None;
}

void MelissaLoopRangeComponent::mouseMove(float xRatio)
{
    const auto mousePoint = juce::Point<float>(xRatio * getWidth(), 0);
    mouseOnLoopStartEdge_ = getLoopStartEdgeRect().contains(mousePoint);
    mouseOnLoopEndEdge_   = getLoopEndEdgeRect().contains(mousePoint);
    
    repaint();
}

void MelissaLoopRangeComponent::mouseDrag(float xRatio)
{
    const auto mousePosRatio = std::clamp(xRatio, 0.f, 1.f);
    
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
    else if (abs(mouseClickXRatio_ - xRatio) * getWidth() > 4)
    {
        mouseStatus_ = kMouseStatus_Range;
        if (xRatio >= mouseClickXRatio_)
        {
            aRatio_ = mouseClickXRatio_;
            draggingLoopStart_ = true;
        }
        else
        {
            bRatio_ = mouseClickXRatio_;
            draggingLoopStart_ = false;
        }

    }
    
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
