#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Melissa.h"
#include "MelissaControlComponent.h"
#include "MelissaDebugComponent.h"
#include "MelissaIncDecButton.h"
#include "MelissaLookAndFeel.h"
#include "MelissaPlayPauseButton.h"
#include "MelissaWaveformControlComponent.h"

class MainComponent   : public AudioAppComponent,
                        public FileBrowserListener,
                        public KeyListener,
                        public MelissaWaveformControlListener,
                        public Timer,
                        public Thread
{
public:
    MainComponent();
    ~MainComponent();
    
    // AudioAppComponent
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void paint (Graphics& g) override;
    void resized() override;
    
    // FileBrowserListener
    void selectionChanged() override {};
    void fileClicked(const File& file, const MouseEvent& e) override {}
    void fileDoubleClicked(const File& file) override;
    void browserRootChanged(const File& newRoot) override {};
    
    // KeyListener
    bool keyPressed(const KeyPress &key, Component* originatingComponent) override;
    
    // MelissaWaveformControlListener
    void setPlayPosition(MelissaWaveformControlComponent* sender, float ratio) override;
    void setAPosition(MelissaWaveformControlComponent* sender, float ratio) override;
    void setBPosition(MelissaWaveformControlComponent* sender, float ratio) override;
    
    // Thread
    void run() override;
    
    // Timer
    void    timerCallback() override;
    
    enum Status
    {
        kStatus_Playing,
        kStatus_Pause,
        kStatus_Stop
    } status_;
    bool    openFile(const File& file);
    void    play();
    void    pause();
    void    stop();
    void    resetLoop();
    
    void updateAll();
    void updateAButtonLabel();
    void updateBButtonLabel();
    void updateSpeedButtonLabel();
    void updatePitchButtonLabel();

private:
    std::unique_ptr<Melissa> melissa_;
    std::unique_ptr<AudioSampleBuffer> audioSampleBuf_;
    
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<MelissaControlComponent> controlComponent_;
    std::unique_ptr<MelissaPlayPauseButton> playPauseButton_;
    
    std::unique_ptr<TextButton> aSetButton_;
    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<TextButton> bSetButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    
    std::unique_ptr<Label> speedLabel_;
    std::unique_ptr<MelissaIncDecButton> speedButton_;
    std::unique_ptr<Label> pitchLabel_;
    std::unique_ptr<MelissaIncDecButton> pitchButton_;
    
    std::unique_ptr<MelissaDebugComponent> debugComponent_;
    
    MelissaLookAndFeel lookAndFeel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
