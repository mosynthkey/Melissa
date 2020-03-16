#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLabel.h"
#include "MelissaModel.h"

class MelissaWaveformControlComponent : public Component,
                                        public MelissaModelListener,
                                        public Timer
                                        
{
public:
    MelissaWaveformControlComponent();
    virtual ~MelissaWaveformControlComponent();
    
    void resized() override;
    
    void timerCallback() override;
    
    void setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate);
    void setAPosition(float ratio);
    void setBPosition(float ratio);
    void setPlayPosition(float ratio);
    void showTimeTooltip(float posRatio);
    
    // MelissaModelListener
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    
private:
    class TimeLineBar;
    class WaveformView;
    
    std::unique_ptr<TimeLineBar> timeLineBar_;
    std::unique_ptr<WaveformView> waveformView_;
    std::unique_ptr<MelissaLabel> posTooltip_;
    float timeSec_;
};
