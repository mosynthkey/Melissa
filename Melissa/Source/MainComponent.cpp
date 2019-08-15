#include <sstream>
#include "MainComponent.h"
#include "MelissaUtility.h"

using std::make_unique;

class MelissaRoundButton : public Button
{
public:
    MelissaRoundButton(const String& name) : Button(name)
    {
        setOpaque(false);
    }
    
private:
    void paintButton (Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& b = getLocalBounds();
        constexpr int t = 2; // thickness
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(t / 2, t / 2, b.getWidth() - t - 1, b.getHeight() - t - 1, (b.getHeight() - t) / 2, t);
    }
};

class MelissaPlayButton : public Button
{
public:
    MelissaPlayButton(const String& name) : Button(name)
    {
        setOpaque(false);
    }
    
private:
    void paintButton (Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& b = getLocalBounds();
        
        constexpr int t = 2; // thickness
        const int w = b.getWidth();
        const int h = b.getHeight();
        const int triW = w * 9.f / 23.f;
        const int triH = h * 3.f / 7.f;
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawEllipse(t / 2, t / 2, w - t - 1, h - t - 1, t);
        
        const int x0 = (w - triW) * 4.f / 7.f;
        const int y0 = (h - triH) / 2;
        
        Path path;
        path.addTriangle (x0, y0, x0, y0 + triH, x0 + triW, h / 2);
        g.fillPath(path);
    }
};

MainComponent::MainComponent() : Thread("MelissaProcessThread"),
status_(kStatus_Stop),
melissa_(make_unique<Melissa>()),
waveformComponent_(make_unique<MelissaWaveformControlComponent>()),
controlComponent_(make_unique<MelissaControlComponent>()),
button_(make_unique<MelissaRoundButton>("testButton")),
playButton_(make_unique<MelissaPlayButton>("playButton")),
debugComponent_(make_unique<MelissaDebugComponent>())
{
#if JUCE_MAC || JUCE_WINDOWS
    getLookAndFeel().setDefaultSansSerifTypefaceName("Helvetica Neue");
#endif
    
    setSize (1400, 860);
    
    waveformComponent_->setBounds(60, 20, getWidth() - 60 * 2, 200);
    waveformComponent_->setListener(this);
    addAndMakeVisible(waveformComponent_.get());
    
    controlComponent_->setBounds(0, 240, getWidth(), 240);
    addAndMakeVisible(controlComponent_.get());
    
    button_->setBounds(0, 240, 140, 40);
    addAndMakeVisible(button_.get());
    
    addAndMakeVisible(playButton_.get());
    
    debugComponent_->setBounds(0, 500, getWidth(), 360);
    debugComponent_->fileBrowserComponent_->addListener(this);
    debugComponent_->playButton_->onClick  = [this]() { play(); };
    debugComponent_->pauseButton_->onClick = [this]() { pause(); };
    debugComponent_->stopButton_->onClick  = [this]() { stop(); };
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
    startTimer(100);
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
    playButton_->setBounds((getWidth() - 100) / 2, 300, 100, 100);
}

void MainComponent::fileDoubleClicked(const File& file)
{
    openFile(file);
}

void MainComponent::setPlayPosition(MelissaWaveformControlComponent* sender, float ratio)
{
    std::cout << "setPlayPosition " << ratio << std::endl;
    if (melissa_ == nullptr) return;
    
    melissa_->setPlayingPosRatio(ratio);
}

void MainComponent::setAPosition(MelissaWaveformControlComponent* sender, float ratio)
{
    std::cout << "setAPosition " << ratio << std::endl;
    if (melissa_ == nullptr) return;
    
    melissa_->setAPosRatio(ratio);
}

void MainComponent::setBPosition(MelissaWaveformControlComponent* sender, float ratio)
{
    std::cout << "setBPosition " << ratio << std::endl;
    if (melissa_ == nullptr) return;
    
    melissa_->setBPosRatio(ratio);
}

void MainComponent::run()
{
    while (true)
    {
        if (melissa_ != nullptr && melissa_->needToProcess())
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
    
    std::stringstream ss;
    ss << "Prosessing : " << static_cast<uint32_t>(melissa_->getProgress() * 100) << "%";
    
    debugComponent_->statusLabel_->setText(melissa_->needToProcess() ? ss.str() : "Process done", NotificationType::dontSendNotification);
    debugComponent_->debugLabel_->setText(melissa_->getStatusString(), dontSendNotification);
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
    
    
    return true;
}

void MainComponent::play()
{
    if (melissa_ == nullptr) return;
    status_ = kStatus_Playing;
}

void MainComponent::pause()
{
    if (melissa_ == nullptr) return;
    status_ = kStatus_Pause;
}

void MainComponent::stop()
{
    if (melissa_ == nullptr) return;
    status_ = kStatus_Stop;
    melissa_->setPlayingPosMSec(0);
}

