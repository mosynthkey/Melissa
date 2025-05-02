//
//  MelissaBrowserComponent.cpp
//  Melissa
//

#include "MelissaBrowserComponent.h"

MelissaBrowserComponent::MelissaBrowserComponent() : homeURL_("https://google.com")
{
    addAndMakeVisible(urlTextEditor_);
    urlTextEditor_.addListener(this);
    urlTextEditor_.setTextToShowWhenEmpty("Enter URL...", Colours::grey);

    addAndMakeVisible(goButton_);
    goButton_.setButtonText("Go");
    goButton_.onClick = [this]() { goToURL(urlTextEditor_.getText()); };

    addAndMakeVisible(homeButton_);
    homeButton_.setButtonText("Home");
    homeButton_.onClick = [this]() { goHome(); };

    addAndMakeVisible(backButton_);
    backButton_.setButtonText("<");
    backButton_.onClick = [this]() { goBack(); };

    addAndMakeVisible(forwardButton_);
    forwardButton_.setButtonText(">");
    forwardButton_.onClick = [this]() { goForward(); };

    webBrowser_ = std::make_unique<juce::WebBrowserComponent>();
    addAndMakeVisible(webBrowser_.get());

    goToURL(homeURL_);
}

MelissaBrowserComponent::~MelissaBrowserComponent()
{
}

void MelissaBrowserComponent::resized()
{
    auto r = getLocalBounds();
    auto topBar = r.removeFromTop(40);

    backButton_.setBounds(topBar.removeFromLeft(40).reduced(2));
    forwardButton_.setBounds(topBar.removeFromLeft(40).reduced(2));
    homeButton_.setBounds(topBar.removeFromLeft(60).reduced(2));

    goButton_.setBounds(topBar.removeFromRight(40).reduced(2));
    urlTextEditor_.setBounds(topBar.reduced(2));

    webBrowser_->setBounds(r);
}

void MelissaBrowserComponent::textEditorReturnKeyPressed(TextEditor &editor)
{
    if (&editor == &urlTextEditor_)
    {
        goToURL(urlTextEditor_.getText());
    }
}

void MelissaBrowserComponent::goToURL(const String &url)
{
    if (url.isEmpty())
        return;

    String processedUrl = url;
    if (!url.containsIgnoreCase("://"))
    {
        processedUrl = "https://" + url;
    }

    webBrowser_->goToURL(processedUrl);
    urlTextEditor_.setText(processedUrl, false);
}

void MelissaBrowserComponent::goHome()
{
    goToURL(homeURL_);
}

void MelissaBrowserComponent::goBack()
{
    webBrowser_->goBack();
}

void MelissaBrowserComponent::goForward()
{
    webBrowser_->goForward();
}

String MelissaBrowserComponent::getCurrentURL() const
{
    return urlTextEditor_.getText();
}

Point<int> MelissaBrowserComponent::getScrollPosition() const
{
    return Point<int>(0, 0);
}

void MelissaBrowserComponent::restoreState(const String &url, const Point<int> &scrollPosition)
{
    if (url.isNotEmpty())
    {
        goToURL(url);
    }
}
