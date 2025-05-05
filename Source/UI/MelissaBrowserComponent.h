//
//  MelissaBrowserComponent.h
//  Melissa
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"
#include "MelissaDataSource.h"

class MelissaBrowserComponent : public Component,
                                public TextEditor::Listener,
                                public juce::LookAndFeel_V4,
                                public MelissaDataSourceListener
{
public:
    MelissaBrowserComponent();
    ~MelissaBrowserComponent() override;

    void resized() override;
    void paint(Graphics &g) override
    {
        g.fillAll(MelissaUISettings::getSubColour());
    }

    void goToURL(const String &url);
    void goHome();
    void goBack();
    void goForward();

    void textEditorReturnKeyPressed(TextEditor &editor) override;

private:
    class CustomWebBrowserComponent : public juce::WebBrowserComponent
    {
    public:
        CustomWebBrowserComponent(MelissaBrowserComponent& owner) : owner_(owner) {}
        
        void pageFinishedLoading(const String& url) override
        {
            owner_.urlTextEditor_.setText(url, false);
            WebBrowserComponent::pageFinishedLoading(url);
            MelissaDataSource::getInstance()->setBrowserUrl(url);
        }
        
    private:
        MelissaBrowserComponent& owner_;
    };

    void songChanged(const juce::String &filePath, size_t bufferLength, int32_t sampleRate) override;

    std::unique_ptr<CustomWebBrowserComponent> webBrowser_;
    juce::TextEditor urlTextEditor_;
    juce::DrawableButton goButton_;
    juce::DrawableButton homeButton_;
    juce::DrawableButton backButton_;
    juce::DrawableButton forwardButton_;
    String homeURL_;

    enum
    {
        kIcon_Back,
        kIcon_Forward,
        kIcon_Home,
        kIcon_Go,
        kNumOfIcons
    };
    std::unique_ptr<juce::Drawable> iconImages_[kNumOfIcons];
    std::unique_ptr<juce::Drawable> iconHighlightedImages_[kNumOfIcons];

    virtual void fillTextEditorBackground(juce::Graphics &g, int width, int height, juce::TextEditor &te) override
    {
        const auto &c = te.getLocalBounds();
        g.setColour(MelissaUISettings::getMainColour());
        g.fillRoundedRectangle(0, 0, c.getWidth(), c.getHeight(), c.getHeight() / 2);
    }

    virtual void drawTextEditorOutline(juce::Graphics &g, int width, int height, juce::TextEditor &te) override
    {
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaBrowserComponent)
};
