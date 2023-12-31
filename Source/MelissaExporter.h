/*
  ==============================================================================

    MelissaExporter.h
    Created: 21 May 2023 8:59:16am
    Author:  Masaki Ono

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SoundTouch.h"

class MelissaExporter
{
public:
    enum ExportFormat
    {
        kExportFormat_wav48000_24,
        kExportFormat_wav44100_16,
        kExportFormat_ogg48000,
        kExportFormat_ogg44100,
    };
    
    MelissaExporter();
    ~MelissaExporter();
    
    using FileAndVolume = std::pair<juce::File, float>;
    
    void addInputFile(std::vector<FileAndVolume> fileAndVolumes_, float pitch, float speed, float startRatio, float endRatio, bool eqSwitch, float eqFreq, float eqGain, float eqQ, uint32_t gapMSec, uint32_t fadeInMSec = 0, uint32_t fadeOutMSec = 0);
    void setExportSettings(ExportFormat format, juce::File filePathToExport) { format_ = format; filePathToExport_ = filePathToExport; }
    void exportToFile();
    
private:
    juce::AudioFormatManager formatManager_;
    
    struct Input
    {
        using ReaderAndVolume = std::pair<std::unique_ptr<juce::AudioFormatReader>, float>;
        std::vector<ReaderAndVolume> readerAndVolumes;
        float pitch;
        float speed;
        juce::int64 startSampleIndex;
        juce::int64 endSampleIndex;
        bool eqSwitch;
        float eqFreq;
        float eqGain;
        float eqQ;
        juce::int64 fadeInNumSamples;
        juce::int64 fadeOutNumSamples;
        uint32_t gapMSec;
        
    };
    std::vector<std::unique_ptr<Input>> inputs_;
    ExportFormat format_;
    juce::File filePathToExport_;
};
