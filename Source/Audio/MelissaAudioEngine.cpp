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
    
    float getNextSampleIndex()
    {
        if (readIndex_ < que_.size())
        {
            return que_[static_cast<size_t>(readIndex_)];
        }
        else
        {
            return 0.f;
        }
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
        
        const int numOfSamplesToDelete = static_cast<int>(readIndex_) - 1;
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


MelissaEqualizer::MelissaEqualizer() :
shouldUpdateCoefs_(true)
{
    sampleRate_ = 48000;
    freq_ = 500;
    q_ = 7.f;
    gainDb_ = 0.f;
    reset();
}

void MelissaEqualizer::reset()
{
    for (size_t chIndex = 0; chIndex < kNumOfChs; ++chIndex)
    {
        std::fill(z_[chIndex], z_[chIndex] + kNumOfZs_, 0.f);
    }
}

void MelissaEqualizer::updateCoefs()
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

void MelissaEqualizer::process(float* inBuffer, float* outBuffer)
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

void MelissaEqualizer::setSampleRate(float sampleRate)
{
    sampleRate_ = sampleRate;
    shouldUpdateCoefs_ = true;
}

void MelissaEqualizer::setFreq(float freq)
{
    freq_ = freq;
    shouldUpdateCoefs_ = true;
}

void MelissaEqualizer::setGain(float gain)
{
    gainDb_ = gain;
    shouldUpdateCoefs_ = true;
}

void MelissaEqualizer::setQ(float q)
{
    q_ = q;
    shouldUpdateCoefs_ = true;
}

MelissaAudioEngine::MelissaAudioEngine() :
model_(MelissaModel::getInstance()), dataSource_(MelissaDataSource::getInstance()), soundTouch_(make_unique<soundtouch::SoundTouch>()), playbackStatus_(kPlaybackStatus_Stop), playbackMode_(kPlaybackMode_LoopOneSong), originalSampleRate_(48000), originalBufferLength_(0), outputSampleRate_(48000),
aIndex_(0), bIndex_(0), processStartIndex_(0), readIndex_(0), playingPosMSec_(0.f), trimStartIndex_(0), trimEndIndex_(0), speed_(100), processingSpeed_(1.f), semitone_(0), volume_(1.f), needToReset_(true), loop_(true), shouldProcess_(true),
#if defined(ENABLE_SPEED_TRAINING)
count_(0), speedMode_(kSpeedMode_Basic), speedIncStart_(100), speedIncPer_(10), speedIncValue_(1), speedIncGoal_(100),
#endif
currentProcessingPlaybackSpeed_(100), volumeBalance_(0.5f), eqSwitch_(false), playPart_(kPlayPart_All), enableCountIn_(false), previousRenderedPosMSec_(0.f), countInSampleIndex_(0), forcePreCountOn_(false)
{
    sampleIndexStretcher_ = std::make_unique<SampleIndexStretcher>();
    speedStretcher_ = std::make_unique<SampleIndexStretcher>();
    eq_ = std::make_unique<MelissaEqualizer>();
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
    beepGen_.setOutputSampleRate(sampleRate);
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
    if (processedBufferQue_.size() <= bufferLength || 
        !sampleIndexStretcher_->isStretchedSampleIndicesPrepared(bufferLength) ||
        !speedStretcher_->isStretchedSampleIndicesPrepared(bufferLength))
    {
        if (!shouldProcess_) status_ = kStatus_RequestingForNextSong;
        mutex_.unlock();
        return;
    }
    mutex_.unlock();
    
    for (int sampleIndex = 0; sampleIndex < bufferLength; ++sampleIndex)
    {
        if (enableCountIn_ && 0 < countInSampleIndex_)
        {
            // Pre-count metronome
            if (countInSampleIndex_ % static_cast<int>(60.f / model_->getBpm() * outputSampleRate_) == 0) beepGen_.trigger(880);
            
            bufferToRender[0][sampleIndex] = beepGen_.render();
            if (1 < numOfChannels) bufferToRender[1][sampleIndex] = bufferToRender[0][sampleIndex];
            countInSampleIndex_--;
            if (countInSampleIndex_ == 0)
            {
                previousRenderedPosMSec_ = static_cast<float>(sampleIndexStretcher_->getNextSampleIndex()) / originalSampleRate_ * 1000.f;
            }
        }
        else
        {
            mutex_.lock();
            const float currentPlaybackPosMSec = static_cast<float>(sampleIndexStretcher_->getNextSampleIndex()) / originalSampleRate_ * 1000.f;
            currentPlaybackSpeed_ = static_cast<int32_t>(speedStretcher_->getNextSampleIndex());
            mutex_.unlock();
            const bool looped = static_cast<int>(currentPlaybackPosMSec)   == static_cast<int>(model_->getLoopAPosMSec()) && static_cast<int>(previousRenderedPosMSec_) == static_cast<int>(model_->getLoopBPosMSec());
            if (enableCountIn_ && (looped || forcePreCountOn_))
            {
                const auto bpm = model_->getBpm();
                const auto accent = model_->getAccent();
                countInSampleIndex_ = static_cast<size_t>(60.f / bpm * accent * outputSampleRate_);
                forcePreCountOn_ = false;
            }
            else
            {
                mutex_.lock();
                if (processedBufferQue_.size() > 0)
                {
                    previousRenderedPosMSec_ = timeIndicesMSec[sampleIndex] = playingPosMSec_ = currentPlaybackPosMSec;

                    sampleIndexStretcher_->getStretchedSampleIndices(1, timeQue_);
                    speedStretcher_->getStretchedSampleIndices(1, speedQue_);
                    
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
                    else if (outputMode_ == kOutputMode_Mid)
                    {
                        const auto mid = (buffer[0] + buffer[1]) / 2.f;
                        buffer[0] = buffer[1] = mid;
                    }
                    
                    if (numOfChannels == 1)
                    {
                        // mono
                        bufferToRender[0][sampleIndex] = (buffer[0] + buffer[1]) * volumeBalance_;
                    }
                    else
                    {
                        // stereo
                        bufferToRender[0][sampleIndex] = buffer[0] * volumeBalance_;
                        bufferToRender[1][sampleIndex] = buffer[1] * volumeBalance_;
                    }

                    processedBufferQue_.erase(processedBufferQue_.begin(), processedBufferQue_.begin() + 2);
                }
                mutex_.unlock();
            }
        }
    }
    
    model_->updatePlayingPosMSecFromDsp(playingPosMSec_);
}

void MelissaAudioEngine::process()
{
    if (dataSource_->getBufferLength() == 0 || sampleIndexStretcher_ == nullptr || speedStretcher_ == nullptr) return;
    if (needToReset_) resetProcessedBuffer();
    
    uint32_t receivedSampleSize = soundTouch_->receiveSamples(bufferForSoundTouch_, processLength_);
    while (receivedSampleSize == 0)
    {
        int numSamplesToRead = 0;
        
        const size_t readStartIndex = readIndex_;
        for (size_t iSample = 0; iSample < processLength_; ++iSample)
        {
            if (readIndex_ > bIndex_)
            {
                if (loop_)
                {
#if defined(ENABLE_SPEED_TRAINING)
                    ++count_;
                    
                    if (speedMode_ == kSpeedMode_Training && speedIncPer_ != 0)
                    {
                        const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
                        currentProcessingPlaybackSpeed_ = speed_ + (count_ / speedIncPer_) * speedIncValue_;
                        if (currentProcessingPlaybackSpeed_ > speedIncGoal_) currentProcessingPlaybackSpeed_ = speedIncGoal_;
                        soundTouch_->setTempo(fsConvPitch * currentProcessingPlaybackSpeed_ / 100.f);
                    }
#endif
                    readIndex_ = aIndex_;
                    break;
                }
                else
                {
                    shouldProcess_ = false;
                }
            }
            mutex_.lock();
            sampleIndexStretcher_->putSampleIndex(readIndex_);
            speedStretcher_->putSampleIndex(currentProcessingPlaybackSpeed_);
            mutex_.unlock();
            ++numSamplesToRead;
            ++readIndex_;
        }
        
        float lCh[processLength_];
        float rCh[processLength_];
        float* readAudio[] = { lCh, rCh };
        if (shouldProcess_) dataSource_->readBuffer(MelissaDataSource::kReader_Playback, readStartIndex, numSamplesToRead, playPart_, readAudio);
        
        for (int sampleIndex = 0; sampleIndex < numSamplesToRead; ++sampleIndex)
        {
            bufferForSoundTouch_[sampleIndex * 2 + 0] = lCh[sampleIndex];
            bufferForSoundTouch_[sampleIndex * 2 + 1] = rCh[sampleIndex];
        }
        
        soundTouch_->putSamples(bufferForSoundTouch_, numSamplesToRead);
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
    soundTouch_->setTempo(fsConvPitch * currentProcessingPlaybackSpeed_ / 100.f);
    soundTouch_->setPitch(fsConvPitch * exp(0.69314718056 * semitone_ / 12.f));
    processingSpeed_ = static_cast<float>(originalSampleRate_) / outputSampleRate_ * (currentProcessingPlaybackSpeed_ / 100.f);
    sampleIndexStretcher_->setSpeed(processingSpeed_);
    speedStretcher_->setSpeed(processingSpeed_);

    processedBufferQue_.clear();
    timeQue_.clear();
    speedQue_.clear();
    
    playingPosMSec_ = static_cast<float>(processStartIndex_) / originalSampleRate_ * 1000.f;
    if (processStartIndex_ < aIndex_ || bIndex_ < processStartIndex_) processStartIndex_ = aIndex_;
    readIndex_ = processStartIndex_;
    needToReset_ = false;
    shouldProcess_ = true;
    currentPlaybackSpeed_ = speed_;

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

void MelissaAudioEngine::setTrimMode(bool shouldTrim)
{
    shouldTrim_ = shouldTrim;
    //if (shouldTrim_) doTrim();
}

void MelissaAudioEngine::resetSpeedTraining()
{
    count_ = 0;
    currentProcessingPlaybackSpeed_ = speedIncStart_;
    needToReset_ = true;
}

void MelissaAudioEngine::playbackStatusChanged(PlaybackStatus status)
{
    playbackStatus_ = status;
    setupPreCountIfNeeded();
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
    if (speedMode_ == kSpeedMode_Basic) currentProcessingPlaybackSpeed_ = speed_;
#else
    currentProcessingPlaybackSpeed_ = speed_;
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
        currentProcessingPlaybackSpeed_  = speed_;
        speedIncValue_ = 1;
        speedIncPer_   = 10;
        speedIncGoal_  = speed_;
    }
    else
    {
        speed_         = model_->getSpeedIncStart();
        currentProcessingPlaybackSpeed_  = model_->getSpeedIncStart();
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
    if (speedMode_ == kSpeedMode_Training) currentProcessingPlaybackSpeed_  = speedIncStart_;
    count_ = 0;
}

void MelissaAudioEngine::speedIncValueChanged(int speedIncValue)
{
    speedIncValue_ = speedIncValue;
    if (speedMode_ == kSpeedMode_Training) currentProcessingPlaybackSpeed_  = speedIncStart_;
    count_ = 0;
}

void MelissaAudioEngine::speedIncPerChanged(int speedIncPer)
{
    speedIncPer_ = speedIncPer;
    if (speedMode_ == kSpeedMode_Training) currentProcessingPlaybackSpeed_  = speedIncStart_;
    count_ = 0;
}

void MelissaAudioEngine::speedIncGoalChanged(int speedIncGoal)
{
    speedIncGoal_ = speedIncGoal;
    if (speedMode_ == kSpeedMode_Training) currentProcessingPlaybackSpeed_ = speedIncStart_;
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
    if (processStartIndex_ < aIndex_)
    {
        setupPreCountIfNeeded();
        processStartIndex_ = aIndex_;
    }
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

void MelissaAudioEngine::playPartChanged(PlayPart playPart)
{
    playPart_ = playPart;
}

void MelissaAudioEngine::preCountSwitchChanged(bool preCountSwitch)
{
#if defined(ENABLE_PRECOUNT)
    enableCountIn_ = preCountSwitch;
#endif
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

void MelissaAudioEngine::setupPreCountIfNeeded()
{
    if (enableCountIn_ && playbackStatus_ == kPlaybackStatus_Playing)
    {
#if defined(ENABLE_PRECOUNT)
        forcePreCountOn_ = true;
#endif
        processStartIndex_ = aIndex_;
        needToReset_ = true;
    }
}
