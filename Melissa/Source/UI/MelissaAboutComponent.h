//
//  MelissaAboutComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BinaryData.h"

class MelissaAboutComponent : public Component
{
public:
    MelissaAboutComponent()
    {
        setSize(600, 290);
        
        imageComponent_ = std::make_unique<ImageComponent>();
        imageComponent_->setImage(ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));
        imageComponent_->setBounds(0, 0, 600, 200);
        addAndMakeVisible(imageComponent_.get());
        
        copyrightLabel_ = std::make_unique<Label>();
        copyrightLabel_->setJustificationType(Justification::right);
        copyrightLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        copyrightLabel_->setText("Copyright (c) 2022 Masaki Ono. All rights reserved.", dontSendNotification);
        copyrightLabel_->setBounds(0, 200, 600, 30);
        addAndMakeVisible(copyrightLabel_.get());
        
        versionLabel_ = std::make_unique<Label>();
        versionLabel_->setJustificationType(Justification::right);
        versionLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        versionLabel_->setText(String("Version ") + ProjectInfo::versionString + String(" (Build: ") + __DATE__  + String(")"), dontSendNotification);
        versionLabel_->setBounds(0, 230, 600, 30);
        addAndMakeVisible(versionLabel_.get());
        
        gitHubLinkButton_ = std::make_unique<HyperlinkButton>("Open GitHub page", URL("https://github.com/mosynthkey/Melissa"));
        gitHubLinkButton_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main), false, Justification::right);
        gitHubLinkButton_->setSize(160, 30);
        gitHubLinkButton_->setTopRightPosition(600, 260);
        addAndMakeVisible(gitHubLinkButton_.get());
        
        websiteLinkButton_ = std::make_unique<HyperlinkButton>("Visit official site", URL("https://mosynthkey.github.io/Melissa/"));
        websiteLinkButton_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main), false, Justification::right);
        websiteLinkButton_->setSize(160, 30);
        websiteLinkButton_->setTopRightPosition(gitHubLinkButton_->getX() - 10, 260);
        addAndMakeVisible(websiteLinkButton_.get());
    }
    
private:
    std::unique_ptr<ImageComponent> imageComponent_;
    std::unique_ptr<Label> copyrightLabel_;
    std::unique_ptr<Label> versionLabel_;
    std::unique_ptr<HyperlinkButton> websiteLinkButton_;
    std::unique_ptr<HyperlinkButton> gitHubLinkButton_;
};
