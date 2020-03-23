#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BinaryData.h"

class MelissaAboutComponent : public Component
{
public:
    MelissaAboutComponent()
    {
        setSize(600, 260);
        
        imageComponent_ = std::make_unique<ImageComponent>();
        imageComponent_->setImage(ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));
        imageComponent_->setBounds(0, 0, 600, 200);
        addAndMakeVisible(imageComponent_.get());
        
        copyrightLabel_ = std::make_unique<Label>();
        copyrightLabel_->setJustificationType(Justification::right);
        copyrightLabel_->setFont(MelissaUISettings::FontSizeMain());
        copyrightLabel_->setText("Copyright (c) 2020 Masaki Ono. All rights reserved.", dontSendNotification);
        copyrightLabel_->setBounds(0, 200, 600, 30);
        addAndMakeVisible(copyrightLabel_.get());
        
        versionLabel_ = std::make_unique<Label>();
        versionLabel_->setJustificationType(Justification::right);
        versionLabel_->setFont(MelissaUISettings::FontSizeMain());
        versionLabel_->setText(String("Version ") + ProjectInfo::versionString, dontSendNotification);
        versionLabel_->setBounds(0, 230, 600, 30);
        addAndMakeVisible(versionLabel_.get());
    }
    
private:
    std::unique_ptr<ImageComponent> imageComponent_;
    std::unique_ptr<Label> copyrightLabel_;
    std::unique_ptr<Label> versionLabel_;
};
