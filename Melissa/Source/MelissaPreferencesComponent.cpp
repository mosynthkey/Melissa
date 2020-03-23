#include "MelissaUISettings.h"
#include "MelissaPreferencesComponent.h"

MelissaPreferencesComponent::MelissaPreferencesComponent(AudioDeviceManager* audioDeviceManager) :
audioDeviceManager_(audioDeviceManager)
{
    constexpr bool showMidiSelector = false;
    deviceComponent_ = std::make_unique<AudioDeviceSelectorComponent>(*audioDeviceManager_, 0, 0, 0, 2, showMidiSelector, false, true, false);
    addAndMakeVisible(deviceComponent_.get());
    
#if defined(ENABLE_SHORTCUT_EDITOR)
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
    setSize(800, 600);
#else
    setSize(800, 300);
#endif
    
    setLookAndFeel(&lookAndFeel_);

}

MelissaPreferencesComponent::~MelissaPreferencesComponent()
{
    setLookAndFeel(nullptr);
#if defined(ENABLE_SHORTCUT_EDITOR)
    for (auto&& tab : tabs_)
    {
        tab->setLookAndFeel(nullptr);
    }
#endif
}

void MelissaPreferencesComponent::updateTab()
{
    deviceComponent_->setVisible(tabs_[kTab_AudioMidi]->getToggleState());
}

void MelissaPreferencesComponent::paint(Graphics& g)
{
    g.fillAll(Colour(MelissaUISettings::DialogBackgoundColour()));
}

void MelissaPreferencesComponent::resized()
{
#if defined(ENABLE_SHORTCUT_EDITOR)
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
#else
    deviceComponent_->setBounds(0, 0, getWidth(), getHeight());
#endif
}
