//
//  MelissaWaveformControlPopupComponent.h
//  Melissa
//
//  Copyright(c) 2025 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"
#include "MelissaModel.h"
#include "MelissaDataSource.h"
#include "BinaryData.h"

class MelissaWaveformControlPopupComponent : public juce::Component, public MelissaDataSourceListener
{
public:
    MelissaWaveformControlPopupComponent();
    ~MelissaWaveformControlPopupComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void showPopup(juce::Component* parent, juce::Rectangle<int> buttonBounds);
    void hidePopup();
    bool isVisible() const { return isPopupVisible_; }
    
    void syncWithControls(float zoomValue, bool autoSnapState, bool followState);
    float getCurrentZoomValue() const;
    bool getAutoSnapState() const;
    bool getFollowState() const;

    // MelissaDataSourceListener
    void waveformZoomChanged(float zoomValue) override;
    void waveformFollowChanged(bool followPlayingPosition) override;
    void waveformSnapChanged(bool snapToBeats) override;

    std::function<void()> onCloseClicked;
    std::function<void(float)> onZoomChanged;
    std::function<void()> onZoomResetClicked;
    std::function<void()> onLoopSnapClicked;
    std::function<void(bool)> onAutoSnapToggled;
    std::function<void(bool)> onFollowToggled;

    // Public access for LookAndFeel setting
    std::unique_ptr<juce::ToggleButton> followToggleButton_;
    std::unique_ptr<juce::ToggleButton> autoSnapToggleButton_;

private:
    bool isPopupVisible_;
    
    std::unique_ptr<juce::DrawableButton> closeButton_;
    std::unique_ptr<juce::Drawable> closeButtonDrawable_;
    std::unique_ptr<juce::Drawable> closeButtonHighlightedDrawable_;
    std::unique_ptr<juce::Label> waveformLabel_;
    std::unique_ptr<juce::Label> zoomLabel_;
    std::unique_ptr<juce::Label> followLabel_;
    std::unique_ptr<juce::Label> autoLabel_;
    std::unique_ptr<juce::Slider> zoomSlider_;
    std::unique_ptr<juce::TextButton> zoomResetButton_;
    std::unique_ptr<juce::TextButton> loopSnapButton_;

    MelissaLookAndFeel laf_;
    MelissaLookAndFeel_ZoomSlider zoomSliderLaf_;
    juce::ComponentAnimator animator_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaWaveformControlPopupComponent)
};
