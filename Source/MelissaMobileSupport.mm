//
//  MelissaMobileSupport.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaMobileSupport.h"


File MelissaMobileSupport::importFile(const URL& fileUrl)
{
    const auto songName = fileUrl.getFileName();
    
    const auto documentDir = File::getSpecialLocation(File::userDocumentsDirectory);
    const auto fileDir = documentDir; //documentDir.getChildFile(songName);
    
    /*
    if (fileDir.exists() && !fileDir.existsAsFile()) return File();
    
    // Create dirctory and copy
    fileDir.createDirectory();
     */
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

MelissaAdComponent::MelissaAdComponent()
{
    
}

MelissaAdComponent::~MelissaAdComponent()
{
    
}

void MelissaAdComponent::resized()
{
    
}

void MelissaAdComponent::paint(Graphics& g)
{
    
}
