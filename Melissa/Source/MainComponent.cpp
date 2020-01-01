#include <sstream>
#include "MainComponent.h"
#include "MelissaUtility.h"

using std::make_unique;

enum
{
    kFileChooserTabGroup = 1001,
    kPracticeMemoTabGroup = 1002,
    
    kMaxSizeOfRecentList = 50,
};


MainComponent::MainComponent() : Thread("MelissaProcessThread"),
status_(kStatus_Stop), shouldExit_(false)
{
#if JUCE_MAC || JUCE_WINDOWS
    getLookAndFeel().setDefaultSansSerifTypefaceName("Hiragino Kaku Gothic ProN");
#endif
    
    addListener(this);
    
    melissa_ = make_unique<Melissa>();
    
    // look and feel
    setLookAndFeel(&lookAndFeel_);
    
    waveformComponent_ = make_unique<MelissaWaveformControlComponent>();
    waveformComponent_->setListener(this);
    addAndMakeVisible(waveformComponent_.get());
    
    controlComponent_ = make_unique<MelissaControlComponent>();
    addAndMakeVisible(controlComponent_.get());
    
    playPauseButton_ = make_unique<MelissaPlayPauseButton>("PlayButton");
    playPauseButton_->onClick = [this]() { if (status_ == kStatus_Playing) { pause(); } else { play(); } };
    addAndMakeVisible(playPauseButton_.get());
    
    toHeadButton_ = make_unique<MelissaToHeadButton>("ToHeadButton");
    toHeadButton_->onClick = [this]() { toHead(); };
    addAndMakeVisible(toHeadButton_.get());
    
    timeLabel_ = make_unique<Label>();
    timeLabel_->setJustificationType(Justification::centred);
    timeLabel_->setText("-:--", dontSendNotification);
    addAndMakeVisible(timeLabel_.get());
    
    fileNameLabel_ = make_unique<Label>();
    fileNameLabel_->setJustificationType(Justification::centred);
    fileNameLabel_->setText("---", dontSendNotification);
    addAndMakeVisible(fileNameLabel_.get());
    
    {
        metronomeOnOffButton_ = make_unique<ToggleButton>();
        metronomeOnOffButton_->setClickingTogglesState(true);
        metronomeOnOffButton_->onClick = [this]()
        {
            melissa_->setMetoronome(metronomeOnOffButton_->getToggleState());
        };
        metronomeOnOffButton_->setButtonText("On");
        addAndMakeVisible(metronomeOnOffButton_.get());
    }
    
    {
        bpmButton_ = make_unique<MelissaIncDecButton>();
        bpmButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setBpm(melissa_->getBpm() - (b ? 10 : 1));
            updateBpm();
        };
        bpmButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setBpm(melissa_->getBpm() + (b ? 10 : 1));
            updateBpm();
        };
        addAndMakeVisible(bpmButton_.get());
    }
    
    {
        metronomeOffsetButton_ = make_unique<MelissaIncDecButton>();
        metronomeOffsetButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setMetronomeOffsetSec(melissa_->getMetronomeOffsetSec() - 1);
            updateMetronomeOffset();
        };
        metronomeOffsetButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setMetronomeOffsetSec(melissa_->getMetronomeOffsetSec() + 1);
            updateMetronomeOffset();
        };
        addAndMakeVisible(metronomeOffsetButton_.get());
    }
    
    {
        analyzeButton_ = make_unique<TextButton>();
        analyzeButton_->setButtonText("Analyze");
        analyzeButton_->onClick = [this]()
        {
            melissa_->analyzeBpm();
            updateBpm();
            updateMetronomeOffset();
        };
        addAndMakeVisible(analyzeButton_.get());
    }
    
    {
        volumeSlider_ = make_unique<Slider>();
        volumeSlider_->setRange(0.1f, 4.0f);
        volumeSlider_->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        volumeSlider_->onValueChange = [this]()
        {
            melissa_->setVolume(volumeSlider_->getValue());
        };
        addAndMakeVisible(volumeSlider_.get());
    }
    
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
        resetButton_ = make_unique<TextButton>();
        resetButton_->setButtonText("Reset");
        resetButton_->onClick = [this]() { resetLoop(); };
        addAndMakeVisible(resetButton_.get());
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
            melissa_->setSpeed(melissa_->getSpeed() - (b ? 1 : 10));
            updateSpeedButtonLabel();
        };
        speedButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setSpeed(melissa_->getSpeed() + (b ? 1 : 10));
            updateSpeedButtonLabel();
        };
        speedButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        addAndMakeVisible(speedButton_.get());
    }
    
    {
        speedIncPerButton_ = make_unique<MelissaIncDecButton>();
        speedIncPerButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setSpeedIncPer(melissa_->getSpeedIncPer() - 1);
            updateSpeedButtonLabel();
        };
        speedIncPerButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setSpeedIncPer(melissa_->getSpeedIncPer() + 1);
            updateSpeedButtonLabel();
        };
        addAndMakeVisible(speedIncPerButton_.get());
    }
    
    {
        speedIncValueButton_ = make_unique<MelissaIncDecButton>();
        speedIncValueButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setSpeedIncValue(melissa_->getSpeedIncValue() - 1);
            updateSpeedButtonLabel();
        };
        speedIncValueButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setSpeedIncValue(melissa_->getSpeedIncValue() + 1);
            updateSpeedButtonLabel();
        };
        addAndMakeVisible(speedIncValueButton_.get());
    }
    
    {
        speedIncMaxButton_ = make_unique<MelissaIncDecButton>();
        speedIncMaxButton_->onClickDecButton_ = [this](bool b)
        {
            melissa_->setSpeedIncMax(melissa_->getSpeedIncMax() - 1);
            updateSpeedButtonLabel();
        };
        speedIncMaxButton_->onClickIncButton_ = [this](bool b)
        {
            melissa_->setSpeedIncMax(melissa_->getSpeedIncMax() + 1);
            updateSpeedButtonLabel();
        };
        addAndMakeVisible(speedIncMaxButton_.get());
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
    
    {
        browseToggleButton_ = make_unique<ToggleButton>();
        browseToggleButton_->setButtonText("Browse");
        browseToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        browseToggleButton_->setRadioGroupId(kFileChooserTabGroup);
        browseToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Browse); };
        addAndMakeVisible(browseToggleButton_.get());
        
        setListToggleButton_ = make_unique<ToggleButton>();
        setListToggleButton_->setButtonText("Set List");
        setListToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        setListToggleButton_->setRadioGroupId(kFileChooserTabGroup);
        setListToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_SetList); };
        addAndMakeVisible(setListToggleButton_.get());
        
        recentToggleButton_ = make_unique<ToggleButton>();
        recentToggleButton_->setButtonText("Recent");
        recentToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        recentToggleButton_->setRadioGroupId(kFileChooserTabGroup);
        recentToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Recent); };
        addAndMakeVisible(recentToggleButton_.get());
    }
    
    {
        recentTable_ = make_unique<MelissaRecentListBox>(this);
        recentTable_->setLookAndFeel(&lookAndFeel_);
        addAndMakeVisible(recentTable_.get());
    }
    
    {
        practiceTable_ = make_unique<MelissaPracticeTableListBox>(this);
        addAndMakeVisible(practiceTable_.get());
    }
    
    {
        memoTextEditor_ = make_unique<TextEditor>();
        memoTextEditor_->setLookAndFeel(nullptr);
        memoTextEditor_->setMultiLine(true, false);
        memoTextEditor_->setLookAndFeel(&lookAndFeelMemo_);
        memoTextEditor_->onFocusLost = [&]()
        {
            saveMemo();
        };
        memoTextEditor_->setReturnKeyStartsNewLine(true);
        addAndMakeVisible(memoTextEditor_.get());
    }
    
    {
        practiceListToggleButton_ = make_unique<ToggleButton>();
        practiceListToggleButton_ = make_unique<ToggleButton>();
        practiceListToggleButton_->setButtonText("Practice List");
        practiceListToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        practiceListToggleButton_->setRadioGroupId(kPracticeMemoTabGroup);
        practiceListToggleButton_->onClick = [&]() { updatePracticeMemo(kPracticeMemoTab_Practice); };
        addAndMakeVisible(practiceListToggleButton_.get());
        
        memoToggleButton_ = make_unique<ToggleButton>();
        memoToggleButton_->setButtonText("Memo");
        memoToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        memoToggleButton_->setRadioGroupId(kPracticeMemoTabGroup);
        memoToggleButton_->onClick = [&]() { updatePracticeMemo(kPracticeMemoTab_Memo); };
        addAndMakeVisible(memoToggleButton_.get());
        
        practiceListToggleButton_->setToggleState(true, dontSendNotification);
        updatePracticeMemo(kPracticeMemoTab_Practice);
    }
    
    {
        pracListNameTextEditor_ = make_unique<TextEditor>();
        pracListNameTextEditor_->setJustification(Justification::centredLeft);
        addAndMakeVisible(pracListNameTextEditor_.get());
    }
    
    {
        addToListButton_ = make_unique<TextButton>();
        addToListButton_->setButtonText("Add");
        addToListButton_->onClick = [this]()
        {
            auto name = pracListNameTextEditor_->getText();
            if (name.isEmpty()) name = MelissaUtility::getFormattedTimeMSec(melissa_->getAPosMSec());
            
            addToPracticeList(name);
            
            if (auto songs = setting_["songs"].getArray())
            {
                auto song = getSongSetting(fileFullPath_);
                practiceTable_->setList(*(song.getProperty("list", Array<var>()).getArray()), melissa_->getTotalLengthMSec());
            }
        };
        addAndMakeVisible(addToListButton_.get());
    }
    
    {
        wildCardFilter_ = std::make_unique<WildcardFileFilter>("*.mp3;*.wav;*.m4a", "*", "Music Files");
        fileBrowserComponent_ = make_unique<FileBrowserComponent>(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
                                                                  File::getSpecialLocation(File::userHomeDirectory),
                                                                  wildCardFilter_.get(),
                                                                  nullptr);
        fileBrowserComponent_->setColour(ListBox::backgroundColourId, Colours::transparentWhite);
        fileBrowserComponent_->setFilenameBoxLabel("File");
        fileBrowserComponent_->addListener(this);
        addAndMakeVisible(fileBrowserComponent_.get());
    }
    
    {
        for (size_t sectionTitle_i = 0; sectionTitle_i < kNumOfSectionTitles; ++sectionTitle_i)
        {
            const std::string titles_[] = { "A-B Loop", "Speed" };
            auto sectionTitle = std::make_unique<MelissaSectionTitleComponent>(titles_[sectionTitle_i], 200);
            addAndMakeVisible(sectionTitle.get());
            sectionTitles_.emplace_back(std::move(sectionTitle));
        }
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
    
    {
        // load setting file
        settingsDir_ = (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Melissa"));
        if (!(settingsDir_.exists() && settingsDir_.isDirectory()))
        {
            settingsDir_.createDirectory();
        }
        settingsFile_ = settingsDir_.getChildFile("Settings.melissa");
        if (!settingsFile_.existsAsFile()) createSettingsFile();
        
        setting_ = JSON::parse(settingsFile_.loadFileAsString());
        
        auto global = setting_["global"];
        fileBrowserComponent_->setRoot(File(global.getProperty("root_dir", "/")));
        
        if (!setting_.hasProperty("recent"))
        {
            setting_.getDynamicObject()->setProperty("recent", Array<var>());
        }
        recent_ = setting_["recent"].getArray();
        
        auto current = setting_["current"];
        File file(current["file"].toString());
        if (file.existsAsFile())
        {
            openFile(file);
            
            melissa_->setVolume(static_cast<float>(current.getProperty("volume", 1.f)));
            melissa_->setAPosRatio(static_cast<float>(current.getProperty("a", 0.f)));
            melissa_->setBPosRatio(static_cast<float>(current.getProperty("b", 1.f)));
            melissa_->setSpeed(static_cast<float>(current.getProperty("speed", 100)));
            melissa_->setPitch(static_cast<float>(current.getProperty("pitch", 0.f)));
            updateAll();
        }
        
        recentTable_->setList(*(setting_.getProperty("recent", Array<var>()).getArray()));
        browseToggleButton_->setToggleState(true, dontSendNotification);
        setListToggleButton_->setToggleState(false, dontSendNotification);
        recentToggleButton_->setToggleState(false, dontSendNotification);
        updateFileChooserTab(kFileChooserTab_Browse);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    
    saveSettings();
    
    setLookAndFeel(nullptr);
    recentTable_->setLookAndFeel(nullptr);
    browseToggleButton_->setLookAndFeel(nullptr);
    setListToggleButton_->setLookAndFeel(nullptr);
    recentToggleButton_->setLookAndFeel(nullptr);
    practiceListToggleButton_->setLookAndFeel(nullptr);
    memoToggleButton_->setLookAndFeel(nullptr);
    memoTextEditor_->setLookAndFeel(nullptr);
    
    stopThread(4000.f);
    stopTimer();
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
    g.setGradientFill(ColourGradient(Colour(0xff4a90e2), 0.f, 0.f, Colour(0xff204673), getWidth(), getHeight(), false));
    g.fillAll();
}

void MainComponent::resized()
{
    controlComponent_->setBounds(0, 220, getWidth(), 280);
    playPauseButton_->setBounds((getWidth() - 100) / 2, 280, 100, 100);
    toHeadButton_->setBounds(playPauseButton_->getX() - 60 , playPauseButton_->getY() + playPauseButton_->getHeight() / 2 - 15, 30, 30);
    fileNameLabel_->setBounds(getWidth() / 2 - 100, playPauseButton_->getBottom() + 10, 200, 30);
    timeLabel_->setBounds(getWidth() / 2 - 100, fileNameLabel_->getBottom(), 200, 30);
    waveformComponent_->setBounds(60, 20, getWidth() - 60 * 2, 200);
    
    // left-bottom part (Browser / SetList / Recent)
    {
        constexpr int32_t tabMargin = 2;
        int32_t browserWidth = 360;
        int32_t w = (browserWidth - tabMargin * (kNumOfFileChooserTabs - 1)) / kNumOfFileChooserTabs;
        int32_t y = controlComponent_->getBottom();
        
        int32_t x = 20;
        browseToggleButton_ ->setBounds(x, y, w, 30);
        x += (w + tabMargin);
        setListToggleButton_->setBounds(x, y, w, 30);
        x += (w + tabMargin);
        recentToggleButton_ ->setBounds(x, y, w, 30);
        
        w = 180;
        x = 20 + browserWidth + 20;
        practiceListToggleButton_->setBounds(x, y, w, 30);
        x += (w + 2);
        memoToggleButton_->setBounds(x, y, w, 30);
        
        pracListNameTextEditor_->setBounds(getWidth() - 60 - 20 - 220, y, 200, 30);
        addToListButton_->setBounds(getWidth() - 60 - 20, y, 60, 30);
        
        y += 40;
        const int32_t h = getHeight() - y - 20;
        fileBrowserComponent_->setBounds(20, y, browserWidth, h);
        recentTable_->setBounds(fileBrowserComponent_->getBounds());
        practiceTable_->setBounds(fileBrowserComponent_->getRight() + 20, y, getWidth() - fileBrowserComponent_->getRight() - 40, h);
        memoTextEditor_->setBounds(fileBrowserComponent_->getRight() + 20, y, getWidth() - fileBrowserComponent_->getRight() - 40, h);
    }
    
    int32_t marginX = 60;
    
    {
        int32_t y = 300;
        metronomeOnOffButton_->setBounds(marginX, y, 80, 30);
        bpmButton_->setBounds(metronomeOnOffButton_->getRight() + 10, y, 140, 30);
        metronomeOffsetButton_->setBounds(bpmButton_->getRight() + 10, y, 140, 30);
        analyzeButton_->setBounds(metronomeOffsetButton_->getRight() + 10, y, 80, 30);
    }
    
    {
        int32_t y = 390;
        volumeSlider_->setBounds(metronomeOnOffButton_->getX(), y, 200, 30);
        
        int32_t x = 300;
        pitchLabel_->setBounds(x, y, 60, 30);
        x = pitchLabel_->getRight() + 10;
        pitchButton_->setBounds(x, y, 140, 30);
    }
    
    {
        int32_t y = 300;
        int32_t x = getWidth() - 60 - marginX;
        resetButton_->setBounds(x, y, 70, 30);
        
        x = resetButton_->getX() - 140 - 10;
        bButton_->setBounds(x, y, 140, 30);
        x = bButton_->getX() - 60 - 10;
        bSetButton_->setBounds(x, y,  60, 30);
        
        x = bSetButton_->getX() - 140 - 10;
        aButton_->setBounds(x, y, 140, 30);
        x = aButton_->getX() - 60 - 10;
        aSetButton_->setBounds(x, y,  60, 30);
        
        sectionTitles_[kSectionTitle_Loop]->setBounds(aSetButton_->getX(), y - 30 - 10, resetButton_->getRight() - aSetButton_->getX(), 30);
        
        y += 110;
        speedLabel_->setBounds(aSetButton_->getBounds().withY(y));
        speedButton_->setBounds(aButton_->getBounds().withY(y));
        
        x = bSetButton_->getX();
        speedIncValueButton_->setBounds(x, y - 20, 140, 30);
        speedIncPerButton_->setBounds(x, y + 20, 140, 30);
        speedIncMaxButton_->setBounds(x + 150, y, 140, 30);
        
        sectionTitles_[kSectionTitle_Speed]->setBounds(aSetButton_->getX(), y - 50 - 10, resetButton_->getRight() - aSetButton_->getX(), 30);
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

void MainComponent::setMelissaParameters(float aRatio, float bRatio, float speed, int32_t pitch)
{
    melissa_->setAPosRatio(aRatio);
    melissa_->setBPosRatio(bRatio);
    melissa_->setSpeed(speed);
    melissa_->setPitch(pitch);
    updateAll();
}

void MainComponent::getMelissaParameters(float* aRatio, float* bRatio, float* speed, int32_t* pitch, int32_t* count)
{
    *aRatio = melissa_->getAPosRatio();
    *bRatio = melissa_->getBPosRatio();
    *speed  = melissa_->getSpeed();
    *pitch  = melissa_->getPitch();
    *count  = 0;
}

void MainComponent::updatePracticeList(const Array<var>& list)
{
    if (auto songs = setting_["songs"].getArray())
    {
        auto song = getSongSetting(fileFullPath_);
        song.getDynamicObject()->setProperty("list", list);
    }
}

void MainComponent::loadFile(const String& filePath)
{
    openFile(File(filePath));
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
    while (!shouldExit_)
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

void MainComponent::exitSignalSent()
{
    shouldExit_ = true;
}

void MainComponent::timerCallback()
{
    if (melissa_ == nullptr) return;
    
    timeLabel_->setText(MelissaUtility::getFormattedTimeMSec(melissa_->getPlayingPosMSec()), dontSendNotification);
    waveformComponent_->setPlayPosition(melissa_->getPlayingPosRatio());
}

void MainComponent::updateFileChooserTab(FileChooserTab tab)
{
    fileBrowserComponent_->setVisible(tab == kFileChooserTab_Browse);
    recentTable_->setVisible(tab == kFileChooserTab_Recent);
}

void MainComponent::updatePracticeMemo(PracticeMemoTab tab)
{
    practiceTable_->setVisible(tab == kPracticeMemoTab_Practice);
    memoTextEditor_->setVisible(tab == kPracticeMemoTab_Memo);
    
}

bool MainComponent::openFile(const File& file)
{
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return false;
    
    fileNameLabel_->setText("Loading...", dontSendNotification);
    
    // read audio data from reader
    const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
    audioSampleBuf_ = make_unique<AudioSampleBuffer>(2, lengthInSamples);
    reader->read(audioSampleBuf_.get(), 0, lengthInSamples, 0, true, true);
    
    melissa_->reset();
    const float* buffer[] = { audioSampleBuf_->getReadPointer(0), audioSampleBuf_->getReadPointer(1) };
    melissa_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    waveformComponent_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    audioSampleBuf_ = nullptr;
    
    fileName_ = file.getFileName();
    fileFullPath_ = file.getFullPathName();
    fileNameLabel_->setText(fileName_, dontSendNotification);
    updateAll();
    
    if (auto songs = setting_["songs"].getArray())
    {
        auto song = getSongSetting(fileFullPath_);
        practiceTable_->setList(*(song.getProperty("list", Array<var>()).getArray()), melissa_->getTotalLengthMSec());
        memoTextEditor_->setText(song.getProperty("memo", "").toString());
    }
    
    addToRecent(fileFullPath_);
    recentTable_->setList(*(setting_.getProperty("recent", Array<var>()).getArray()));
    
    delete reader;
    
    saveSettings();
    
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

void MainComponent::toHead()
{
    if (melissa_ == nullptr) return;
    melissa_->setPlayingPosRatio(melissa_->getAPosRatio());
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

void MainComponent::addToPracticeList(String name)
{
    auto addToList = [this, name](Array<var>* list)
    {
        auto prac = new DynamicObject();
        prac->setProperty("name", name);
        prac->setProperty("a", melissa_->getAPosRatio());
        prac->setProperty("b", melissa_->getBPosRatio());
        prac->setProperty("speed", melissa_->getSpeed());
        prac->setProperty("pitch", melissa_->getPitch());
        prac->setProperty("count", 0);
        list->addIfNotAlreadyThere(prac);
    };
    
    auto songs = setting_.getProperty("songs", Array<var>()).getArray();
    for (auto& song : *songs)
    {
        if (song.getProperty("file", "").toString() == fileFullPath_)
        {
            auto list = song.getProperty("list", Array<var>()).getArray();
            addToList(list);
            return;
        }
    }
    
    auto song = new DynamicObject();
    song->setProperty("file", fileFullPath_);
    song->setProperty("volume", melissa_->getVolume());
    song->setProperty("bpm", static_cast<float>(melissa_->getBpm()));
    song->setProperty("metronome_offset", melissa_->getMetronomeOffsetSec());
    song->setProperty("list", Array<var>());
    song->setProperty("memo", memoTextEditor_->getText());
    addToList(song->getProperty("list").getArray());
    songs->addIfNotAlreadyThere(song);
}

void MainComponent::saveMemo()
{
    auto songs = setting_.getProperty("songs", Array<var>()).getArray();
    for (auto& song : *songs)
    {
        if (song.getProperty("file", "").toString() == fileFullPath_)
        {
            song.getDynamicObject()->setProperty("memo", memoTextEditor_->getText());
            return;
        }
    }
    
    auto song = new DynamicObject();
    song->setProperty("file", fileFullPath_);
    song->setProperty("volume", melissa_->getVolume());
    song->setProperty("bpm", static_cast<float>(melissa_->getBpm()));
    song->setProperty("metronome_offset", melissa_->getMetronomeOffsetSec());
    song->setProperty("list", Array<var>());
    song->setProperty("memo", memoTextEditor_->getText());
    songs->addIfNotAlreadyThere(song);
}

void MainComponent::addToRecent(String filePath)
{
    recent_->removeFirstMatchingValue(filePath);
    recent_->insert(0, filePath);
    if (recent_->size() >= kMaxSizeOfRecentList)
    {
        recent_->resize(kMaxSizeOfRecentList);
    }
}

void MainComponent::updateAll()
{
    updateAButtonLabel();
    updateBButtonLabel();
    updateSpeedButtonLabel();
    updatePitchButtonLabel();
    updateBpm();
    updateMetronomeOffset();
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
    const int32_t speed = melissa_->getSpeed();
    speedButton_->setText(String(speed) + "%");
    
    const int32_t incPer = melissa_->getSpeedIncPer();
    speedIncPerButton_->setText("/ " + String(incPer));
    
    const int32_t incValue = melissa_->getSpeedIncValue();
    speedIncValueButton_->setText("+ " + String(incValue) + " %");
    
    const int32_t incMax = melissa_->getSpeedIncMax();
    speedIncMaxButton_->setText("Max: " + String(incMax) + " %");
}

void MainComponent::updatePitchButtonLabel()
{
    pitchButton_->setText(MelissaUtility::getFormattedPitch(melissa_->getPitch()));
}

void MainComponent::updateBpm()
{
    bpmButton_->setText(String(static_cast<uint32_t>(melissa_->getBpm())));
}

void MainComponent::updateMetronomeOffset()
{
    metronomeOffsetButton_->setText(MelissaUtility::getFormattedTimeMSec(melissa_->getMetronomeOffsetSec() * 1000.f));
}

void MainComponent::createSettingsFile()
{
    auto all = new DynamicObject();
    
    auto global = new DynamicObject();
    global->setProperty("version", 1);
    all->setProperty("global", var(global));
    
    auto current = new DynamicObject();
    current->setProperty("file", fileFullPath_);
    current->setProperty("volume", melissa_->getVolume());
    current->setProperty("bpm", static_cast<float>(melissa_->getBpm()));
    current->setProperty("metronome_offset", melissa_->getMetronomeOffsetSec());
    current->setProperty("a", melissa_->getAPosRatio());
    current->setProperty("b", melissa_->getBPosRatio());
    current->setProperty("speed", melissa_->getSpeed());
    current->setProperty("pitch", melissa_->getPitch());
    all->setProperty("current", var(current));
    all->setProperty("recent", Array<var>());
    all->setProperty("songs", Array<var>());
    
    settingsFile_.replaceWithText(JSON::toString(all));
}

void MainComponent::saveSettings()
{
    saveMemo();
    
    auto global = setting_["global"].getDynamicObject();
    global->setProperty("version", "1.0");
    global->setProperty("root_dir", fileBrowserComponent_->getRoot().getFullPathName());
    
    auto current = setting_["current"].getDynamicObject();
    current->setProperty("file", fileFullPath_);
    current->setProperty("volume", melissa_->getVolume());
    current->setProperty("bpm", static_cast<float>(melissa_->getBpm()));
    current->setProperty("metronome_offset", melissa_->getMetronomeOffsetSec());
    current->setProperty("a", melissa_->getAPosRatio());
    current->setProperty("b", melissa_->getBPosRatio());
    current->setProperty("speed", melissa_->getSpeed());
    current->setProperty("pitch", melissa_->getPitch());
    
    settingsFile_.replaceWithText(JSON::toString(setting_));
}

var MainComponent::getSongSetting(String fileName)
{
    auto songs = setting_["songs"].getArray();
    for (auto& song : *songs)
    {
        if (song.getProperty("file", "").toString() == fileName) return song;
    }
    
    return var();
}
