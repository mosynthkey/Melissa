//
//  MelissaWaveformControlComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaLabel.h"
#include "MelissaLoopRangeComponent.h"
#include "MelissaModel.h"
#include "MelissaWaveformMouseEventComponent.h"

class MelissaWaveformControlComponent : public Component,
                                        public MelissaDataSourceListener
{
public:
    MelissaWaveformControlComponent();
    virtual ~MelissaWaveformControlComponent();
    
    void resized() override;
    
    void setPlayPosition(float ratio);
    void showTimeTooltip(float posRatio);
    void hideTimeTooltip();
    
    // MelissaDataSourceListener
    void songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate) override;
    void markerUpdated() override;
    
private:
    class WaveformView;
    std::unique_ptr<WaveformView> waveformView_;
    
    class Marker;
    std::unique_ptr<Component> markerBaseComponent_;
    std::vector<std::unique_ptr<Marker>> markers_;
    void arrangeMarkers() const;
    
    std::unique_ptr<MelissaLoopRangeComponent> loopRangeComponent_;
    std::unique_ptr<MelissaWaveformMouseEventComponent> mouseEventComponent_;
    
    std::vector<std::unique_ptr<Label>> timeLabels_;
    void arrangeTimeLabels() const;
    
    std::unique_ptr<Label> posTooltip_;
    float timeSec_;
};
