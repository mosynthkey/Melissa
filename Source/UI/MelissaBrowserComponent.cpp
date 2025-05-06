//
//  MelissaBrowserComponent.cpp
//  Melissa
//

#include "MelissaBrowserComponent.h"

MelissaBrowserComponent::MelissaBrowserComponent() : 
    goButton_("", DrawableButton::ImageRaw), 
    homeButton_("", DrawableButton::ImageRaw), 
    backButton_("", DrawableButton::ImageRaw), 
    forwardButton_("", DrawableButton::ImageRaw), 
    homeURL_("https://google.com")
{
    iconImages_[kIcon_Back] = juce::Drawable::createFromImageData(BinaryData::left_svg, BinaryData::left_svgSize);
    iconImages_[kIcon_Forward] = juce::Drawable::createFromImageData(BinaryData::right_svg, BinaryData::right_svgSize);
    iconImages_[kIcon_Home] = juce::Drawable::createFromImageData(BinaryData::home_svg, BinaryData::home_svgSize);
    iconImages_[kIcon_Go] = juce::Drawable::createFromImageData(BinaryData::go_svg, BinaryData::go_svgSize);

    iconHighlightedImages_[kIcon_Back] = juce::Drawable::createFromImageData(BinaryData::left_svg, BinaryData::left_svgSize);
    iconHighlightedImages_[kIcon_Forward] = juce::Drawable::createFromImageData(BinaryData::right_svg, BinaryData::right_svgSize);
    iconHighlightedImages_[kIcon_Home] = juce::Drawable::createFromImageData(BinaryData::home_svg, BinaryData::home_svgSize);
    iconHighlightedImages_[kIcon_Go] = juce::Drawable::createFromImageData(BinaryData::go_svg, BinaryData::go_svgSize);
    for (int iconIndex = 0; iconIndex < kNumOfIcons; ++iconIndex) 
    {
        iconImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getMainColour());
        iconImages_[iconIndex]->replaceColour(Colours::black, MelissaUISettings::getTextColour(0.6f));
        iconHighlightedImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getMainColour());
        iconHighlightedImages_[iconIndex]->replaceColour(Colours::black, MelissaUISettings::getTextColour());
    }

    addAndMakeVisible(urlTextEditor_);
    urlTextEditor_.addListener(this);
    urlTextEditor_.setLookAndFeel(this);
    urlTextEditor_.setTextToShowWhenEmpty("Enter URL...", Colours::grey);

    addAndMakeVisible(goButton_);
    goButton_.setImages(iconImages_[kIcon_Go].get(), iconHighlightedImages_[kIcon_Go].get());
    goButton_.onClick = [this]() { goToURL(urlTextEditor_.getText()); };

    addAndMakeVisible(homeButton_);
    homeButton_.setImages(iconImages_[kIcon_Home].get(), iconHighlightedImages_[kIcon_Home].get());
    homeButton_.onClick = [this]() { goHome(); };

    addAndMakeVisible(backButton_);
    backButton_.setImages(iconImages_[kIcon_Back].get(), iconHighlightedImages_[kIcon_Back].get());
    backButton_.onClick = [this]() { goBack(); };

    addAndMakeVisible(forwardButton_);
    forwardButton_.setImages(iconImages_[kIcon_Forward].get(), iconHighlightedImages_[kIcon_Forward].get());
    forwardButton_.onClick = [this]() { goForward(); };

    webBrowser_ = std::make_unique<CustomWebBrowserComponent>(*this);
    addAndMakeVisible(webBrowser_.get());

    goToURL(homeURL_);
    
    MelissaDataSource::getInstance()->addListener(this);
}

MelissaBrowserComponent::~MelissaBrowserComponent()
{
}

void MelissaBrowserComponent::resized()
{
    auto r = getLocalBounds();
    auto topBar = r.removeFromTop(30);

    backButton_.setBounds(topBar.removeFromLeft(30));
    topBar.removeFromLeft(10);
    forwardButton_.setBounds(topBar.removeFromLeft(30));
    topBar.removeFromLeft(10);
    homeButton_.setBounds(topBar.removeFromLeft(30));
    topBar.removeFromLeft(10);
    goButton_.setBounds(topBar.removeFromRight(30));
    topBar.removeFromRight(10);
    urlTextEditor_.setBounds(topBar);

    r.removeFromTop(10);
    webBrowser_->setBounds(r);
}

void MelissaBrowserComponent::textEditorReturnKeyPressed(TextEditor & editor)
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

void MelissaBrowserComponent::songChanged(const juce::String&, size_t, int32_t)
{
    goToURL(MelissaDataSource::getInstance()->getBrowserUrl());
}
