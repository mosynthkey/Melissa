#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLabel.h"

class MelissaWaveformControlComponent;

class MelissaWaveformControlListener
{
public:
    virtual ~MelissaWaveformControlListener() {};
    virtual void setPlayPosition(MelissaWaveformControlComponent* sender, float ratio) {};
    virtual void setAPosition(MelissaWaveformControlComponent* sender, float ratio) {};
    virtual void setBPosition(MelissaWaveformControlComponent* sender, float ratio) {};
};

class MelissaWaveformControlComponent : public Component,
                                        public Timer
{
public:
    MelissaWaveformControlComponent();
    virtual ~MelissaWaveformControlComponent();
    
    void resized() override;
    
    void timerCallback() override;
    
    void setListener(MelissaWaveformControlListener* listener);
    void setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate);
    void setABPosition(float aRatio, float bRatio);
    void setPlayPosition(float ratio);
    void showTimeTooltip(float posRatio);
    
private:
    class TimeLineBar;
    class WaveformView;
    
    MelissaWaveformControlListener* listener_;
    std::unique_ptr<Label> aLabel_, bLabel_;
    std::unique_ptr<TimeLineBar> timeLineBar_;
    std::unique_ptr<WaveformView> waveformView_;
    std::unique_ptr<MelissaLabel> posTooltip_;
    float timeSec_;
};
