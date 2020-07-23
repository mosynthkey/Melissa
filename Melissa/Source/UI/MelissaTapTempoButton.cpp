//
//  MelissaTapTempoButton.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaDefinitions.h"
#include "MelissaModel.h"
#include "MelissaTapTempoButton.h"
#include "MelissaUISettings.h"

namespace
{
constexpr float kTimeOutMSec = 60.f / kBpmMin * 1000;

template <typename T>
T estimateBpm(std::vector<T> bpms)
{
    std::sort(bpms.begin(), bpms.end());
    
    constexpr auto coef = 0.8f;
    const size_t startIndex = bpms.size() * (coef / 2);
    const size_t endIndex = (bpms.size() - 1) * (1.f - (coef / 2));
    
    if (startIndex == endIndex) return bpms[startIndex];
    
    T sum = 0;
    for (size_t i = startIndex; i <= endIndex; ++i) sum += bpms[i];
    return sum / static_cast<T>(endIndex - startIndex + 1);
}
}

MelissaTapTempoButton::MelissaTapTempoButton() :
isActive_(false),
shouldCorrect_(false)
{
    
}

void MelissaTapTempoButton::paint(Graphics& g)
{
    g.setColour(Colour(MelissaUISettings::getAccentColour()).withAlpha(0.8f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), getHeight() / 2);
    
    g.setColour(Colours::white);
    g.drawText("Tap", getLocalBounds(), Justification::centred);
}
    
void MelissaTapTempoButton::mouseDown(const MouseEvent& event)
{
    stopTimer();
    
    auto model = MelissaModel::getInstance();
    const float speed = model->getPlayingSpeed() / 100.f;
    
    const auto now = std::chrono::system_clock::now();
    if (isActive_)
    {
        const auto intervalMSec =  std::chrono::duration_cast<std::chrono::milliseconds>(now - prevTimePoint_).count();
        if (intervalMSec < kTimeOutMSec / speed)
        {
            const auto bpm = 60.f / (intervalMSec / 1000.f);
            measuredBpms_.emplace_back(bpm);
        }
        else
        {
            measuredBpms_.clear();
        }
        
        float estimatedBpm = std::round(estimateBpm(measuredBpms_));
        if (shouldCorrect_) estimatedBpm /= speed;
        model->setBpm(static_cast<int>(estimatedBpm));
        startTimer(static_cast<int>(intervalMSec * 2));
    }
    else
    {
        isActive_ = true;
        measuredBpms_.clear();
        
        startTimer(kTimeOutMSec);
    }
    
    prevTimePoint_ = now;
}

void MelissaTapTempoButton::timerCallback()
{
    stopTimer();
    isActive_ = false;
}
