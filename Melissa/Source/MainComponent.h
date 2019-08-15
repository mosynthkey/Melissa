#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Melissa.h"
#include "MelissaControlComponent.h"
#include "MelissaDebugComponent.h"
#include "MelissaWaveformControlComponent.h"

class MelissaRoundButton;
class MelissaPlayButton;

class MainComponent   : public AudioAppComponent,
                        public FileBrowserListener,
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

private:
    std::unique_ptr<Melissa> melissa_;
    std::unique_ptr<AudioSampleBuffer> audioSampleBuf_;
    
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<MelissaControlComponent> controlComponent_;
    std::unique_ptr<MelissaRoundButton> button_;
    std::unique_ptr<MelissaPlayButton> playButton_;
    
    std::unique_ptr<MelissaDebugComponent> debugComponent_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
