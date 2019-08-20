#include <sstream>
#include "MainComponent.h"
#include "MelissaUtility.h"

using std::make_unique;

MainComponent::MainComponent() : Thread("MelissaProcessThread"),
status_(kStatus_Stop)
{
#if JUCE_MAC || JUCE_WINDOWS
    getLookAndFeel().setDefaultSansSerifTypefaceName("Hiragino Kaku Gothic ProN");
#endif
    
    melissa_ = make_unique<Melissa>();
    
    setLookAndFeel(&lookAndFeel_);
    
    waveformComponent_ = make_unique<MelissaWaveformControlComponent>();
    waveformComponent_->setListener(this);
    addAndMakeVisible(waveformComponent_.get());
    
    controlComponent_ = make_unique<MelissaControlComponent>();
    addAndMakeVisible(controlComponent_.get());
    
    playPauseButton_ = make_unique<MelissaPlayPauseButton>("playButton");
    playPauseButton_->onClick = [this]() { if (status_ == kStatus_Playing) { pause(); } else { play(); } };
    addAndMakeVisible(playPauseButton_.get());
    
    debugComponent_ = make_unique<MelissaDebugComponent>();
    debugComponent_->setLookAndFeel(&lookAndFeel_);
    debugComponent_->fileBrowserComponent_->addListener(this);
    debugComponent_->playButton_->onClick  = [this]() { play(); };
    debugComponent_->pauseButton_->onClick = [this]() { pause(); };
    debugComponent_->stopButton_->onClick  = [this]() { stop(); };
    debugComponent_->resetLoopButton_->onClick  = [this]() { resetLoop(); };
    debugComponent_->posSlider_->onDragEnd = [this]()
    {
        const float playPointMSec = debugComponent_->posSlider_->getValue() / 100.f * melissa_->getTotalLengthMSec();
        melissa_->setPlayingPosMSec(playPointMSec);
    };
    debugComponent_->aSlider_->onDragEnd = [this]()
    {
        const auto value = debugComponent_->aSlider_->getValue();
        const float pointMSec = value / 100.f * melissa_->getTotalLengthMSec();
        melissa_->setAPosMSec(pointMSec);
        debugComponent_->aLabel_->setText("A:" + MelissaUtility::getFormattedTimeMSec(pointMSec), dontSendNotification);
    };
    debugComponent_->bSlider_->onDragEnd = [this]()
    {
        const auto value = debugComponent_->bSlider_->getValue();
        const float pointMSec = value / 100.f * melissa_->getTotalLengthMSec();
        melissa_->setBPosMSec(pointMSec);
        debugComponent_->bLabel_->setText("B:" + MelissaUtility::getFormattedTimeMSec(pointMSec), dontSendNotification);
    };
    debugComponent_->rateSlider_->onDragEnd = [this]()
    {
        melissa_->setSpeed(debugComponent_->rateSlider_->getValue() / 1000.f);
    };
    debugComponent_->pitchSlider_->onDragEnd = [this]()
    {
        melissa_->setPitch(debugComponent_->pitchSlider_->getValue());
    };
    debugComponent_->volumeSlider_->onDragEnd = [this]()
    {
        melissa_->setVolume(debugComponent_->volumeSlider_->getValue());
    };
    addAndMakeVisible(debugComponent_.get());
    
    {
        aButton_ = make_unique<MelissaIncDecButton>();
        aButton_->setText("-:--");
        aButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setAPosMSec(melissa_->getAPosMSec() - (b ? 100 : 1000));
            updateAButtonLabel();
        };
        aButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setAPosMSec(melissa_->getAPosMSec() + (b ? 100 : 1000));
            updateAButtonLabel();
        };
        addAndMakeVisible(aButton_.get());
    }
    
    {
        aSetButton_ = make_unique<TextButton>();
        aSetButton_->setButtonText("A");
        aSetButton_->onClick = [this]()
        {
            melissa_->setAPosMSec(melissa_->getPlayingPosMSec());
            updateAButtonLabel();
        };
        addAndMakeVisible(aSetButton_.get());
    }
    
    {
        bButton_ = make_unique<MelissaIncDecButton>();
        bButton_->setText("-:--");
        bButton_->setBounds(0, 240, 140, 34);
        bButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setBPosMSec(melissa_->getBPosMSec() - (b ? 100 : 1000));
            updateBButtonLabel();
        };
        bButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setBPosMSec(melissa_->getBPosMSec() + (b ? 100 : 1000));
            updateBButtonLabel();
        };
        addAndMakeVisible(bButton_.get());
    }
    
    {
        bSetButton_ = make_unique<TextButton>();
        bSetButton_->setButtonText("B");
        bSetButton_->onClick = [this]()
        {
            melissa_->setBPosMSec(melissa_->getPlayingPosMSec());
            updateBButtonLabel();
        };
        addAndMakeVisible(bSetButton_.get());
    }
    
    {
        speedLabel_ = make_unique<Label>();
        speedLabel_->setText("Speed", dontSendNotification);
        speedLabel_->setJustificationType(Justification::centred);
        addAndMakeVisible(speedLabel_.get());
    }
    
    {
        speedButton_ = make_unique<MelissaIncDecButton>();
        speedButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setSpeed(melissa_->getSpeed() - (b ? 0.01 : 0.1));
            updateSpeedButtonLabel();
        };
        speedButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setSpeed(melissa_->getSpeed() + (b ? 0.01 : 0.1));
            updateSpeedButtonLabel();
        };
        speedButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        addAndMakeVisible(speedButton_.get());
    }
    
    {
        pitchLabel_ = make_unique<Label>();
        pitchLabel_->setText("Pitch", dontSendNotification);
        pitchLabel_->setJustificationType(Justification::centred);
        addAndMakeVisible(pitchLabel_.get());
    }
    
    {
        pitchButton_ = make_unique<MelissaIncDecButton>();
        pitchButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setPitch(melissa_->getPitch() - 1);
            updatePitchButtonLabel();
        };
        pitchButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setPitch(melissa_->getPitch() + 1);
            updatePitchButtonLabel();
        };
        pitchButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        addAndMakeVisible(pitchButton_.get());
    }
    
    setSize (1400, 860);

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
    
    startThread();
    startTimer(1000 / 10);
    
    addKeyListener(this);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    melissa_->setOutputSampleRate(sampleRate);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    if (melissa_ == nullptr || status_ != kStatus_Playing) return;
    float* buffer[] = { bufferToFill.buffer->getWritePointer(0), bufferToFill.buffer->getWritePointer(1) };
    melissa_->render(buffer, bufferToFill.numSamples);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

void MainComponent::paint(Graphics& g)
{
    g.setGradientFill(ColourGradient(Colour(0xff019cdf), 0.f, 0.f, Colour(0xffc82788), getWidth(), getHeight(), true));
    g.fillAll();
}

void MainComponent::resized()
{
    controlComponent_->setBounds(0, 240, getWidth(), 240);
    playPauseButton_->setBounds((getWidth() - 100) / 2, 300, 100, 100);
    waveformComponent_->setBounds(60, 20, getWidth() - 60 * 2, 200);
    debugComponent_->setBounds(0, 500, getWidth(), 360);
    
    {
        const int32_t y = 300;
        aSetButton_->setBounds(820, y,  60, 30);
        aButton_->setBounds(900, y, 140, 30);
        
        bSetButton_->setBounds(1080, y,  60, 30);
        bButton_->setBounds(1160, y, 140, 30);
    }
    
    {
        const int32_t y = 390;
        speedLabel_->setBounds(820, y, 60, 30);
        speedButton_->setBounds(900, y, 140, 30);
        
        pitchLabel_->setBounds(1080, y, 60, 30);
        pitchButton_->setBounds(1160, y, 140, 30);
    }
}

bool MainComponent::keyPressed(const KeyPress &key, Component* originatingComponent)
{
    const auto keyCode = key.getKeyCode();
    
    if (keyCode == 32) // space
    {
        if (status_ == kStatus_Playing)
        {
            pause();
        }
        else
        {
            play();
        }
        return true;
    }
    else if (keyCode == 63234)
    {
        auto currentMSec = melissa_->getPlayingPosMSec();
        melissa_->setPlayingPosMSec(currentMSec - 1000);
        return true;
    }
    else if (keyCode == 63235)
    {
        auto currentMSec = melissa_->getPlayingPosMSec();
        melissa_->setPlayingPosMSec(currentMSec + 1000);
        return true;
    }
    
    std::cout << keyCode << std::endl;
        
    return false;
}

void MainComponent::fileDoubleClicked(const File& file)
{
    openFile(file);
}

void MainComponent::setPlayPosition(MelissaWaveformControlComponent* sender, float ratio)
{
    if (melissa_ == nullptr) return;
    
    melissa_->setPlayingPosRatio(ratio);
}

void MainComponent::setAPosition(MelissaWaveformControlComponent* sender, float ratio)
{
    if (melissa_ == nullptr) return;
    
    melissa_->setAPosRatio(ratio);
    updateAButtonLabel();
}

void MainComponent::setBPosition(MelissaWaveformControlComponent* sender, float ratio)
{
    if (melissa_ == nullptr) return;
    
    melissa_->setBPosRatio(ratio);
    updateBButtonLabel();
}

void MainComponent::run()
{
    while (true)
    {
        if (melissa_ != nullptr && melissa_->isBufferSet() && melissa_->needToProcess())
        {
            melissa_->process();
        }
        else
        {
            wait(100);
        }
    }
}

void MainComponent::timerCallback()
{
    if (melissa_ == nullptr) return;
    
    debugComponent_->posLabel_->setText(MelissaUtility::getFormattedTimeMSec(melissa_->getPlayingPosMSec()) , dontSendNotification);
    
    debugComponent_->statusLabel_->setText(melissa_->needToProcess() ? "Processing" : "", NotificationType::dontSendNotification);
    debugComponent_->debugLabel_->setText(melissa_->getStatusString(), dontSendNotification);
    
    waveformComponent_->setPlayPosition(melissa_->getPlayingPosRatio());
}

bool MainComponent::openFile(const File& file)
{
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return false;
    
    // read audio data from reader
    const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
    audioSampleBuf_ = make_unique<AudioSampleBuffer>(2, lengthInSamples);
    reader->read(audioSampleBuf_.get(), 0, lengthInSamples, 0, true, true);
    
    melissa_->reset();
    const float* buffer[] = { audioSampleBuf_->getReadPointer(0), audioSampleBuf_->getReadPointer(1) };
    melissa_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    waveformComponent_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    audioSampleBuf_ = nullptr;
    
    updateAll();
    
    return true;
}

void MainComponent::play()
{
    if (melissa_ == nullptr) return;
    status_ = kStatus_Playing;
    playPauseButton_->setMode(MelissaPlayPauseButton::kMode_Pause);
}

void MainComponent::pause()
{
    if (melissa_ == nullptr) return;
    status_ = kStatus_Pause;
    playPauseButton_->setMode(MelissaPlayPauseButton::kMode_Play);
}

void MainComponent::stop()
{
    if (melissa_ == nullptr) return;
    status_ = kStatus_Stop;
    melissa_->setPlayingPosMSec(0);
    playPauseButton_->setMode(MelissaPlayPauseButton::kMode_Play);
}

void MainComponent::resetLoop()
{
    melissa_->setAPosRatio(0.f);
    melissa_->setBPosRatio(1.f);
    waveformComponent_->setAPosition(0.f);
    waveformComponent_->setBPosition(1.f);
    
    updateAButtonLabel();
    updateBButtonLabel();
}

void MainComponent::updateAll()
{
    updateAButtonLabel();
    updateBButtonLabel();
    updateSpeedButtonLabel();
    updatePitchButtonLabel();
}

void MainComponent::updateAButtonLabel()
{
    const auto aPosMSec = melissa_->getAPosMSec();
    aButton_->setText(MelissaUtility::getFormattedTimeMSec(aPosMSec));
    waveformComponent_->setAPosition(melissa_->getAPosRatio());
}

void MainComponent::updateBButtonLabel()
{
    const auto bPosMSec = melissa_->getBPosMSec();
    bButton_->setText(MelissaUtility::getFormattedTimeMSec(bPosMSec));
    waveformComponent_->setBPosition(melissa_->getBPosRatio());
}

void MainComponent::updateSpeedButtonLabel()
{
    int32_t speed = melissa_->getSpeed() * 100;
    speedButton_->setText(String(speed) + "%");
}

void MainComponent::updatePitchButtonLabel()
{
    int32_t pitch = melissa_->getPitch();
    pitchButton_->setText(((pitch > 0) ? String("+") : String("")) + String(pitch));
}
