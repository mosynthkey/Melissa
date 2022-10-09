//
//  MelissaAudioEngine.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <cmath>
#include <iostream>
#include <limits.h>
#include <sstream>
#include "MelissaAudioEngine.h"
#include "MelissaDataSource.h"
#include "MelissaModel.h"
#include "MelissaUtility.h"

using std::make_unique;

class MelissaAudioEngine::SampleIndexStretcher
{
public:
    SampleIndexStretcher() :
    readIndex_(0.f),
    speed_(1.f)
    {
    }
    
    void reset()
    {
        std::lock_guard<std::mutex> lock(queMutex_);
        readIndex_ = 0;
        que_.clear();
    }
    
    void setSpeed(float speed)
    {
        std::lock_guard<std::mutex> lock(queMutex_);
        speed_ = speed;
        readIndex_ = 0;
        que_.clear();
    }
    
    void putSampleIndex(size_t sampleIndex)
    {
        std::lock_guard<std::mutex> lock(queMutex_);
        que_.push_back(sampleIndex);
    }
    
    bool isStretchedSampleIndicesPrepared(size_t length) //const
    {
        assert(0.f <= readIndex_);
        prevReadIndex_ = readIndex_;
        prevSpeed_ = speed_;
        prevLength_ = length;
        prevQueSize_ = que_.size();
        return (readIndex_ + length * speed_) < que_.size();
    }
    
    void getStretchedSampleIndices(size_t length, std::deque<float>& stretchedSampleIndex)
    {
        std::lock_guard<std::mutex> lock(queMutex_);

        stretchedSampleIndex.clear();
        
        for (size_t index = 0; index < length; ++index)
        {
            stretchedSampleIndex.push_back(que_[static_cast<size_t>(readIndex_)]);
            readIndex_ += speed_;
        }
        
        const size_t numOfSamplesToDelete = static_cast<int>(readIndex_) - 1;
        if (0 < numOfSamplesToDelete)
        {
            que_.erase(que_.begin(), que_.begin() + numOfSamplesToDelete);
            readIndex_ -= numOfSamplesToDelete;
        }
    }
    
private:
    std::mutex queMutex_;
    std::deque<size_t> que_;
    float readIndex_;
    float speed_;

    float prevReadIndex_;
    float prevSpeed_;
    size_t prevLength_;
    size_t prevQueSize_;
};

class MelissaAudioEngine::Equalizer
{
public:
    Equalizer() :
    shouldUpdateCoefs_(true)
    {
        sampleRate_ = 48000;
        freq_ = 500;
        q_ = 7.f;
        gainDb_ = 0.f;
        reset();
    }
    
    void reset()
    {
        for (size_t chIndex = 0; chIndex < kNumOfChs; ++chIndex)
        {
            std::fill(z_[chIndex], z_[chIndex] + kNumOfZs_, 0.f);
        }
    }
    
    void updateCoefs()
    {
        const float gain = pow(10, abs(gainDb_) / 20.f) * ((gainDb_ > 0) ? 1 : -1);
        const float omega = 2.f * M_PI *  freq_ / sampleRate_;
        const float alpha = sin(omega) * sinh(log(2.f) / 2.f * q_ * omega / sin(omega));
        const float A     = pow(10.f, (gain / 40.f) );
         
        a[0] =  1.0f + alpha / A;
        a[1] = -2.0f * cos(omega);
        a[2] =  1.0f - alpha / A ;
        b[0] =  1.0f + alpha * A ;
        b[1] = -2.0f * cos(omega);
        b[2] =  1.0f - alpha * A ;
        
        shouldUpdateCoefs_ = false;
    }
    
    void process(float* inBuffer, float* outBuffer)
    {
        if (shouldUpdateCoefs_) updateCoefs();
        
        for (size_t chIndex = 0; chIndex < 2; ++chIndex)
        {
            const float signalIn = inBuffer[chIndex];
            outBuffer[chIndex] = (b[0] / a[0]) * signalIn
                               + (b[1] / a[0]) * z_[chIndex][0]
                               + (b[2] / a[0]) * z_[chIndex][1]
                               - (a[1] / a[0]) * z_[chIndex][2]
                               - (a[2] / a[0]) * z_[chIndex][3];
            
            z_[chIndex][1] = z_[chIndex][0];
            z_[chIndex][0] = signalIn;
            z_[chIndex][3] = z_[chIndex][2];
            z_[chIndex][2] = outBuffer[chIndex];
        }
    }
    
    void setSampleRate(float sampleRate)
    {
        sampleRate_ = sampleRate;
        shouldUpdateCoefs_ = true;
    }
    
    void setFreq(float freq)
    {
        freq_ = freq;
        shouldUpdateCoefs_ = true;
    }
    
    void setGain(float gain)
    {
        gainDb_ = gain;
        shouldUpdateCoefs_ = true;
    }
    
    void setQ(float q)
    {
        q_ = q;
        shouldUpdateCoefs_ = true;
    }
    
private:
    float a[3], b[3];
    static constexpr size_t kNumOfZs_ = 4;
    static constexpr size_t kNumOfChs = 2;
    float z_[kNumOfChs][kNumOfZs_];
    float shouldUpdateCoefs_;
    float sampleRate_, freq_, gainDb_, q_;
};

MelissaAudioEngine::MelissaAudioEngine() :
model_(MelissaModel::getInstance()), dataSource_(MelissaDataSource::getInstance()), soundTouch_(make_unique<soundtouch::SoundTouch>()), playbackMode_(kPlaybackMode_LoopOneSong), originalSampleRate_(48000), originalBufferLength_(0), outputSampleRate_(48000),
aIndex_(0), bIndex_(0), processStartIndex_(0), readIndex_(0), playingPosMSec_(0.f), speed_(100), processingSpeed_(1.f), semitone_(0), volume_(1.f), needToReset_(true), loop_(true), shouldProcess_(true),
#if defined(ENABLE_SPEED_TRAINING)
count_(0), speedMode_(kSpeedMode_Basic), speedIncStart_(100), speedIncPer_(10), speedIncValue_(1), speedIncGoal_(100),
#endif
currentSpeed_(100), volumeBalance_(0.5f), eqSwitch_(false)
{
    sampleIndexStretcher_ = std::make_unique<SampleIndexStretcher>();
    eq_ = std::make_unique<Equalizer>();
}

MelissaAudioEngine::~MelissaAudioEngine() {}

void MelissaAudioEngine::updateBuffer()
{
    reset();
    
    originalSampleRate_ = dataSource_->getSampleRate();
    originalBufferLength_ = dataSource_->getBufferLength();
    
    processStartIndex_ = 0;
    
    model_->setLoopPosRatio(0.f, 1.f);
    model_->setPitch(0);
    model_->setSpeed(100);
   
    needToReset_ = true;
}

void MelissaAudioEngine::setOutputSampleRate(int32_t sampleRate)
{
    if (outputSampleRate_ == sampleRate) return;
    
    outputSampleRate_ = sampleRate;
    eq_->setSampleRate(sampleRate);
    needToReset_ = true;
}

float MelissaAudioEngine::getPlayingPosMSec() const
{
    return playingPosMSec_;
}

float MelissaAudioEngine::getPlayingPosRatio() const
{
    return static_cast<float>(getPlayingPosMSec()) / 1000.f / (static_cast<float>(originalBufferLength_) / originalSampleRate_);
}

void MelissaAudioEngine::render(float* bufferToRender[], size_t numOfChannels, std::vector<float>& timeIndicesMSec, size_t bufferLength)
{
    if (status_ != kStatus_Playing) return;
    jassert(1 <= numOfChannels && numOfChannels <= 2);
    
    mutex_.lock();
    if (processedBufferQue_.size() <= bufferLength || !sampleIndexStretcher_->isStretchedSampleIndicesPrepared(bufferLength))
    {
        if (!shouldProcess_) status_ = kStatus_RequestingForNextSong;
        mutex_.unlock();
        return;
    }
    sampleIndexStretcher_->getStretchedSampleIndices(bufferLength, timeQue_);
    mutex_.unlock();
    
    for (int iSample = 0; iSample < bufferLength; ++iSample)
    {
        mutex_.lock();
        if (processedBufferQue_.size() > 0 && timeQue_.size() > 0)
        {
            float buffer[] = { processedBufferQue_[0], processedBufferQue_[1] };
            
            if (eqSwitch_) eq_->process(buffer, buffer);
            buffer[0] *= volume_;
            buffer[1] *= volume_;
            
            if (outputMode_ == kOutputMode_LL)
            {
                buffer[1] = buffer[0];
            }
            else if (outputMode_ == kOutputMode_RR)
            {
                buffer[0] = buffer[1];
            }
            else if (outputMode_ == kOutputMode_CenterCancel)
            {
                const auto lrDiff = buffer[0] - buffer[1];
                buffer[0] = buffer[1] = lrDiff;
            }
            
            if (numOfChannels == 1)
            {
                // mono
                bufferToRender[0][iSample] = (buffer[0] + buffer[1]) * volumeBalance_;
            }
            else
            {
                // stereo
                bufferToRender[0][iSample] = buffer[0] * volumeBalance_;
                bufferToRender[1][iSample] = buffer[1] * volumeBalance_;
            }

            processedBufferQue_.erase(processedBufferQue_.begin(), processedBufferQue_.begin() + 2);
            
            timeIndicesMSec[iSample] = playingPosMSec_ = static_cast<float>(timeQue_[0]) / originalSampleRate_ * 1000.f;
            timeQue_.pop_front();
        }
        mutex_.unlock();
    }
    
    model_->updatePlayingPosMSecFromDsp(playingPosMSec_);
}

void MelissaAudioEngine::process()
{
    if (dataSource_->getBufferLength() == 0 || sampleIndexStretcher_ == nullptr) return;
    if (needToReset_) resetProcessedBuffer();
    
    uint32_t receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    while (receivedSampleSize == 0)
    {
        for (size_t iSample = 0; iSample < processLength_; ++iSample)
        {
            if (readIndex_ > bIndex_)
            {
                if (loop_)
                {
                    readIndex_ = aIndex_;
#if defined(ENABLE_SPEED_TRAINING)
                    ++count_;
                    
                    if (speedMode_ == kSpeedMode_Training && speedIncPer_ != 0)
                    {
                        const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
                        currentSpeed_ = speed_ + (count_ / speedIncPer_) * speedIncValue_;
                        if (currentSpeed_ > speedIncGoal_) currentSpeed_ = speedIncGoal_;
                        soundTouch_->setTempo(fsConvPitch * currentSpeed_ / 100.f);
                    }
#endif
                }
                else
                {
                    shouldProcess_ = false;
                }
            }
            mutex_.lock();
            sampleIndexStretcher_->putSampleIndex(readIndex_);
            mutex_.unlock();
            bufferForSoundTouch_[iSample * 2 + 0] = shouldProcess_ ? dataSource_->readBuffer(0, readIndex_, playPart_) : 0.f;
            bufferForSoundTouch_[iSample * 2 + 1] = shouldProcess_ ? dataSource_->readBuffer(1, readIndex_, playPart_) : 0.f;
            ++readIndex_;
        }
        
        soundTouch_->putSamples(bufferForSoundTouch_, processLength_);
        receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    }
    
    mutex_.lock();
    for (size_t iSample = 0; iSample < receivedSampleSize * 2; ++iSample)
    {
        processedBufferQue_.emplace_back(bufferForSoundTouch_[iSample]);
    }
    mutex_.unlock();
}

bool MelissaAudioEngine::needToProcess() const
{
    return needToReset_ || (shouldProcess_ && (processedBufferQue_.size() < queLength_));
}

bool MelissaAudioEngine::isBufferSet() const
{
    return originalBufferLength_ != 0;
}

void MelissaAudioEngine::reset()
{
    needToReset_ = true;
}

void MelissaAudioEngine::resetProcessedBuffer()
{
    mutex_.lock();

    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
    
    soundTouch_->clear();
    soundTouch_->setChannels(2);
    soundTouch_->setSampleRate(originalSampleRate_);
    soundTouch_->setTempo(fsConvPitch * currentSpeed_ / 100.f);
    soundTouch_->setPitch(fsConvPitch * exp(0.69314718056 * semitone_ / 12.f));
    processingSpeed_ = static_cast<float>(originalSampleRate_) / outputSampleRate_ * (currentSpeed_ / 100.f);
    sampleIndexStretcher_->setSpeed(processingSpeed_);
    
    processedBufferQue_.clear();
    timeQue_.clear();
    
    playingPosMSec_ = static_cast<float>(processStartIndex_) / originalSampleRate_ * 1000.f;
    if (processStartIndex_ < aIndex_ || bIndex_ < processStartIndex_) processStartIndex_ = aIndex_;
    readIndex_ = processStartIndex_;
    needToReset_ = false;
    shouldProcess_ = true;
    
    if (playbackMode_ == kPlaybackMode_LoopOneSong)
    {
        loop_ = true;
    }
    else if (playbackMode_ == kPlaybackMode_LoopPlaylistSongs)
    {
        const bool loopRangeWholeSong = (aIndex_ == 0 && bIndex_ == originalBufferLength_);
        loop_ = !loopRangeWholeSong;
    }
    status_ = kStatus_Playing;
    
#if defined(ENABLE_SPEED_TRAINING)
    count_ = 0;
#endif

    mutex_.unlock();
}

MelissaAudioEngine::Status MelissaAudioEngine::getStatus() const
{
    return status_;
}

void MelissaAudioEngine::setStatus(Status status)
{
    status_ = status;
}

std::string MelissaAudioEngine::getStatusString() const
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

void MelissaAudioEngine::playbackModeChanged(PlaybackMode mode)
{
    if (playbackMode_ == mode) return;
    playbackMode_ = mode;
    
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

void MelissaAudioEngine::musicVolumeChanged(float volume)
{
    volume_ = volume;
}

void MelissaAudioEngine::pitchChanged(float semitone)
{
    semitone_ = semitone;
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

void MelissaAudioEngine::speedChanged(int speed)
{
    speed_ = speed;
#if defined(ENABLE_SPEED_TRAINING)
    if (speedMode_ == kSpeedMode_Basic) currentSpeed_ = speed_;
#else
    currentSpeed_ = speed_;
#endif
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

#if defined(ENABLE_SPEED_TRAINING)
void MelissaAudioEngine::speedModeChanged(SpeedMode mode)
{
    if (mode == kSpeedMode_Basic)
    {
        speed_ = model_->getSpeed();
        currentSpeed_  = speed_;
        speedIncValue_ = 1;
        speedIncPer_   = 10;
        speedIncGoal_  = speed_;
    }
    else
    {
        speed_         = model_->getSpeedIncStart();
        currentSpeed_  = model_->getSpeedIncStart();
        speedIncValue_ = model_->getSpeedIncValue();
        speedIncPer_   = model_->getSpeedIncPer();
        speedIncGoal_  = model_->getSpeedIncGoal();
    }
    
    speedMode_ = mode;
    count_ = 0;
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

void MelissaAudioEngine::speedIncStartChanged(int speedIncStart)
{
    speedIncStart_ = speedIncStart;
    if (speedMode_ == kSpeedMode_Training) currentSpeed_  = speedIncStart_;
    count_ = 0;
}

void MelissaAudioEngine::speedIncValueChanged(int speedIncValue)
{
    speedIncValue_ = speedIncValue;
    if (speedMode_ == kSpeedMode_Training) currentSpeed_  = speedIncStart_;
    count_ = 0;
}

void MelissaAudioEngine::speedIncPerChanged(int speedIncPer)
{
    speedIncPer_ = speedIncPer;
    if (speedMode_ == kSpeedMode_Training) currentSpeed_  = speedIncStart_;
    count_ = 0;
}

void MelissaAudioEngine::speedIncGoalChanged(int speedIncGoal)
{
    speedIncGoal_ = speedIncGoal;
    if (speedMode_ == kSpeedMode_Training) currentSpeed_ = speedIncStart_;
    count_ = 0;
}
#endif

void MelissaAudioEngine::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    if (!(0 <= aRatio && aRatio < bRatio && bRatio <= 1.f)) return;
    
    aIndex_ = static_cast<int32_t>(aRatio * originalBufferLength_);
    bIndex_ = static_cast<int32_t>(bRatio * originalBufferLength_);
    if (readIndex_ < aIndex_ || bIndex_ < readIndex_)
    {
        readIndex_ = aIndex_;
    }
    updateLoopParameters();
}

void MelissaAudioEngine::playingPosChanged(float time, float ratio)
{
    processStartIndex_ = static_cast<size_t>((originalBufferLength_ - 1) * ratio);
    if (processStartIndex_ < aIndex_) processStartIndex_ = aIndex_;
    if (bIndex_ < processStartIndex_) processStartIndex_ = bIndex_;
    needToReset_ = true;
}

void MelissaAudioEngine::musicMetronomeBalanceChanged(float balance)
{
    if (balance < 0.5f)
    {
        volumeBalance_ = 1.f;
    }
    else
    {
        volumeBalance_ = cos(M_PI / 2.f * (balance - 0.5f) * 2.f);
    }
}

void MelissaAudioEngine::outputModeChanged(OutputMode outputMode)
{
    outputMode_ = outputMode;
}

void MelissaAudioEngine::eqSwitchChanged(bool on)
{
    eqSwitch_ = on;
}

void MelissaAudioEngine::eqFreqChanged(size_t band, float freq)
{
    eq_->setFreq(freq);
}

void MelissaAudioEngine::eqGainChanged(size_t band, float gain)
{
    eq_->setGain(gain);
}

void MelissaAudioEngine::eqQChanged(size_t band, float q)
{
    eq_->setQ(q);
}

void MelissaAudioEngine::updateLoopParameters()
{
    if (playbackMode_ == kPlaybackMode_LoopOneSong)
    {
        loop_ = true;
    }
    else if (playbackMode_ == kPlaybackMode_LoopPlaylistSongs)
    {
        const bool loopRangeWholeSong = (aIndex_ == 0 && bIndex_ == originalBufferLength_);
        loop_ = !loopRangeWholeSong;
    }
}

void MelissaAudioEngine::playPartChanged(StemType playPart)
{
    playPart_ = playPart;
}
