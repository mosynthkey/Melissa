//
//  MelissaAboutComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BinaryData.h"

class MelissaAboutComponent : public juce::Component, public juce::LookAndFeel_V4
{
public:
    MelissaAboutComponent()
    {
        using namespace juce;
        setSize(600, 520);
        imageComponent_ = std::make_unique<ImageComponent>();
        imageComponent_->setImage(ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));
        imageComponent_->setBounds(0, 0, 600, 200);
        addAndMakeVisible(imageComponent_.get());
        
        copyrightLabel_ = std::make_unique<juce::Label>();
        copyrightLabel_->setJustificationType(Justification::right);
        copyrightLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        copyrightLabel_->setText("Copyright (c) Masaki Ono. All rights reserved.", dontSendNotification);
        copyrightLabel_->setBounds(0, 200, 600, 30);
        addAndMakeVisible(copyrightLabel_.get());
        
        versionLabel_ = std::make_unique<juce::Label>();
        versionLabel_->setJustificationType(Justification::right);
        versionLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
#ifdef MELISSA_FULL_VERSION
        versionLabel_->setText(String("Version ") + ProjectInfo::versionString + juce::String(" (Build: ") + __DATE__  + juce::String(")"), dontSendNotification);
#else
        versionLabel_->setText(String("Version ") + ProjectInfo::versionString + juce::String(" Lite (Build: ") + __DATE__  + juce::String(")"), dontSendNotification);
#endif
        versionLabel_->setBounds(0, 230, 600, 30);
        addAndMakeVisible(versionLabel_.get());
        
        licenseEditor_ = std::make_unique<TextEditor>();
        licenseEditor_->setLookAndFeel(this);
        licenseEditor_->setMultiLine(true);
        licenseEditor_->setReadOnly(true);
        licenseEditor_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Small));
        licenseEditor_->setColour(juce::TextEditor::textColourId , MelissaUISettings::getTextColour());
        licenseEditor_->setText(getLicenseText());
        licenseEditor_->setBounds(0, 270, 600, 200);
        addAndMakeVisible(licenseEditor_.get());
        
        gitHubLinkButton_ = std::make_unique<HyperlinkButton>("Open GitHub page", URL("https://github.com/mosynthkey/Melissa"));
        gitHubLinkButton_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main), false, Justification::right);
        gitHubLinkButton_->setSize(160, 30);
        gitHubLinkButton_->setTopRightPosition(600, 480);
        addAndMakeVisible(gitHubLinkButton_.get());
        
        websiteLinkButton_ = std::make_unique<HyperlinkButton>("Visit official site", URL("https://mosynthkey.github.io/Melissa/"));
        websiteLinkButton_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main), false, Justification::right);
        websiteLinkButton_->setSize(160, 30);
        websiteLinkButton_->setTopRightPosition(gitHubLinkButton_->getX() - 10, 480);
        addAndMakeVisible(websiteLinkButton_.get());
    }
    
    juce::String getLicenseText()
    {
        juce::String text = "THIRD-PARTY SOFTWARE LICENSES\n\n";
        
        // JUCE
        text += "JUCE - https://juce.com/\n";
        text += "License: Starter\n\n";
        
        // SoundTouch
        text += "SoundTouch - https://www.surina.net/soundtouch/\n";
        text += "License: LGPL v2.1\n\n";
        
        // Eigen
        text += "Eigen - https://eigen.tuxfamily.org/\n";
        text += "License: MPL2\n\n";
        
        // JSON for Modern C++
        text += "JSON for Modern C++ - https://github.com/nlohmann/json\n";
        text += "License: MIT\n\n";
        
        // Tensorflow
        text += "TensorFlow - https://www.tensorflow.org/\n";
        text += "License: Apache 2.0\n\n";
        
        // Demucs
        text += "Demucs - https://github.com/facebookresearch/demucs\n";
        text += "License: MIT\n\n";

        // Demucs.cpp
        text += "Demucs.cpp - https://github.com/sevagh/demucs.cpp\n";
        text += "License: MIT\n\n";

        // Spleeter
        text += "Spleeter - https://github.com/deezer/spleeter\n";
        text += "License: MIT\n\n";
        
        // Spleeterpp
        text += "Spleeterpp - https://github.com/gvne/spleeterpp\n";
        text += "License: MIT\n\n";

        // Spleet
        text += "Spleet - https://github.com/gvne/spleet\n";
        text += "License: MIT\n\n";
        
        // OpenBLAS
        text += "OpenBLAS - https://www.openblas.net/\n";
        text += "License: BSD\n\n";
        
        // Beat and Tempo Tracking
        /*
        text += "Beat and Tempo Tracking - https://github.com/adamstark/Beat-and-Tempo-Tracking\n";
        text += "License: MIT\n\n";
         */
        
        text += "For complete license texts, please visit each project's website.";
        
        return text;
    }

    virtual void fillTextEditorBackground(Graphics &g, int width, int height, TextEditor &) override
    {
        g.fillAll(MelissaUISettings::getSubColour());
    }

    virtual void drawTextEditorOutline(Graphics &, int width, int height, TextEditor &) override
    {

    }

    virtual void drawScrollbar(juce::Graphics &g, juce::ScrollBar &scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        using namespace juce;
        g.setColour(MelissaUISettings::getSubColour());

        if (isScrollbarVertical)
        {
            g.fillRoundedRectangle(x, y + thumbStartPosition, width, thumbSize, width / 2);
        }
        else
        {
            g.fillRoundedRectangle(x + thumbStartPosition, y, thumbSize, height, height / 2);
        }
    }

    private : std::unique_ptr<juce::ImageComponent> imageComponent_;
    std::unique_ptr<juce::Label> copyrightLabel_;
    std::unique_ptr<juce::Label> versionLabel_;
    std::unique_ptr<juce::TextEditor> licenseEditor_;
    std::unique_ptr<juce::HyperlinkButton> websiteLinkButton_;
    std::unique_ptr<juce::HyperlinkButton> gitHubLinkButton_;
};
