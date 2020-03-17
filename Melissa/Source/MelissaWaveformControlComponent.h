#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLabel.h"
#include "MelissaModel.h"

class MelissaWaveformControlComponent : public Component,
                                        public Timer
                                        
{
public:
    MelissaWaveformControlComponent();
    virtual ~MelissaWaveformControlComponent();
    
    void resized() override;
    
    void timerCallback() override;
    
    void setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate);
    void setPlayPosition(float ratio);
    void showTimeTooltip(float posRatio);
    
private:
    class TimeLineBar;
    class WaveformView;
    
    std::unique_ptr<TimeLineBar> timeLineBar_;
    std::unique_ptr<WaveformView> waveformView_;
    std::unique_ptr<MelissaLabel> posTooltip_;
    float timeSec_;
};
