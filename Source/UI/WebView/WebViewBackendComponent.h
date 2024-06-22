#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "SinglePageBrowser.h"

//==============================================================================
class WebViewBackendComponent
    : public juce::Component
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
            [safe_this = juce::Component::SafePointer(this)]
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
                return;
            })
        .withResourceProvider([this](const auto& url)
        {
            return getResource(url);
        },
        juce::URL { SinglePageBrowser::localDevServerAddress }.getOrigin())
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewBackendComponent)
};
