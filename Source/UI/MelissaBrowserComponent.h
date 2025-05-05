//
//  MelissaBrowserComponent.h
//  Melissa
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaUISettings.h"
class MelissaBrowserComponent : public Component, 
                                public TextEditor::Listener,
                                public juce::LookAndFeel_V4
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

    String getCurrentURL() const;
    Point<int> getScrollPosition() const;
    void restoreState(const String &url, const Point<int> &scrollPosition);

private:
    std::unique_ptr<juce::WebBrowserComponent> webBrowser_;
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
