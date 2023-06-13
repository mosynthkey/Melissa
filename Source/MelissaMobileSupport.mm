//
//  MelissaMobileSupport.mm
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaMobileSupport.h"
#include "MelissaUISettings.h"
#import "AdMobAdDelegateViewController.h"

using namespace juce;

juce::File MelissaMobileSupport::importFile(const juce::URL& fileUrl)
{
    using namespace juce;
    
    const auto songName = fileUrl.getFileName();
    
    const auto documentDir = File::getSpecialLocation(File::userDocumentsDirectory);
    const auto fileDir = documentDir; //documentDir.getChildFile(songName);
    
    const File targetFile = fileDir.getChildFile(fileUrl.getFileName());
    targetFile.create();
    
    MemoryBlock memoryBlock;
    auto inputStream = fileUrl.createInputStream(false);
    if (inputStream == nullptr) return File();
    inputStream->readIntoMemoryBlock(memoryBlock);
    
    auto outputStream = targetFile.createOutputStream();
    if (outputStream == nullptr) return File();
    outputStream->write(memoryBlock.getData(), memoryBlock.getSize());
    
    return targetFile;
}

#ifdef ENABLE_MOBILEAD

AdMobAdDelegateViewController *viewController;
//RewardAdViewController *viewController;

MelissaAdComponent::MelissaAdComponent()
{
    viewController = [[AdMobAdDelegateViewController alloc] init];
    uiViewComponent.setView([viewController view]);
    addAndMakeVisible(uiViewComponent);
}

MelissaAdComponent::~MelissaAdComponent()
{
    [viewController release];
}

void MelissaAdComponent::resized()
{
    uiViewComponent.setBounds(getLocalBounds());
    viewController.view.frame = CGRectMake(0, 0, getWidth(), getHeight());
}

void MelissaAdComponent::paint(Graphics& g)
{
    g.fillAll(MelissaUISettings::getMainColour());
    g.setColour(Colours::pink);
    g.drawRect(getLocalBounds());
}
#endif
