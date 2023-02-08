//
//  MelissaTutorialComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <tuple>
#include "MelissaDataSource.h"
#include "MelissaTutorialComponent.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

using namespace juce;

MelissaTutorialComponent::MelissaTutorialComponent(MelissaHost* host) : host_(host), currentPage_(0)
{
    setInterceptsMouseClicks(false, true);
    skipButton_ = std::make_unique<TextButton>();
    skipButton_->setSize(100, 30);
    skipButton_->setButtonText(TRANS("skip"));
    skipButton_->onClick = [&]()
    {
        host_->closeTutorial();
    };
    addAndMakeVisible(skipButton_.get());
    
    nextButton_ = std::make_unique<TextButton>();
    nextButton_->setSize(100, 30);
    nextButton_->setButtonText(TRANS("next"));
    nextButton_->onClick = [&]()
    {
        ++currentPage_;
        if (currentPage_ < pages_.size())
        {
            update();
        }
        else
        {
            host_->closeTutorial();
        }
    };
    addAndMakeVisible(nextButton_.get());
    
    explanationLabel_ = std::make_unique<Label>();
    explanationLabel_->setColour(Label::textColourId, Colours::white);
    explanationLabel_->setSize(100, 30);
    addAndMakeVisible(explanationLabel_.get());
}

void MelissaTutorialComponent::setPages(const std::vector<Page>& pages)
{
    pages_ = pages;
    currentPage_ = 0;
    update();
}

void MelissaTutorialComponent::update()
{
    const size_t numOfPages = pages_.size();
    if (numOfPages == 0) return;
    if (currentPage_ < numOfPages - 1)
    {
        nextButton_->setButtonText(TRANS("next"));
    }
    else
    {
        nextButton_->setButtonText(TRANS("end"));
    }
    
    auto* targetComponent = pages_[currentPage_].targetComponent_;
    auto explanation = pages_[currentPage_].explanation;
    
    const auto targetRect = targetComponent->getBounds().expanded(4, 4);
    explanationLabel_->setText(explanation, dontSendNotification);
    explanationLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    const auto labelSize = MelissaUtility::getStringSize(explanationLabel_->getFont(), explanation);
    explanationLabel_->setSize(labelSize.first + 10, labelSize.second);
    
    int expLabalY = targetRect.getBottom() + 20;
    if (getHeight() < expLabalY) expLabalY = targetRect.getY() - 20 - (explanationLabel_->getHeight() + 10 + nextButton_->getHeight());
    explanationLabel_->setTopLeftPosition(targetRect.getX(), expLabalY);
    
    int explanationLabelX = explanationLabel_->getRight() - nextButton_->getWidth();
    if (getWidth() <= explanationLabelX + explanationLabel_->getWidth())
    {
        explanationLabelX = explanationLabel_->getRight() - (nextButton_->getWidth());
    }
    nextButton_->setTopLeftPosition(explanationLabelX, explanationLabel_->getBottom() + 10);
    
    targetRectangle_.setBounds(targetRect.getX(), targetRect.getY(), targetRect.getWidth(), targetRect.getHeight());
    repaint();
}

void MelissaTutorialComponent::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    
    const int margin = 20;
    skipButton_->setTopLeftPosition(w - skipButton_->getWidth() - margin, h - skipButton_->getHeight() - margin);
    update();
}

void MelissaTutorialComponent::paint(Graphics& g)
{
    g.excludeClipRegion(targetRectangle_);
    g.fillAll(Colours::black.withAlpha(0.8f));
    
    g.setColour(Colours::white.withAlpha(0.4f));
    const int margin = 8;
    const int x = explanationLabel_->getX() - margin;
    const int y = explanationLabel_->getY() - margin;
    g.drawRoundedRectangle(x, y, nextButton_->getRight() + margin - x, nextButton_->getBottom() + margin - y, 10, 2.f);
}
