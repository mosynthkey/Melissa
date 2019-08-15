#include "MainComponent.h"

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

MainComponent::MainComponent() :
controlComponent_(make_unique<MelissaControlComponent>()),
button_(make_unique<MelissaRoundButton>("testButton")),
playButton_(make_unique<MelissaPlayButton>("playButton"))
{
    setSize (1400, 860);
    
    controlComponent_->setBounds(0, 240, getWidth(), 240);
    addAndMakeVisible(controlComponent_.get());
    
    button_->setBounds(0, 240, 140, 40);
    addAndMakeVisible(button_.get());
    
    addAndMakeVisible(playButton_.get());

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
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

void MainComponent::paint (Graphics& g)
{
    g.setGradientFill(ColourGradient(Colour(0xff019cdf), 0.f, 0.f, Colour(0xffc82788), getWidth(), getHeight(), true));
    g.fillAll();
}

void MainComponent::resized()
{
    playButton_->setBounds((getWidth() - 100) / 2, 300, 100, 100);
}
