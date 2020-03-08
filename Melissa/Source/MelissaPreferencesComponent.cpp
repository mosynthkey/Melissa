#include "MelissaColourScheme.h"
#include "MelissaPreferencesComponent.h"

MelissaPreferencesComponent::MelissaPreferencesComponent(AudioDeviceManager* audioDeviceManager) :
audioDeviceManager_(audioDeviceManager)
{
    deviceComponent_ = std::make_unique<AudioDeviceSelectorComponent>(*audioDeviceManager_, 0, 0, 0, 2, true, false, true, false);
    addAndMakeVisible(deviceComponent_.get());
    
    {
        // Tab
        auto createToggleButton = [&](String title, Tab tab, bool toggleState)
        {
            auto b = std::make_unique<ToggleButton>();
            b->setButtonText(title);
            b->setLookAndFeel(&lookAndFeelTab_);
            b->setRadioGroupId(2001);
            b->setToggleState(toggleState, dontSendNotification);
            b->onClick = [&]() { updateTab(); };
            addAndMakeVisible(b.get());
            return b;
        };
        
        String tabTitle[kNumOfTabs] = { "Audio / MIDI ", "Keyboard Shortcut", "MIDI Control Assign" };
        for (int tabs_i = 0; tabs_i < kNumOfTabs; ++tabs_i)
        {
            tabs_[tabs_i] = createToggleButton(tabTitle[tabs_i], static_cast<Tab>(tabs_i), tabs_i == 0);
        }
        updateTab();
    }
    
    setLookAndFeel(&lookAndFeel_);
    setSize(800, 600);
}

MelissaPreferencesComponent::~MelissaPreferencesComponent()
{
    setLookAndFeel(nullptr);
    for (auto&& tab : tabs_)
    {
        tab->setLookAndFeel(nullptr);
    }
}

void MelissaPreferencesComponent::updateTab()
{
    deviceComponent_->setVisible(tabs_[kTab_AudioMidi]->getToggleState());
}

void MelissaPreferencesComponent::paint(Graphics& g)
{
    g.fillAll(Colour(MelissaColourScheme::DialogBackgoundColour()));
}

void MelissaPreferencesComponent::resized()
{
    constexpr int tabWidth = 240;
    constexpr int tabHeight = 30;
    constexpr int tabMargin = 2;
    constexpr int totalTabWidth = tabWidth * kNumOfTabs + tabMargin * (kNumOfTabs - 1);
    
    int x = (getWidth() - totalTabWidth) / 2;
    for (auto&& tab : tabs_)
    {
        tab->setBounds(x, 10, tabWidth, tabHeight);
        x += (tabWidth + tabMargin);
    }
    
    constexpr int y = tabHeight + 40;
    deviceComponent_->setBounds(0, y, getWidth(), getHeight() - y);
}
