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
soundTouch_(make_unique<soundtouch::SoundTouch>()), isOriginalBufferPrepared_(false), originalSampleRate_(-1), outputSampleRate_(-1),
aIndex_(0), bIndex_(0), playingIndex_(0), readIndex_(0), writeIndex_(0),  speed_(1.f), semitone_(0), progress_(0.f), isProcessDone_(false),
isProcessedBufferPrepared_(false), totalReceivedSampleSize_(0)
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
    
    isOriginalBufferPrepared_ = true;
    isProcessDone_ = isProcessedBufferPrepared_ = false;
}

void Melissa::setOutputSampleRate(int32_t sampleRate)
{
    if (outputSampleRate_ == sampleRate) return;
    
    outputSampleRate_ = sampleRate;
    isProcessDone_ = isProcessedBufferPrepared_ = false;
}

void Melissa::setAPosMSec(float aPosMSec)
{
    if (!isOriginalBufferPrepared_) return;
    size_t newAIndex_ = static_cast<size_t>(aPosMSec / 1000.f * originalSampleRate_);
    
    const size_t bufferLength = originalBuffer_[0].size();
    if (bufferLength < newAIndex_) newAIndex_ = 0;
    
    aIndex_ = newAIndex_;
    playingIndex_ = 0;
    
    isProcessDone_ = isProcessedBufferPrepared_ = false;
}

void Melissa::setBPosMSec(float bPosMSec)
{
    if (!isOriginalBufferPrepared_) return;
    bIndex_ = static_cast<size_t>(bPosMSec / 1000.f * originalSampleRate_);
    
    const size_t bufferLength = originalBuffer_[0].size();
    if (bufferLength < aIndex_) aIndex_ = 0;
    if (bIndex_ < aIndex_) bIndex_ = bufferLength - 1;
    
    isProcessDone_ = isProcessedBufferPrepared_ = false;
}

void Melissa::setPlayingPosMSec(float playingPosMSec)
{
    float playingPosIndex = static_cast<size_t>(playingPosMSec / 1000.f * originalSampleRate_);
    if (playingPosIndex < aIndex_) playingPosIndex = aIndex_;
    if (bIndex_ < playingPosIndex) playingPosIndex = bIndex_;
    
    playingIndex_ = (playingPosIndex - aIndex_) * soundTouch_->getInputOutputSampleRatio();
}

int32_t Melissa::getTotalLengthMSec() const
{
    return static_cast<float>(originalBuffer_[0].size()) / originalSampleRate_ * 1000.f;
}

int32_t Melissa::getPlayingPosMSec() const
{
    const float aPosSec = static_cast<float>(aIndex_) / originalSampleRate_;
    return (aPosSec + static_cast<float>(playingIndex_) / outputSampleRate_ * speed_) * 1000;
}

void Melissa::setSpeed(float speed)
{
    if (speed_ == speed) return;
    
    speed_ = speed;
    isProcessDone_ = isProcessedBufferPrepared_ = false;
}

void Melissa::setPitch(int32_t semitone)
{
    if (semitone_ == semitone) return;
    
    semitone_ = semitone;
    isProcessDone_ = isProcessedBufferPrepared_ = false;
}


void Melissa::render(float* bufferToRender[], size_t bufferLength)
{
    if (!isOriginalBufferPrepared_ || !isProcessedBufferPrepared_) return;
    if (!mutex_.try_lock()) return;
    
    for (int iSample = 0; iSample < bufferLength; ++iSample)
    {
        if (++playingIndex_ > processedBuffer_[0].size()) playingIndex_ = 0; // loop
        for (int iCh = 0; iCh < 2; ++iCh)
        {
            bufferToRender[iCh][iSample] = processedBuffer_[iCh][playingIndex_];
        }
    }
    
    mutex_.unlock();
}

void Melissa::process()
{
    if (isProcessDone_ || !isOriginalBufferPrepared_) return;
    if (!isProcessedBufferPrepared_) prepareProcessedBuffer_();
    
    const bool needToRead = (readIndex_ < bIndex_);
    if (needToRead)
    {
        int32_t putSampleLength = 0;
        for (int iSample = 0; iSample < processLength_; ++iSample)
        {
            if (bIndex_ <= readIndex_) break;
            
            bufferForSoundTouch_[2 * iSample + 0] = originalBuffer_[0][readIndex_];
            bufferForSoundTouch_[2 * iSample + 1] = originalBuffer_[1][readIndex_];
            ++readIndex_;
            ++putSampleLength;
        }
        soundTouch_->putSamples(bufferForSoundTouch_, putSampleLength);
    }
    
    auto receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    totalReceivedSampleSize_ += receivedSampleSize;
    
    if (receivedSampleSize == 0 && !needToRead)
    {
        soundTouch_->flush();
        receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
        totalReceivedSampleSize_ += receivedSampleSize;
        isProcessDone_ = true;
    }
    
    for (int iSample = 0; iSample < receivedSampleSize; ++iSample)
    {
        processedBuffer_[0][writeIndex_] = bufferForSoundTouch_[2 * iSample + 0] / 10.f;
        processedBuffer_[1][writeIndex_] = bufferForSoundTouch_[2 * iSample + 1] / 10.f;
        ++writeIndex_;
        progress_ = static_cast<float>(writeIndex_) / processedBuffer_[0].size();
    }
    
    if (isProcessDone_)
    {
        progress_ = 1.f;
        soundTouch_->clear();
    }
}

bool Melissa::needToProcess()
{
    return !isProcessDone_;
}

float Melissa::getProgress() const
{
    return progress_;
}

void Melissa::reset()
{
    mutex_.lock();
    
    isOriginalBufferPrepared_  = false;
    isProcessedBufferPrepared_ = false;
    
    playingIndex_ = 0;
    progress_ = 0.f;
    isProcessDone_ = false;
    totalReceivedSampleSize_ = 0;
    
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        originalBuffer_[iCh].clear();
        processedBuffer_[iCh].clear();
    }
    
    mutex_.unlock();
}

std::string Melissa::getStatusString() const
{
    if (!isOriginalBufferPrepared_ || !isProcessedBufferPrepared_ || processedBuffer_[0].size() == 0) return "";
    
    std::stringstream ss;
    
    constexpr size_t numOfBlocks = 40;
    
    // read
    {
        size_t aPos = static_cast<float>(aIndex_) / originalBuffer_[0].size() * numOfBlocks;
        for (size_t iBlock = 0; iBlock < aPos; ++iBlock) ss << " ";
        ss << "A";
        
        size_t bPos = static_cast<float>(bIndex_ - aIndex_) / originalBuffer_[0].size() * numOfBlocks;
        for (size_t iBlock = 0; iBlock < bPos; ++iBlock) ss << " ";
        ss << "B" << std::endl;
        
        for (size_t iBlock = 0; iBlock < numOfBlocks; ++iBlock) ss << "#";
        ss << std::endl;
        
        size_t readPos = static_cast<float>(readIndex_) / originalBuffer_[0].size() * numOfBlocks;
        for (size_t iBlock = 0; iBlock < readPos; ++iBlock) ss << " ";
        ss << "r";
    }
    
    ss << std::endl << std::endl;
    
    // write
    {
        size_t writePos = static_cast<float>(writeIndex_) / processedBuffer_[0].size() * numOfBlocks;
        for (size_t iBlock = 0; iBlock < writePos; ++iBlock) ss << " ";
       ss << "w" << std::endl;
        
        for (size_t iBlock = 0; iBlock < numOfBlocks; ++iBlock) ss << "#";
        ss << std::endl;
        
        size_t playPos = static_cast<float>(playingIndex_) / processedBuffer_[0].size() * numOfBlocks;
        for (size_t iBlock = 0; iBlock < playPos; ++iBlock) ss << " ";
        ss << ">";
    }
    
    return ss.str();
}


void Melissa::prepareProcessedBuffer_()
{
    std::cout << "prepareProcessedBuffer()" << std::endl;
    
    mutex_.lock();
    
    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
    
    isProcessedBufferPrepared_ = true;
    readIndex_ = aIndex_;
    writeIndex_ = 0;
    
    soundTouch_->clear();
    soundTouch_->setChannels(2);
    soundTouch_->setSampleRate(originalSampleRate_);
    soundTouch_->setTempo(fsConvPitch * speed_);
    soundTouch_->setPitch(fsConvPitch * exp(0.69314718056 * semitone_ / 12.f));
    
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        processedBuffer_[iCh].resize((bIndex_ - aIndex_ + 1) * soundTouch_->getInputOutputSampleRatio());
    }
    
    mutex_.unlock();
}
