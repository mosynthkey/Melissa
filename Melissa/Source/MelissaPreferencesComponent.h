#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaLookAndFeel.h"

class MelissaPreferencesComponent : public Component
{
public:
    MelissaPreferencesComponent(AudioDeviceManager* audioDeviceManager);
    virtual ~MelissaPreferencesComponent();
    
    enum Tab
    {
        kTab_AudioMidi,
        kTab_MidiAssign,
        kNumOfTabs
    };
    void updateTab();
    
    // Component
    void paint(Graphics& graphics) override;
    void resized() override;
    
private:
    std::vector<std::unique_ptr<ToggleButton>> tabs_;
    
    AudioDeviceManager* audioDeviceManager_;
    std::unique_ptr<AudioDeviceSelectorComponent> deviceComponent_;
    
    MelissaLookAndFeel lookAndFeel_;
    MelissaLookAndFeel_Tab lookAndFeelTab_;
};

