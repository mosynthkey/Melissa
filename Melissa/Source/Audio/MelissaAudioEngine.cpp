//
//  MelissaAudioEngine.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#define _USE_MATH_DEFINES

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
        readIndex_ = 0;
        que_.clear();
    }
    
    void setSpeed(float speed)
    {
        speed_ = speed;
        reset();
    }
    
    void putSampleIndex(size_t sampleIndex)
    {
        que_.push_back(sampleIndex);
    }
    
    void getStretchedSampleIndices(size_t length, std::deque<float>& stretchedSampleIndex)
    {
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
    std::deque<size_t> que_;
    float readIndex_;
    float speed_;
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
        printf("gain = %f\n", gain);
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
model_(MelissaModel::getInstance()), dataSource_(MelissaDataSource::getInstance()), soundTouch_(make_unique<soundtouch::SoundTouch>()), originalSampleRate_(-1), originalBufferLength_(0), outputSampleRate_(-1),
aIndex_(0), bIndex_(0), processStartIndex_(0), readIndex_(0), playingPosMSec_(0.f), speed_(100), processingSpeed_(1.f), semitone_(0), volume_(1.f), needToReset_(true), count_(0), speedIncStart_(100), speedIncPer_(0), speedIncValue_(0), speedIncGoal_(100), currentSpeed_(100), volumeBalance_(0.5f), eqSwitch_(false)
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

void MelissaAudioEngine::render(float* bufferToRender[], size_t bufferLength)
{
    
    if (processedBufferQue_.size() <= bufferLength) return;
    mutex_.lock();
    sampleIndexStretcher_->getStretchedSampleIndices(bufferLength, timeQue_);
    mutex_.unlock();
    
    for (int iSample = 0; iSample < bufferLength; ++iSample)
    {
        metronome_.osc_ += 2.f / (outputSampleRate_ / metronome_.pitch_);
        if (metronome_.osc_ > 1.f) metronome_.osc_ = -1.f;
        
        const auto metronomeOsc = metronome_.osc_ * metronome_.amp_ * metronome_.volume_ * (metronome_.on_ ? 1.f : 0.f);
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
            
            const float musicVolumeCoef = cos(M_PI / 2.f * volumeBalance_);
            bufferToRender[0][iSample] = buffer[0] * musicVolumeCoef + metronomeOsc * (1.f - musicVolumeCoef);
            bufferToRender[1][iSample] = buffer[1] * musicVolumeCoef + metronomeOsc * (1.f - musicVolumeCoef);
            processedBufferQue_.erase(processedBufferQue_.begin(), processedBufferQue_.begin() + 2);
            
            playingPosMSec_ = static_cast<float>(timeQue_[0]) / originalSampleRate_ * 1000.f;
            timeQue_.pop_front();
        }
        mutex_.unlock();
        
        const int beatSection = (playingPosMSec_ - metronome_.beatPositionMSec_) / ((60.f / metronome_.bpm_) * 1000.f);
        if (metronome_.accent_ != 0 && (beatSection / metronome_.accent_ != metronome_.prevBeatSection_ / metronome_.accent_))
        {
            metronome_.amp_ = 1.f;
            metronome_.pitch_ = 880 * 2;
        }
        else if (beatSection != metronome_.prevBeatSection_)
        {
            metronome_.amp_ = 1.f;
            metronome_.pitch_ = 880;
        }
        metronome_.prevBeatSection_ = beatSection;
        
        if (metronome_.amp_ > 0.f)
        {
            metronome_.amp_ -= 1.f / static_cast<float>(outputSampleRate_) * 20.f;
        }
        else if (metronome_.amp_ < 0.f)
        {
            metronome_.amp_ = 0.f;
        }
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
                readIndex_ = aIndex_;
                ++count_;
                
                if (speedIncPer_ != 0)
                {
                    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
                    currentSpeed_ = speed_ + (count_ / speedIncPer_) * speedIncValue_;
                    if (currentSpeed_ > speedIncGoal_) currentSpeed_ = speedIncGoal_;
                    model_->setCurrentSpeed(currentSpeed_);
                    soundTouch_->setTempo(fsConvPitch * currentSpeed_ / 100.f);
                }

            }
            mutex_.lock();
            sampleIndexStretcher_->putSampleIndex(readIndex_);
            mutex_.unlock();
            bufferForSoundTouch_[iSample * 2 + 0] = dataSource_->readBuffer(0, readIndex_);
            bufferForSoundTouch_[iSample * 2 + 1] = dataSource_->readBuffer(1, readIndex_);
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
    return (processedBufferQue_.size() < queLength_);
}

bool MelissaAudioEngine::isBufferSet() const
{
    return originalBufferLength_ != 0;
}

void MelissaAudioEngine::reset()
{
    needToReset_ = true;
    
    mutex_.lock();
    soundTouch_->clear();
    processedBufferQue_.clear();
    mutex_.unlock();
}

void MelissaAudioEngine::resetProcessedBuffer()
{
    const auto fsConvPitch = static_cast<float>(originalSampleRate_) / outputSampleRate_;
    
    soundTouch_->clear();
    soundTouch_->setChannels(2);
    soundTouch_->setSampleRate(originalSampleRate_);
    soundTouch_->setTempo(fsConvPitch * speed_ / 100.f);
    soundTouch_->setPitch(fsConvPitch * exp(0.69314718056 * semitone_ / 12.f));
    processingSpeed_ = static_cast<float>(originalSampleRate_) / outputSampleRate_ * (speed_ / 100.f);
    sampleIndexStretcher_->setSpeed(processingSpeed_);
    
    mutex_.lock();
    processedBufferQue_.clear();
    timeQue_.clear();
    sampleIndexStretcher_->reset();
    mutex_.unlock();
    
    playingPosMSec_ = static_cast<float>(processStartIndex_) / originalSampleRate_ * 1000.f;
    if (processStartIndex_ < aIndex_ || bIndex_ < processStartIndex_) processStartIndex_ = aIndex_;
    readIndex_ = processStartIndex_;
    needToReset_ = false;
    count_ = 0;
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

void MelissaAudioEngine::analyzeBpm()
{
    // Ref : http://hp.vector.co.jp/authors/VA046927/tempo/tempo.html
    
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
            const float sig = (dataSource_->readBuffer(0, sampleIndex) + dataSource_->readBuffer(1, sampleIndex));
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
            std::cout << "Bpm might be " << estimatedBpm << " (" << correlationMax << ")" << std::endl;
        }
    }
    
    model_->setBpm(estimatedBpm);
    
    float theta = atan2(bMax, aMax);
    if (theta < 0) theta += 2.f * M_PI;
    const float beatPositionMSec = theta / (2 * M_PI * (estimatedBpm / 60.f));
    model_->setBeatPositionMSec(beatPositionMSec);
    
    std::cout << "BPM = " << estimatedBpm << std::endl;
    std::cout << beatPositionMSec << " sec" << std::endl;
}

void MelissaAudioEngine::musicVolumeChanged(float volume)
{
    volume_ = volume;
}

void MelissaAudioEngine::pitchChanged(int semitone)
{
    semitone_ = semitone;
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

void MelissaAudioEngine::speedModeChanged(SpeedMode mode)
{
    if (mode == kSpeedMode_Basic)
    {
        speed_ = model_->getSpeed();
        currentSpeed_  = speed_;
        speedIncValue_ = 0;
        speedIncPer_   = 0;
        speedIncGoal_  = speed_;
    }
    else
    {
        if (currentSpeed_ < speedIncStart_)
        {
            speed_ = speedIncStart_;
        }
        else if (speedIncGoal_ < currentSpeed_)
        {
            speed_ = speedIncGoal_;
        }
        else
        {
            speed_ = currentSpeed_;
        }
        speedIncValue_ = model_->getSpeedIncValue();
        speedIncPer_   = model_->getSpeedIncPer();
        speedIncGoal_  = model_->getSpeedIncGoal();
    }
    
    count_ = 0;
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

void MelissaAudioEngine::speedChanged(int speed)
{
    speed_ = speed;
    currentSpeed_ = speed;
    model_->setCurrentSpeed(speed);
    processStartIndex_ =  playingPosMSec_ * originalSampleRate_ / 1000.f;
    needToReset_ = true;
}

void MelissaAudioEngine::speedIncStartChanged(int speedIncStart)
{
    speedIncStart_ = speedIncStart;
    count_ = 0;
}

void MelissaAudioEngine::speedIncValueChanged(int speedIncValue)
{
    speedIncValue_ = speedIncValue;
    count_ = 0;
}

void MelissaAudioEngine::speedIncPerChanged(int speedIncPer)
{
    speedIncPer_ = speedIncPer;
    count_ = 0;
}

void MelissaAudioEngine::speedIncGoalChanged(int speedIncGoal)
{
    speedIncGoal_ = speedIncGoal;
    count_ = 0;
}

void MelissaAudioEngine::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    if (!(0 <= aRatio && aRatio < bRatio && bRatio <= 1.f)) return;
    
    aIndex_ = static_cast<int32_t>(aRatio * originalBufferLength_);
    bIndex_ = static_cast<int32_t>(bRatio * originalBufferLength_);
    if (aIndex_ <= readIndex_ && readIndex_ < bIndex_) return;
    readIndex_ = aIndex_;
    needToReset_ = true;
}

void MelissaAudioEngine::playingPosChanged(float time, float ratio)
{
    processStartIndex_ = static_cast<size_t>((originalBufferLength_ - 1) * ratio);
    if (processStartIndex_ < aIndex_) processStartIndex_ = aIndex_;
    if (bIndex_ < processStartIndex_) processStartIndex_ = bIndex_;
    needToReset_ = true;
}

void MelissaAudioEngine::metronomeSwitchChanged(bool on)
{
    metronome_.on_ = on;
}

void MelissaAudioEngine::bpmChanged(float bpm)
{
    metronome_.bpm_ = bpm;
}

void MelissaAudioEngine::beatPositionChanged(float beatPositionMSec)
{
    metronome_.beatPositionMSec_ = beatPositionMSec;
}

void MelissaAudioEngine::accentUpdated(int accent)
{
    metronome_.accent_ = accent;
}

void MelissaAudioEngine::metronomeVolumeUpdated(float volume)
{
    metronome_.volume_ = volume;
}

void MelissaAudioEngine::musicMetronomeBalanceUpdated(float balance)
{
    volumeBalance_ = balance;
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
