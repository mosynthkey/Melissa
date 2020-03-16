//
//  Melissa.cpp
//  Melissa
//
//  Created by Masaki on 2019/07/29.
//

#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <limits.h>
#include <sstream>
#include "Melissa.h"
#include "MelissaUtility.h"

using std::make_unique;

Melissa::Melissa() :
soundTouch_(make_unique<soundtouch::SoundTouch>()), isOriginalBufferPrepared_(false), originalSampleRate_(-1), originalBufferLength_(0), outputSampleRate_(-1),
aIndex_(0), bIndex_(0), processStartIndex_(0), aNextIndex_(-1), bNextIndex_(-1), readIndex_(0), playingPosMSec_(0.f), speed_(100), semitone_(0), volume_(1.f), needToReset_(false), count_(0), speedIncPer_(0), speedIncValue_(0), speedIncMax_(100), currentSpeed_(100)
{
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        originalBuffer_[iCh].clear();
    }
}

void Melissa::setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate)
{
    reset();
    
    originalSampleRate_ = sampleRate;
    
    for (int iCh = 0; iCh < 2; ++iCh)
    {
        originalBuffer_[iCh].resize(bufferLength);
        for (int iSample = 0; iSample < bufferLength; ++iSample)
        {
            originalBuffer_[iCh][iSample] = buffer[iCh][iSample];
        }
    }
    
    originalBufferLength_ = bufferLength;
    
    aIndex_ = 0;
    bIndex_ =  bufferLength - 1;
    processStartIndex_ = 0;
    setPitch(0);
    setSpeed(100);
   
    isOriginalBufferPrepared_ = true;
    needToReset_ = true;
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
    if (getBPosMSec() < aPosMSec || ((originalBufferLength_ - 1) / originalSampleRate_ * 1000.f) < aPosMSec) return;
    
    aNextIndex_ = static_cast<int32_t>(aPosMSec / 1000.f * originalSampleRate_);
    if (originalBufferLength_ < aNextIndex_) aNextIndex_ = 0;

    processStartIndex_ = getPlayingPosRatio() * originalBufferLength_;
    needToReset_ = true;
}

void Melissa::setBPosMSec(float bPosMSec)
{
    if (!isOriginalBufferPrepared_) return;
    if (bPosMSec < getAPosMSec() || ((originalBufferLength_ - 1) / originalSampleRate_ * 1000.f) < bPosMSec) return;
    
    bNextIndex_ = static_cast<int32_t>(bPosMSec / 1000.f * originalSampleRate_);
    if (bNextIndex_ < readIndex_)
    {
        processStartIndex_ = aIndex_;
    }
    else
    {
        processStartIndex_ = getPlayingPosRatio() * originalBufferLength_;
    }
    
    needToReset_ = true;
}

void Melissa::setPlayingPosMSec(float playingPosMSec)
{
    processStartIndex_ = static_cast<size_t>(playingPosMSec / 1000.f * originalSampleRate_);
    if (processStartIndex_ < aIndex_) processStartIndex_ = aIndex_;
    if (bIndex_ < processStartIndex_) processStartIndex_ = bIndex_;
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

void Melissa::setABPosRatio(float aRatio, float bRatio)
{
    if (!(0 <= aRatio && aRatio < bRatio && bRatio <= 1.f)) return;
    
    aNextIndex_ = static_cast<int32_t>(aRatio * originalBufferLength_);
    bNextIndex_ = static_cast<int32_t>(bRatio * originalBufferLength_);
    if (aNextIndex_ <= readIndex_ && readIndex_ < bNextIndex_) return;
    readIndex_ = aNextIndex_;
    needToReset_ = true;
}

float Melissa::getAPosMSec() const
{
    return static_cast<float>((aNextIndex_ == -1) ? aIndex_ : aNextIndex_) / originalSampleRate_ * 1000.f;
}

float Melissa::getBPosMSec() const
{
    return static_cast<float>((bNextIndex_ == -1) ? bIndex_ : bNextIndex_) / originalSampleRate_ * 1000.f;
}

int32_t Melissa::getTotalLengthMSec() const
{
    return static_cast<float>(originalBufferLength_) / originalSampleRate_ * 1000.f;
}

float Melissa::getPlayingPosMSec() const
{
    return playingPosMSec_;
}

float Melissa::getAPosRatio() const
{
    return static_cast<float>(getAPosMSec()) / 1000.f / (static_cast<float>(originalBufferLength_) / originalSampleRate_);
}

float Melissa::getBPosRatio() const
{
    return static_cast<float>(getBPosMSec()) / 1000.f / (static_cast<float>(originalBufferLength_) / originalSampleRate_);
}

float Melissa::getPlayingPosRatio() const
{
    return static_cast<float>(getPlayingPosMSec()) / 1000.f / (static_cast<float>(originalBufferLength_) / originalSampleRate_);
}

void Melissa::setSpeed(int32_t speed)
{
    if (speed_ == speed) return;
    
    if (speed > 200) speed = 200;
    if (speed < 20) speed = 20;
    
    speed_ = speed;
    currentSpeed_ = speed;
    
    needToReset_ = true;
}

void Melissa::setSpeedIncPer(int32_t speedIncPer)
{
    if (speedIncPer < 0) speedIncPer = 0;
    if (speedIncPer > 100) speedIncPer = 100;
    speedIncPer_ = speedIncPer;
    count_ = 0;
}

void Melissa::setSpeedIncValue(int32_t speedIncValue)
{
    if (speedIncValue < 0) speedIncValue = 0;
    if (speedIncValue > 10) speedIncValue = 10;
    speedIncValue_ = speedIncValue;
    count_ = 0;
}

void Melissa::setSpeedIncMax(int32_t speedIncMax)
{
    if (speedIncMax > 200) speedIncMax = 200;
    if (speedIncMax < speed_) speed_ = speedIncMax;
    speedIncMax_ = speedIncMax;
}

void Melissa::setPitch(int32_t semitone)
{
    if (semitone_ == semitone) return;
    
    if (semitone > 24) semitone = 24;
    if (semitone < -24) semitone = -24;
    
    semitone_ = semitone;
    needToReset_ = true;
}

void Melissa::setVolume(float volume)
{
    if (volume < 0.f) volume = 0.f;
    if (2.f < volume) volume = 2.f;
    
    volume_ = volume;
}

void Melissa::render(float* bufferToRender[], size_t bufferLength)
{
    if (!isOriginalBufferPrepared_) return;
    
    bool triggerMetronome = false;

    
    for (int iSample = 0; iSample < bufferLength; ++iSample)
    {
        metronome_.osc_ += 2.f / (outputSampleRate_ / 880.f);
        if (metronome_.osc_ > 1.f) metronome_.osc_ = -1.f;
        
        const auto metronomeOsc = metronome_.osc_ * metronome_.amp_ * metronome_.volume_ * (metronome_.on_ ? 1.f : 0.f);
        mutex_.lock();
        if (processedBufferQue_.size() > 0)
        {
            bufferToRender[0][iSample] = processedBufferQue_[0] * volume_ + metronomeOsc;
            bufferToRender[1][iSample] = processedBufferQue_[1] * volume_ + metronomeOsc;
            processedBufferQue_.erase(processedBufferQue_.begin(), processedBufferQue_.begin() + 2);
            
            if (timeQue_.size() > 0)
            {
                playingPosMSec_ = static_cast<float>(timeQue_[0]) / originalSampleRate_ * 1000.f;
                timeQue_.pop_front();
            }
        }

        mutex_.unlock();
        
        if (--metronome_.count_ < 0)
        {
            metronome_.count_ += 1.f / (static_cast<float>(metronome_.bpm_) / 60.f) * originalSampleRate_ * soundTouch_->getInputOutputSampleRatio();
            metronome_.amp_ = 1.f;
        }
        
        if (metronome_.amp_ > 0.f)
        {
            metronome_.amp_ -= 1.f / static_cast<float>(outputSampleRate_) * 10;
        }
        else if (metronome_.amp_ < 0.f)
        {
            metronome_.amp_ = 0.f;
        }
    }
    
    if (triggerMetronome)
    {
        metronome_.amp_ = 1.f;
    }
    else if (metronome_.amp_ > 0.f)
    {
        metronome_.amp_ -= 1.f / static_cast<float>(outputSampleRate_) * bufferLength * 10;
    }
    
    mutex_.lock();
    updateInternalTime();
    mutex_.unlock();
}

void Melissa::process()
{
    if (!isOriginalBufferPrepared_) return;
    if (needToReset_) resetProcessedBuffer();
    
    const auto speed = static_cast<float>(originalSampleRate_) / outputSampleRate_ * (speed_ / 100);
    size_t sampleIndex[processLength_];
    
    uint32_t receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    while (receivedSampleSize == 0)
    {
        mutex_.lock();
        updateInternalTime();
        mutex_.unlock();
        
        for (size_t iSample = 0; iSample < processLength_; ++iSample)
        {
            if (readIndex_ > bIndex_)
            {
                readIndex_ = aIndex_;
                ++count_;
                
                if (speedIncPer_ != 0)
                {
                    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
                    currentSpeed_ = speed_ + (count_ / speedIncPer_) * speedIncValue_;
                    if (currentSpeed_ > speedIncMax_) currentSpeed_ = speedIncMax_;
                    soundTouch_->setTempo(fsConvPitch * currentSpeed_ / 100.f);
                }

            }
            sampleIndex[iSample] = readIndex_;
            bufferForSoundTouch_[iSample * 2 + 0] = originalBuffer_[0][readIndex_];
            bufferForSoundTouch_[iSample * 2 + 1] = originalBuffer_[1][readIndex_];
            ++readIndex_;
        }
        
        soundTouch_->putSamples(bufferForSoundTouch_, processLength_);
        mutex_.lock();
        for (size_t index = 0; index < processLength_ / speed; ++index)
        {
            timeQue_.emplace_back(sampleIndex[static_cast<size_t>(index * speed)]);
        }
        mutex_.unlock();
        
        receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    }
    
    mutex_.lock();
    for (size_t iSample = 0; iSample < receivedSampleSize * 2; ++iSample)
    {
        processedBufferQue_.emplace_back(bufferForSoundTouch_[iSample]);
    }
    mutex_.unlock();
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
    
    mutex_.lock();
    processedBufferQue_.clear();
    mutex_.unlock();
}

void Melissa::resetProcessedBuffer()
{
    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
    
    soundTouch_->clear();
    soundTouch_->setChannels(2);
    soundTouch_->setSampleRate(originalSampleRate_);
    soundTouch_->setTempo(fsConvPitch * speed_ / 100.f);
    soundTouch_->setPitch(fsConvPitch * exp(0.69314718056 * semitone_ / 12.f));
    
    mutex_.lock();
    processedBufferQue_.clear();
    timeQue_.clear();
    mutex_.unlock();
    
    playingPosMSec_ = static_cast<float>(processStartIndex_) / originalSampleRate_ * 1000.f;
    if (processStartIndex_ < aIndex_ || bIndex_ < processStartIndex_) processStartIndex_ = aIndex_;
    readIndex_ = processStartIndex_;
    needToReset_ = false;
    metronome_.count_ = metronome_.offsetSec_ * originalSampleRate_ * soundTouch_->getInputOutputSampleRatio();
    count_ = 0;
    
    mutex_.lock();
    updateInternalTime();
    mutex_.unlock();
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
    
    for (size_t iBlock = 0; iBlock < static_cast<float>(processStartIndex_) / originalBufferLength_ * numOfBlocks; ++iBlock) ss << " ";;
    ss << "S";
    
    return ss.str();
}

void Melissa::analyzeBpm()
{
    //if (!isOriginalBufferPrepared_) return;
    
    constexpr size_t frameLength = 32;
    const size_t numOfFrames = originalBufferLength_ / frameLength;
    
    std::vector<float> frameAmp(numOfFrames);
    for (size_t iFrame = 0; iFrame < numOfFrames; ++iFrame)
    {
        float amp = 0.f;
        for (size_t iSample = 0; iSample < frameLength; ++iSample)
        {
            // stereo -> monoral
            size_t sampleIndex = iFrame * frameLength + iSample;
            if (sampleIndex > originalBufferLength_) break;
            const float sig = (originalBuffer_[0][sampleIndex] + originalBuffer_[1][sampleIndex]);
            amp += sig * sig;
        }
        frameAmp[iFrame] =  sqrt(amp / frameLength);
    }
    
    std::vector<float> frameAmpDiff(numOfFrames);
    for (size_t iFrame = 0; iFrame < numOfFrames - 1; ++iFrame)
    {
        frameAmpDiff[iFrame] = frameAmp[iFrame + 1] - frameAmp[iFrame];
        if (frameAmpDiff[iFrame] < 0.f) frameAmpDiff[iFrame] = 0.f;
    }
    frameAmpDiff[numOfFrames - 1] = 0.f;
    
    auto getBpmCorrelation = [&](uint32_t bpm, float* a, float* b)
    {
        *a = 0.f;
        *b = 0.f;
        for (size_t iFrame = 0; iFrame < numOfFrames - 1; ++iFrame)
        {
            const double rad = 2 * M_PI * (bpm / 60.f) * iFrame / (originalSampleRate_ / frameLength);
            const double win = 0.5f * (1.f - cos(2.0 * M_PI * static_cast<float>(iFrame) / numOfFrames));
            *a += frameAmpDiff[iFrame] * cos(rad) * win;
            *b += frameAmpDiff[iFrame] * sin(rad) * win;
        }
        *a /= numOfFrames;
        *b /= numOfFrames;
        
        return sqrt(*a * *a + *b * *b);
    };
    
    constexpr uint32_t bpmMax = 240, bpmMin = 60;
    uint32_t estimatedBpm = 0;
    std::vector<float> correlations(bpmMax - bpmMin + 1);
    float correlationMax = 0.f, aMax = 0.f, bMax = 0.f;
    for (uint32_t iBpm = bpmMin; iBpm <= bpmMax; ++iBpm)
    {
        auto c = correlations[iBpm - bpmMin] = getBpmCorrelation(iBpm, &aMax, &bMax);
        if (c > correlationMax)
        {
            correlationMax = c;
            estimatedBpm = iBpm;
            std::cout << "estimatedBpm might be " << estimatedBpm << " (" << correlationMax << ")" << std::endl;
        }
    }
    metronome_.bpm_ = estimatedBpm;
    
    float theta = atan2(bMax, aMax);
    if (theta < 0) theta += 2.f * M_PI;
    metronome_.offsetSec_ = theta / (2 * M_PI * (estimatedBpm / 60.f));
    
    std::cout << "BPM = " << metronome_.bpm_ << std::endl;
    std::cout << metronome_.offsetSec_ << " sec" << std::endl;
}

void Melissa::updateInternalTime()
{
    if (aNextIndex_ != -1)
    {
        aIndex_ = aNextIndex_;
        aNextIndex_ = -1;
    }
    
    if (bNextIndex_ != -1)
    {
        bIndex_ = bNextIndex_;
        bNextIndex_ = -1;
    }
}
