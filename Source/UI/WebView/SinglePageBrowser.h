#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
class SinglePageBrowser
    : public juce::WebBrowserComponent
{
public:
    //==============================================================================
    // Inherit constructor from parent.
    using juce::WebBrowserComponent::WebBrowserComponent;

    // Prevent page loads from navigating away from our single page web app
    bool pageAboutToLoad (const juce::String& newURL) override;

    //==============================================================================
    static const juce::String localDevServerAddress;
    static const juce::String fallbackPageHtml;

private:
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SinglePageBrowser)
};
