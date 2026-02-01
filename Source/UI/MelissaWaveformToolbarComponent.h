//
//  MelissaWaveformToolbarComponent.h
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#pragma once

#include <JuceHeader.h>
#include "MelissaLookAndFeel.h"
#include "MelissaModel.h"
#include "MelissaDataSource.h"
#include "BinaryData.h"

class MelissaWaveformToolbarComponent : public juce::Component, public MelissaDataSourceListener
{
public:
    MelissaWaveformToolbarComponent();
    ~MelissaWaveformToolbarComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // MelissaDataSourceListener
    void waveformZoomChanged(float zoomValue) override;
    void waveformFollowChanged(bool followPlayingPosition) override;
    void waveformSnapChanged(bool snapToBeats) override;

private:
    std::unique_ptr<juce::Label> waveformLabel_;
    std::unique_ptr<juce::Label> zoomLabel_;
    std::unique_ptr<juce::Label> followLabel_;
    std::unique_ptr<juce::ToggleButton> followToggleButton_;
    std::unique_ptr<juce::Label> autoLabel_;
    std::unique_ptr<juce::ToggleButton> autoSnapToggleButton_;
    std::unique_ptr<juce::Slider> zoomSlider_;
    std::unique_ptr<juce::TextButton> zoomResetButton_;
    std::unique_ptr<juce::TextButton> loopSnapButton_;

    MelissaLookAndFeel laf_;
    MelissaLookAndFeel_SlideToggleButton slideToggleButtonLaf_;
    MelissaLookAndFeel_ZoomSlider zoomSliderLaf_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaWaveformToolbarComponent)
};
