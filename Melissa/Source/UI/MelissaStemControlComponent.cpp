//
//  MelissaStemControlComponent.cpp
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#include "MelissaDataSource.h"
#include "MelissaStemControlComponent.h"

MelissaStemControlComponent::MelissaStemControlComponent()
{
    MelissaStemProvider::getInstance()->addListener(this);
    
    auto createAndAddTextButton = [&](const String& text)
    {
        auto b = std::make_unique<TextButton>();
        b->setButtonText(text);
        addAndMakeVisible(b.get());
        return b;
    };
    
    createStemsButton_ = createAndAddTextButton("Create");
    createStemsButton_->onClick = [&]()
    {
        MelissaStemProvider::getInstance()->requestStems(File(MelissaDataSource::getInstance()->getCurrentSongFilePath()));
    };
    
    const String stemNames[] = { "Vocals", "Piano", "Bass", "Drums", "Others" };
    for (int stemTypeIndex = 0; stemTypeIndex < kNumStemTypes; ++stemTypeIndex)
    {
        stemSwitchButtons_[stemTypeIndex] = createAndAddTextButton(stemNames[stemTypeIndex]);
    }
    
    statusLabel_ = std::make_unique<Label>();
    statusLabel_->setText("Waiting", dontSendNotification);
    addAndMakeVisible(statusLabel_.get());
}

void MelissaStemControlComponent::resized()
{
    constexpr int kButtonWidth = 100;
    constexpr int kButtonHeight = 30;
    constexpr int kMargin = 10;
    
    int x = kMargin;
    createStemsButton_->setBounds(x, 0, kButtonWidth, kButtonHeight);
    x += (kButtonWidth + kMargin);
    
    statusLabel_->setBounds(x, 0, kButtonWidth, kButtonHeight);
    x += (kButtonWidth + kMargin);
    
    for (int stemTypeIndex = 0; stemTypeIndex < kNumStemTypes; ++stemTypeIndex)
    {
        stemSwitchButtons_[stemTypeIndex]->setBounds(x, 0, kButtonWidth, kButtonHeight);
        x += (kButtonWidth + kMargin);
    }
}

void MelissaStemControlComponent::stemProviderStatusChanged(StemProviderStatus status)
{
    if (status == kStemProviderStatus_Processing)
    {
        statusLabel_->setText("Processing...", dontSendNotification);
    }
    else
    {
        statusLabel_->setText("Ready", dontSendNotification);
    }
}

void MelissaStemControlComponent::stemProviderResultReported(StemProviderResult result)
{
}
