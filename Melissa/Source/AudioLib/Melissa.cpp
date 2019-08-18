//
//  Melissa.cpp
//  Melissa
//
//  Created by Masaki on 2019/07/29.
//

#include <cmath>
#include <iostream>
#include <limits.h>
#include <sstream>
#include "Melissa.h"

using std::make_unique;

Melissa::Melissa() :
soundTouch_(make_unique<soundtouch::SoundTouch>()), isOriginalBufferPrepared_(false), originalSampleRate_(-1), originalBufferLength_(0), outputSampleRate_(-1),
aIndex_(0), bIndex_(0), startIndex_(0), readIndex_(0), speed_(1.f), semitone_(0), volume_(1.f), needToReset_(false), sampleTime_(0)
{
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        originalBuffer_[iCh].clear();
    }
}

void Melissa::setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate)
{
    originalSampleRate_ = sampleRate;
    
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        originalBuffer_[iCh].resize(bufferLength);
        for (int iSample = 0; iSample < bufferLength; ++iSample)
        {
            originalBuffer_[iCh][iSample] = buffer[iCh][iSample];
        }
    }
    
    aIndex_ = 0;
    bIndex_ =  bufferLength - 1;
    originalBufferLength_ = bufferLength;
    
    isOriginalBufferPrepared_ = true;
}

void Melissa::setOutputSampleRate(int32_t sampleRate)
{
    if (outputSampleRate_ == sampleRate) return;
    
    outputSampleRate_ = sampleRate;
    needToReset_ = true;
}

void Melissa::setAPosMSec(float aPosMSec)
{
    if (!isOriginalBufferPrepared_) return;
    aIndex_ = static_cast<size_t>(aPosMSec / 1000.f * originalSampleRate_);
    if (originalBufferLength_ < aIndex_) aIndex_ = 0;
    
    needToReset_ = true;
}

void Melissa::setBPosMSec(float bPosMSec)
{
    if (!isOriginalBufferPrepared_) return;
    bIndex_ = static_cast<size_t>(bPosMSec / 1000.f * originalSampleRate_);
    
    if (originalBufferLength_ < aIndex_) aIndex_ = 0;
    if (bIndex_ < aIndex_) bIndex_ = originalBufferLength_ - 1;
     startIndex_ = readIndex_;
    if (bIndex_ < readIndex_)
    {
        startIndex_ = aIndex_;
        needToReset_ = true;
    }
}

void Melissa::setPlayingPosMSec(float playingPosMSec)
{
    startIndex_ = static_cast<size_t>(playingPosMSec / 1000.f * originalSampleRate_);
    if (startIndex_ < aIndex_) startIndex_ = aIndex_;
    if (bIndex_ < startIndex_) startIndex_ = bIndex_;
    
    needToReset_ = true;
}

void Melissa::setAPosRatio(float ratio)
{
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;
    setAPosMSec((originalBufferLength_ - 1) * ratio / originalSampleRate_ * 1000.f);
}

void Melissa::setBPosRatio(float ratio)
{
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;
    setBPosMSec((originalBufferLength_ - 1) * ratio / originalSampleRate_ * 1000.f);
}

void Melissa::setPlayingPosRatio(float ratio)
{
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;
    setPlayingPosMSec((originalBufferLength_ - 1) * ratio / originalSampleRate_ * 1000.f);
}

int32_t Melissa::getTotalLengthMSec() const
{
    return static_cast<float>(originalBufferLength_) / originalSampleRate_ * 1000.f;
}

int32_t Melissa::getPlayingPosMSec() const
{
    const float ratio = soundTouch_->getInputOutputSampleRatio();
    const float elapsedTimeMSec = static_cast<float>(sampleTime_ / ratio) / outputSampleRate_ * 1000.f;
    const float loopTimeMSec    = static_cast<float>(bIndex_ - aIndex_) / originalSampleRate_ * 1000.f;
    const float startTimeMSec   = static_cast<float>(startIndex_) / originalSampleRate_ * 1000.f;
    const float aTimeMSec       = static_cast<float>(aIndex_) / originalSampleRate_ * 1000.f;
    const float startBTimeMSec  = static_cast<float>(bIndex_ - startIndex_) / originalSampleRate_ * 1000.f;
    
    if (elapsedTimeMSec < startBTimeMSec)
    {
        return startTimeMSec + elapsedTimeMSec;
    }
    else
    {
        float elapsedTimeFromA = elapsedTimeMSec - startBTimeMSec;
        for (; elapsedTimeFromA > loopTimeMSec; elapsedTimeFromA -= loopTimeMSec);
        return aTimeMSec + elapsedTimeFromA;
    }
}

float Melissa::getPlayingPosRatio() const
{
    return static_cast<float>(getPlayingPosMSec()) / 1000.f / (static_cast<float>(originalBufferLength_) / originalSampleRate_);
}

void Melissa::setSpeed(float speed)
{
    if (speed_ == speed) return;
    
    speed_ = speed;
    needToReset_ = true;
}

void Melissa::setPitch(int32_t semitone)
{
    if (semitone_ == semitone) return;
    
    semitone_ = semitone;
    needToReset_ = true;
}

float Melissa::setVolume(float volume)
{
    if (volume < 0.f) volume = 0.f;
    if (2.f < volume) volume = 2.f;
    
    volume_ = volume;
    
    return volume_;
}

void Melissa::render(float* bufferToRender[], size_t bufferLength)
{
    if (!isOriginalBufferPrepared_) return;
    
    for (int iSample = 0; iSample < bufferLength; ++iSample)
    {
        mutex_.lock();
        if (processedBufferQue_.size() > 0)
        {
            bufferToRender[0][iSample] = processedBufferQue_[0];
            bufferToRender[1][iSample] = processedBufferQue_[1];
            processedBufferQue_.erase(processedBufferQue_.begin(), processedBufferQue_.begin() + 2);
        }
        mutex_.unlock();
    }
    
    sampleTime_ += bufferLength;
}

void Melissa::process()
{
    if (!isOriginalBufferPrepared_) return;
    if (needToReset_) resetProcessedBuffer();
    
    uint32_t receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    while (receivedSampleSize == 0)
    {
        for (size_t iSample = 0; iSample < processLength_; ++iSample)
        {
            if (readIndex_ > bIndex_) readIndex_ = aIndex_;
            bufferForSoundTouch_[iSample * 2 + 0] = originalBuffer_[0][readIndex_];
            bufferForSoundTouch_[iSample * 2 + 1] = originalBuffer_[1][readIndex_];
            ++readIndex_;
        }
        soundTouch_->putSamples(bufferForSoundTouch_, processLength_);
        receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    }
    
    for (size_t iSample = 0; iSample < receivedSampleSize * 2; ++iSample)
    {
        mutex_.lock();
        processedBufferQue_.emplace_back(bufferForSoundTouch_[iSample] * volume_);
        mutex_.unlock();
    }
    
}

bool Melissa::needToProcess() const
{
    return (processedBufferQue_.size() < queLength_);
}

bool Melissa::isBufferSet() const
{
    return originalBufferLength_ != 0;
}

void Melissa::reset()
{
    needToReset_ = true;
    isOriginalBufferPrepared_  = false;
    
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        originalBuffer_[iCh].clear();
    }
    processedBufferQue_.clear();
}

void Melissa::resetProcessedBuffer()
{
    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
    
    soundTouch_->clear();
    soundTouch_->setChannels(2);
    soundTouch_->setSampleRate(originalSampleRate_);
    soundTouch_->setTempo(fsConvPitch * speed_);
    soundTouch_->setPitch(fsConvPitch * exp(0.69314718056 * semitone_ / 12.f));
    
    mutex_.lock();
    processedBufferQue_.clear();
    mutex_.unlock();
    
    sampleTime_ = 0;
    readIndex_ = startIndex_ = aIndex_; // tentative
    needToReset_ = false;
}

std::string Melissa::getStatusString() const
{
    std::stringstream ss;
    
    constexpr size_t numOfBlocks = 60;
    
    for (size_t iBlock = 0; iBlock < static_cast<float>(readIndex_) / originalBufferLength_ * numOfBlocks; ++iBlock) ss << " ";;
    ss << "r" << std::endl;
    
    for (size_t iBlock = 0; iBlock < numOfBlocks; ++iBlock) ss << "#";
    ss << std::endl;
    
    for (size_t iBlock = 0; iBlock < static_cast<float>(aIndex_) / originalBufferLength_ * numOfBlocks; ++iBlock) ss << " ";
    ss << "A" << std::endl;
    
    for (size_t iBlock = 0; iBlock < static_cast<float>(bIndex_) / originalBufferLength_ * numOfBlocks; ++iBlock) ss << " ";;
    ss << "B";
    
    ss << std::endl;
    
    for (size_t iBlock = 0; iBlock < static_cast<float>(startIndex_) / originalBufferLength_ * numOfBlocks; ++iBlock) ss << " ";;
    ss << "S";
    
    return ss.str();
}
