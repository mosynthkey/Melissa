#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "SinglePageBrowser.h"
#include "MelissaCommand.h"
#include "MelissaModel.h"

//==============================================================================
class WebViewBackendComponent : public juce::Component, public MelissaModelListener
{
public:
    //==============================================================================
    explicit WebViewBackendComponent();
    ~WebViewBackendComponent() override;

    void emitCustomSoundChangedEvent(const juce::var& soundName);

private:
    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    //==============================================================================
    juce::MemoryInputStream misWebViewBundle;
    std::unique_ptr<juce::ZipFile> zipWebViewBundle;

    SinglePageBrowser webComponent{ 
        juce::WebBrowserComponent::Options{}
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
        .withUserDataFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)))
        .withNativeIntegrationEnabled()
        .withNativeFunction("sayHello",
                            [safe_this = juce::Component::SafePointer(this), this]
            (const juce::Array<juce::var>& args, std::function<void(juce::var)> complete)
            -> void
            {
                juce::ignoreUnused(args);
                
                if (safe_this.getComponent() == nullptr)
                {
                    complete(juce::var(""));
                    return;
                }
            
                std::cout << "Hello, Webview Melissa!" << std::endl;
                MelissaModel::getInstance()->togglePlaybackStatus();
                webComponent.emitEventIfBrowserIsVisible("helloFromC++", "test");
                return;
            })
        .withNativeFunction("excuteCommand",
                            [safe_this = juce::Component::SafePointer(this), this]
            (const juce::Array<juce::var>& args, std::function<void(juce::var)> complete)
            -> void
            {
                if (safe_this.getComponent() == nullptr)
                {
                    complete(juce::var(""));
                    return;
                }
            
                const auto commandAsStr = args[0].toString();
                const auto value        = static_cast<float>(args[1]);
                MelissaCommand::getInstance()->excuteCommand(commandAsStr, value);
                complete(juce::var(""));
                return;
            })
        .withResourceProvider([this](const auto& url)
        {
            return getResource(url);
        },
        juce::URL { SinglePageBrowser::localDevServerAddress }.getOrigin())
    };
    
    // MelissaModelListener
    void playbackStatusChanged(PlaybackStatus status) override;
    void playbackModeChanged(PlaybackMode mode) override;
    void musicVolumeChanged(float volume) override;
    void pitchChanged(float semitone) override;
    void speedChanged(int speed) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
    void metronomeSwitchChanged(bool on) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void accentChanged(int accent) override;
    void metronomeVolumeChanged(float volume) override;
    void musicMetronomeBalanceChanged(float balance) override;
    void outputModeChanged(OutputMode outputMode) override;
    void eqSwitchChanged(bool on) override;
    void eqFreqChanged(size_t band, float freq) override;
    void eqGainChanged(size_t band, float gain) override;
    void eqQChanged(size_t band, float q) override;
    void playPartChanged(PlayPart playPart) override;
    void customPartVolumeChanged(CustomPartVolume part, float volume) override;
    void mainVolumeChanged(float mainVolume) override;
    void preCountSwitchChanged(bool preCountSwitch) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewBackendComponent)
};
