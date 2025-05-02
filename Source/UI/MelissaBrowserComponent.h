//
//  MelissaBrowserComponent.h
//  Melissa
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaBrowserComponent : public Component, public TextEditor::Listener
{
public:
    MelissaBrowserComponent();
    ~MelissaBrowserComponent() override;

    void resized() override;

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
    TextEditor urlTextEditor_;
    TextButton goButton_;
    TextButton homeButton_;
    TextButton backButton_;
    TextButton forwardButton_;

    String homeURL_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaBrowserComponent)
};
