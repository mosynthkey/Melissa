/*
  ==============================================================================

    MelissaExporter.cpp
    Created: 21 May 2023 8:59:16am
    Author:  Masaki Ono

  ==============================================================================
*/

#include "MelissaAudioEngine.h"
#include "MelissaExporter.h"

using namespace juce;

MelissaExporter::MelissaExporter()
{
    formatManager_.registerBasicFormats();
}

MelissaExporter::~MelissaExporter()
{
    
}

void MelissaExporter::addInputFile(std::vector<FileAndVolume> fileAndVolumes_, float pitch, float speed, float startRatio, float endRatio, bool eqSwitch, float eqFreq, float eqGain, float eqQ, uint32_t gapMSec, uint32_t fadeInMSec, uint32_t fadeOutMSec)
{
    auto input = std::make_unique<Input>();
    
    if (fileAndVolumes_.empty()) throw("No input files");
    
    for (auto&& fileAndVolume : fileAndVolumes_)
    {
        Input::ReaderAndVolume readerAndVolume;
        auto&& reader = readerAndVolume.first = std::unique_ptr<AudioFormatReader>(formatManager_.createReaderFor(fileAndVolume.first));
        if (reader == nullptr) throw("Cannot load this file");
        readerAndVolume.second = fileAndVolume.second;
        input->readerAndVolumes.emplace_back(std::move(readerAndVolume));
    }
    
    const auto& numOfSamples = input->readerAndVolumes[0].first->lengthInSamples;
    const auto& sampleRate   = input->readerAndVolumes[0].first->sampleRate;
    input->pitch = pitch;
    input->speed = speed;
    input->startSampleIndex  = numOfSamples * startRatio;
    input->endSampleIndex    = numOfSamples * endRatio;
    input->eqSwitch = eqSwitch;
    input->eqFreq = eqFreq;
    input->eqGain = eqGain;
    input->eqQ = eqQ;
    input->fadeInNumSamples  = fadeInMSec * sampleRate / 1000;
    input->fadeOutNumSamples = fadeOutMSec * sampleRate / 1000;
    input->gapMSec = gapMSec;
    
    if ((input->startSampleIndex - input->fadeInNumSamples) < 0) input->fadeInNumSamples = input->startSampleIndex;
    if (numOfSamples <= (input->endSampleIndex + input->fadeOutNumSamples)) input->fadeOutNumSamples = (numOfSamples - 1) - input->endSampleIndex;
    
    inputs_.emplace_back(std::move(input));
}

void MelissaExporter::exportToFile()
{
    if (inputs_.size() == 0) throw(TRANS("No input files"));
    
    constexpr int kNumChannels = 2;
    constexpr int kProcessBufferLength = 4096;
    
    float outputSampleRate = 48000;
    int outputBitsPerSample = 24;
    
    WavAudioFormat wavFormat;
    OggVorbisAudioFormat oggFormat;
    std::unique_ptr<AudioFormatWriter> writer;
    
    if (filePathToExport_.existsAsFile())
    {
        filePathToExport_.deleteFile();
    }
    
    switch (format_)
    {
        case kExportFormat_wav48000_24:
            outputSampleRate = 48000;
            outputBitsPerSample = 24;
            break;
        case kExportFormat_wav44100_16:
            outputSampleRate = 44100;
            outputBitsPerSample = 16;
            break;
        case kExportFormat_ogg48000:
            outputSampleRate = 48000;
            outputBitsPerSample = 24;
            break;
        case kExportFormat_ogg44100:
            outputSampleRate = 44100;
            outputBitsPerSample = 24;
            break;
        default:
            throw("Invalid file format");
            return;
    };
    
    if (format_ == kExportFormat_wav48000_24 || format_ == kExportFormat_wav44100_16)
    {
        writer.reset(wavFormat.createWriterFor(new FileOutputStream(filePathToExport_), outputSampleRate, kNumChannels, outputBitsPerSample, {}, 0));
    }
    else
    {
        writer.reset(oggFormat.createWriterFor(new FileOutputStream(filePathToExport_), outputSampleRate, kNumChannels, outputBitsPerSample, {}, 0));
    }
    
    if (writer == nullptr) throw("Cannot write to this file");
    
    float bufferForSoundTouch[kNumChannels * kProcessBufferLength];
    float volume[kProcessBufferLength];
    AudioBuffer<float> tempAudioBuffer(kNumChannels, kProcessBufferLength);
    AudioBuffer<float> stemAudioBuffer(kNumChannels, kProcessBufferLength);
    const float* readPointers[] = { tempAudioBuffer.getReadPointer(0), tempAudioBuffer.getReadPointer(1) };
    float* writePointers[] = { tempAudioBuffer.getWritePointer(0), tempAudioBuffer.getWritePointer(1) };
    
    for (auto&& input : inputs_)
    {
        const int inputSampleRate = input->readerAndVolumes[0].first->sampleRate;
        
        const auto fsConvPitch = inputSampleRate / static_cast<float>(outputSampleRate);
        auto soundTouch = std::make_unique<soundtouch::SoundTouch>();
        soundTouch->setChannels(kNumChannels);
        soundTouch->setSampleRate(inputSampleRate);
        soundTouch->setTempo(fsConvPitch * input->speed / 100.f);
        soundTouch->setPitch(fsConvPitch * exp(0.69314718056 * input->pitch / 12.f));
        
        const auto startIndex = input->startSampleIndex - input->fadeInNumSamples;
        const auto endIndex = input->endSampleIndex + input->fadeOutNumSamples;
        int64 readIndex = startIndex;
        bool isFlushed = false;
        
        MelissaEqualizer eq;
        eq.setSampleRate(outputSampleRate);
        eq.setFreq(input->eqFreq);
        eq.setGain(input->eqGain);
        eq.setQ(input->eqQ);
        eq.reset();
        
        while (true)
        {
            // read
            int64 numReadSamples = kProcessBufferLength;
            if (endIndex < (readIndex + numReadSamples)) numReadSamples = endIndex - readIndex;
            
            for (int index = 0; index < numReadSamples; ++index)
            {
                const int64 volumeIndex = readIndex + index;
                if (readIndex < input->startSampleIndex)
                {
                    // Fade in
                    volume[index] = (volumeIndex - startIndex) / static_cast<float>(input->fadeInNumSamples);
                }
                else if (input->startSampleIndex <= readIndex && readIndex <= input->endSampleIndex)
                {
                    volume[index] = 1.f;
                }
                else if (input->endSampleIndex < readIndex && readIndex < (input->endSampleIndex + input->fadeOutNumSamples))
                {
                    // Fade out
                    volume[index] = 1.f - (volumeIndex - input->endSampleIndex) / static_cast<float>(input->fadeOutNumSamples);
                }
            }
            
            // put to soundtouch
            if (numReadSamples != 0)
            {
                std::fill(bufferForSoundTouch, bufferForSoundTouch + (kNumChannels * kProcessBufferLength), 0.f);
                for (auto&& readerAndVolume : input->readerAndVolumes)
                {
                    readerAndVolume.first->read(&tempAudioBuffer, 0, static_cast<int>(numReadSamples), readIndex, true, true);
                    tempAudioBuffer.applyGain(readerAndVolume.second);
                    for (int index = 0; index < numReadSamples; ++index)
                    {
                        bufferForSoundTouch[index * kNumChannels + 0] += readPointers[0][index] * volume[index];
                        bufferForSoundTouch[index * kNumChannels + 1] += readPointers[1][index] * volume[index];
                    }
                }
                readIndex += numReadSamples;
                
                soundTouch->putSamples(bufferForSoundTouch, static_cast<int>(numReadSamples));
            }
            int numReceivedSamples = soundTouch->receiveSamples(bufferForSoundTouch, kProcessBufferLength);
            if (numReadSamples == 0 && numReceivedSamples == 0)
            {
                if (isFlushed)
                {
                    break;
                }
                else
                {
                    soundTouch->flush();
                    isFlushed = true;
                    numReceivedSamples = soundTouch->receiveSamples(bufferForSoundTouch, kProcessBufferLength);
                }
            }
            
            for (int index = 0; index < numReceivedSamples; ++index)
            {
                float eqBuffer[] = { bufferForSoundTouch[index * kNumChannels + 0], bufferForSoundTouch[index * kNumChannels + 1] };
                if (input->eqSwitch) eq.process(eqBuffer, eqBuffer);
                writePointers[0][index] = eqBuffer[0];
                writePointers[1][index] = eqBuffer[1];
            }
            writer->writeFromAudioSampleBuffer(tempAudioBuffer, 0, numReceivedSamples);
        }
        
        {
            // add gap
            const auto numGapSamples = input->gapMSec * outputSampleRate / 1000;
            AudioBuffer<float> blankAudioBuffer(kNumChannels, numGapSamples);
            blankAudioBuffer.clear();
            writer->writeFromAudioSampleBuffer(blankAudioBuffer, 0, numGapSamples);
        }
    }
    
    writer->flush();
}
