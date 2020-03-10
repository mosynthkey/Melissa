#include <sstream>
#include "MainComponent.h"
#include "MelissaAboutComponent.h"
#include "MelissaColourScheme.h"
#include "MelissaInputDialog.h"
#include "MelissaUtility.h"

using std::make_unique;

enum
{
    kFileChooserTabGroup = 1001,
    kPracticeMemoTabGroup = 1002,
    
    kMaxSizeOfRecentList = 50,
    
    // UI
    kGradationHeight = 20,
};

enum
{
    kMenuID_MainAbout = 1000,
    kMenuID_MainPreferences = 1001,
    kMenuID_FileOpen = 2000,
};


MainComponent::MainComponent() : Thread("MelissaProcessThread"),
status_(kStatus_Stop), shouldExit_(false)
{
#if JUCE_MAC || JUCE_WINDOWS
    getLookAndFeel().setDefaultSansSerifTypefaceName("Hiragino Kaku Gothic ProN");
#endif
    
    melissa_ = make_unique<Melissa>();
    
    createUI();
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
    
    Thread::addListener(this);
    startThread();
    startTimer(1000 / 10);
    
    addKeyListener(this);
    
    {
        // load setting file
        settingsDir_ = (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Melissa"));
        if (!(settingsDir_.exists() && settingsDir_.isDirectory())) settingsDir_.createDirectory();
        
        settingsFile_ = settingsDir_.getChildFile("Settings.json");
        if (!settingsFile_.existsAsFile()) createSettingsFile();
        
        setting_ = JSON::parse(settingsFile_.loadFileAsString());
        
        auto global = setting_["global"];
        
        auto rootDir = File::getSpecialLocation(File::userHomeDirectory);
        if (global.hasProperty("root_dir"))
        {
            rootDir = File(global.getProperty("root_dir", ""));
            rootDir.setAsCurrentWorkingDirectory();
            fileBrowserComponent_->setRoot(rootDir);
        }
        
        auto width = static_cast<int>(global.getProperty("width", 1400));
        auto height = static_cast<int>(global.getProperty("height", 860));
        setSize(width, height);
        
        if (global.hasProperty("device"))
        {
            deviceManager.initialise(0, 2, XmlDocument::parse(global.getProperty("device", "")).get(), true);
        }
        
        if (!setting_.hasProperty("recent"))
        {
            setting_.getDynamicObject()->setProperty("recent", Array<var>());
        }
        
        recent_ = setting_["recent"].getArray();
        recentTable_->setList(*recent_);
        
        setList_ = setting_["setlist"].getArray();
        setListComponent_->setData(*setList_);
        
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
    }
    
    deviceManager.addMidiInputCallback("", this);
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
    
#if JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr);
#endif
    
    stopThread(4000.f);
    stopTimer();
}

void MainComponent::createUI()
{
    setLookAndFeel(&lookAndFeel_);
    
    menuBar_ = make_unique<MenuBarComponent>(this);
    addAndMakeVisible(menuBar_.get());
    
#if JUCE_MAC
    extraAppleMenuItems_ = make_unique<PopupMenu>();
    extraAppleMenuItems_->addItem("About Melissa", [&]() { showAboutDialog(); });
    extraAppleMenuItems_->addItem("Preferences",   [&]() { showPreferencesDialog(); });
    
    MenuBarModel::setMacMainMenu(this, extraAppleMenuItems_.get());
#endif
    
    menuButton_ = std::make_unique<MelissaMenuButton>();
    menuButton_->onClick = [&]()
    {
        PopupMenu menu;
        menu.addItem(kMenuID_MainAbout, "About");
        menu.addItem(kMenuID_MainPreferences, "Preferenes");
        const auto result = menu.show();
        if (result == kMenuID_MainAbout)
        {
            showAboutDialog();
        }
        else if (result == kMenuID_MainPreferences)
        {
            showPreferencesDialog();
        }
    };
    addAndMakeVisible(menuButton_.get());
    
    waveformComponent_ = make_unique<MelissaWaveformControlComponent>();
    waveformComponent_->setListener(this);
    addAndMakeVisible(waveformComponent_.get());
    
    controlComponent_ = make_unique<MelissaControlComponent>();
    addAndMakeVisible(controlComponent_.get());
    
#if defined(SHOW_BOTTOM)
    bottomComponent_ = make_unique<MelissaBottomControlComponent>(this);
    addAndMakeVisible(bottomComponent_.get());
#endif
    
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
    
    fileNameLabel_ = make_unique<MelissaScrollLabel>(timeLabel_->getFont());
    fileNameLabel_->setText("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU");
    addAndMakeVisible(fileNameLabel_.get());
    
#if defined(ENABLE_METRONOME)
    metronomeOnOffButton_ = make_unique<ToggleButton>();
    metronomeOnOffButton_->setClickingTogglesState(true);
    metronomeOnOffButton_->onClick = [this]()
    {
        melissa_->setMetoronome(metronomeOnOffButton_->getToggleState());
    };
    metronomeOnOffButton_->setButtonText("On");
    addAndMakeVisible(metronomeOnOffButton_.get());
    
    bpmButton_ = make_unique<MelissaIncDecButton>();
    bpmButton_->onClick_ = [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        melissa_->setBpm(melissa_->getBpm() + sign * (b ? 10 : 1));
        updateBpm();
    };
    addAndMakeVisible(bpmButton_.get());
    
    metronomeOffsetButton_ = make_unique<MelissaIncDecButton>();
    metronomeOffsetButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        melissa_->setMetronomeOffsetSec(melissa_->getMetronomeOffsetSec() + sign);
        updateMetronomeOffset();
    };
    addAndMakeVisible(metronomeOffsetButton_.get());
    
    analyzeButton_ = make_unique<TextButton>();
    analyzeButton_->setButtonText("Analyze");
    analyzeButton_->onClick = [this]()
    {
        melissa_->analyzeBpm();
        updateBpm();
        updateMetronomeOffset();
    };
    addAndMakeVisible(analyzeButton_.get());
#endif
    
    {
        volumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        volumeSlider_->setRange(0.01f, 2.0f);
        volumeSlider_->setValue(0.f);
        volumeSlider_->onValueChange = [this]()
        {
            melissa_->setVolume(volumeSlider_->getValue());
            const float db = 20 * log10(volumeSlider_->getValue());
            labels_[kLabel_Volume]->setText(String::formatted("Volume (%+1.2fdB)", db), dontSendNotification);
        };
        addAndMakeVisible(volumeSlider_.get());
    }
    
    {
        aButton_ = make_unique<MelissaIncDecButton>();
        aButton_->setText("-:--");
        aButton_->onClick_= [this](bool inc, bool b)
        {
            const int sign = inc ? 1 : -1;
            melissa_->setAPosMSec(melissa_->getAPosMSec() + sign * (b ? 100 : 1000));
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
        bButton_->onClick_= [this](bool inc, bool b)
        {
            const int sign = inc ? 1 : -1;
            melissa_->setBPosMSec(melissa_->getBPosMSec() + sign * (b ? 100 : 1000));
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
        tie_[0] = std::make_unique<MelissaTieComponent>(aSetButton_.get(), aButton_.get());
        addAndMakeVisible(tie_[0].get());
        tie_[1] = std::make_unique<MelissaTieComponent>(bSetButton_.get(), bButton_.get());
        addAndMakeVisible(tie_[1].get());
    }
    
    {
        speedButton_ = make_unique<MelissaIncDecButton>();
        speedButton_->onClick_= [this](bool inc, bool b)
        {
            const int sign = inc ? 1 : -1;
            melissa_->setSpeed(melissa_->getSpeed() + sign * (b ? 1 : 10));
            updateSpeedButtonLabel();
        };
        speedButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        addAndMakeVisible(speedButton_.get());
    }
    
#if defined(ENABLE_SPEEDTRAINER)
    speedIncPerButton_ = make_unique<MelissaIncDecButton>();
    speedIncPerButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        melissa_->setSpeedIncPer(melissa_->getSpeedIncPer() + sign);
        updateSpeedButtonLabel();
    };
    addAndMakeVisible(speedIncPerButton_.get());
    
    speedIncValueButton_ = make_unique<MelissaIncDecButton>();
    speedIncValueButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        melissa_->setSpeedIncValue(melissa_->getSpeedIncValue() + sign);
        updateSpeedButtonLabel();
    };
    addAndMakeVisible(speedIncValueButton_.get());
    
    speedIncMaxButton_ = make_unique<MelissaIncDecButton>();
    speedIncMaxButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        melissa_->setSpeedIncMax(melissa_->getSpeedIncMax() + sign);
        updateSpeedButtonLabel();
    };
    addAndMakeVisible(speedIncMaxButton_.get());
#endif
    
    {
        pitchButton_ = make_unique<MelissaIncDecButton>();
        pitchButton_->onClick_= [this](bool inc, bool b)
        {
            const int sign = inc ? 1 : -1;
            melissa_->setPitch(melissa_->getPitch() + sign);
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
        browseToggleButton_->setToggleState(true, dontSendNotification);
        addAndMakeVisible(browseToggleButton_.get());
        
        setListToggleButton_ = make_unique<ToggleButton>();
        setListToggleButton_->setButtonText("Setlist");
        setListToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        setListToggleButton_->setRadioGroupId(kFileChooserTabGroup);
        setListToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_SetList); };
        setListToggleButton_->setToggleState(false, dontSendNotification);
        addAndMakeVisible(setListToggleButton_.get());
        
        recentToggleButton_ = make_unique<ToggleButton>();
        recentToggleButton_->setButtonText("Recent");
        recentToggleButton_->setLookAndFeel(&lookAndFeelTab_);
        recentToggleButton_->setRadioGroupId(kFileChooserTabGroup);
        recentToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Recent); };
        recentToggleButton_->setToggleState(false, dontSendNotification);
        addAndMakeVisible(recentToggleButton_.get());
    }
    
    {
        recentTable_ = make_unique<MelissaFileListBox>(this);
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
    }
    
    {
        addToListButton_ = make_unique<TextButton>();
        addToListButton_->setButtonText("Add");
        addToListButton_->onClick = [this]()
        {
            auto dialog = std::make_shared<MelissaInputDialog>(this, "Enter the name", "", [&](const String& text) {
                String name(text);
                if (name.isEmpty()) name = MelissaUtility::getFormattedTimeMSec(melissa_->getAPosMSec());
                addToPracticeList(name);
                if (auto songs = setting_["songs"].getArray())
                {
                    auto song = getSongSetting(fileFullPath_);
                    practiceTable_->setList(*(song.getProperty("list", Array<var>()).getArray()), melissa_->getTotalLengthMSec());
                    closeModalDialog();
                }
            });
            showModalDialog(std::dynamic_pointer_cast<Component>(dialog), "Add");
            
        };
        addAndMakeVisible(addToListButton_.get());
    }
    
    {
        wildCardFilter_ = make_unique<WildcardFileFilter>("*.mp3;*.wav;*.m4a", "*", "Music Files");
        fileBrowserComponent_ = make_unique<FileBrowserComponent>(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
                                                                  File::getSpecialLocation(File::userHomeDirectory),
                                                                  wildCardFilter_.get(),
                                                                  nullptr);
        fileBrowserComponent_->setColour(ListBox::backgroundColourId, Colours::transparentWhite);
        fileBrowserComponent_->setFilenameBoxLabel("File");
        fileBrowserComponent_->addListener(this);
        addAndMakeVisible(fileBrowserComponent_.get());
    }
    
    // Labels
    {
        const String labelTitles[kNumOfLabels] = {
#if defined(ENABLE_METRONOME)
            "Switch",
            "BPM",
            "Timing",
#endif
            "Volume",
            "Pitch",
            
            "Start Time",
            "End Time",
            
            "Speed",
#if defined(ENABLE_SPEEDTRAINER)
            "+",
            "Per",
            "Max",
#endif
        };
        for (size_t label_i = 0; label_i < kNumOfLabels; ++label_i)
        {
            auto l = make_unique<Label>();
            l->setLookAndFeel(nullptr);
            l->setText(labelTitles[label_i], dontSendNotification);
            l->setFont(Font(12));
            l->setColour(Label::textColourId, Colours::white.withAlpha(0.6f));
            l->setJustificationType(Justification::centredTop);
            addAndMakeVisible(l.get());
            labels_[label_i] = std::move(l);
        }
    }
    
    // Set List
    setListComponent_ = make_unique<MelissaSetListComponent>(this);
    addChildComponent(setListComponent_.get());
    
    for (size_t sectionTitle_i = 0; sectionTitle_i < kNumOfSectionTitles; ++sectionTitle_i)
    {
        const String titles_[kNumOfSectionTitles] =
        {
            "Settings", "A-B Loop",
#if defined(ENABLE_SPEEDTRAINER)
            "Speed",
#endif
#if defined(ENABLE_METRONOME)
            "Metronome",
#endif
        };
        auto sectionTitle = make_unique<MelissaSectionTitleComponent>(titles_[sectionTitle_i], 0.4f);
        addAndMakeVisible(sectionTitle.get());
        sectionTitles_[sectionTitle_i] = std::move(sectionTitle);
    }
    
    updatePracticeMemo(kPracticeMemoTab_Practice);
    updateFileChooserTab(kFileChooserTab_Browse);
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
    const int w = getWidth();
    const int center = w / 2;
    const auto gradationColour = MelissaColourScheme::BackGroundGradationColour();
    g.setGradientFill(ColourGradient(Colour(gradationColour.first), center, 0.f, Colour(gradationColour.second), center, getHeight(), false));
    g.fillAll();
    
    constexpr int interval = 6;
    bool offset = true;
    g.setColour(Colours::white.withAlpha(0.08f));
    for (int y_i = 0; y_i < getHeight(); y_i += interval)
    {
#if defined(SHOW_BOTTOM)
        if (y_i < controlComponent_->getY() || (controlComponent_->getBottom() <= y_i && y_i < bottomComponent_->getY()))
#else
        if (y_i < controlComponent_->getY() || (controlComponent_->getBottom() <= y_i))
#endif
        {
            for (int x_i = offset ? interval / 2 : 0; x_i < getWidth(); x_i += interval)
            {
                g.fillRect(x_i, y_i, 1, 1);
            }
        }
        offset = !offset;
    }
    
    Colour colours[] = { Colour(0x00000000), Colour(0x50000000) };
    
    int y = controlComponent_->getY() - kGradationHeight;
    g.setGradientFill(ColourGradient(colours[0], center, y, colours[1], center, y + kGradationHeight, false));
    g.fillRect(0, y, w, kGradationHeight);
    
    y = controlComponent_->getBottom();
    g.setGradientFill(ColourGradient(colours[1], center, y, colours[0], center, y + kGradationHeight, false));
    g.fillRect(0, y, w, kGradationHeight);
    
#if defined(SHOW_BOTTOM)
    y = bottomComponent_->getY() - kGradationHeight;
    g.setGradientFill(ColourGradient(colours[0], center, y, colours[1], center, y + kGradationHeight, false));
    g.fillRect(0, y, w, kGradationHeight);
#endif
}

void MainComponent::resized()
{
    menuButton_->setBounds(20, 20, 26, 14);
    
    waveformComponent_->setBounds(60, 20, getWidth() - 60 * 2, 200);
    
    controlComponent_->setBounds(0, 220, getWidth(), 180 /* 240 */);
    playPauseButton_->setBounds((getWidth() - 90) / 2, 240, 90, 90);
    toHeadButton_->setBounds(playPauseButton_->getX() - 60 , playPauseButton_->getY() + playPauseButton_->getHeight() / 2 - 15, 30, 30);
    
    fileNameLabel_->setBounds(getWidth() / 2 - 120, playPauseButton_->getBottom() + 10, 240, 20);
    timeLabel_->setBounds(getWidth() / 2 - 100, fileNameLabel_->getBottom(), 200, 20);
    
    
    // left-bottom part (Browser / SetList / Recent)
    {
        constexpr int32_t tabMargin = 2;
        int32_t browserWidth = 480;
        int32_t w = (browserWidth - tabMargin * (kNumOfFileChooserTabs - 1)) / kNumOfFileChooserTabs;
        int32_t y = controlComponent_->getBottom() + kGradationHeight - 10;
        
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
        
        y += 40;
        {
#if defined(SHOW_BOTTOM)
            const int32_t h = getHeight() - 40 - y;
#else
            const int32_t h = getHeight() - 40 - y + 20;
#endif
            fileBrowserComponent_->setBounds(20, y, browserWidth, h);
            setListComponent_->setBounds(20, y, browserWidth, h);
            recentTable_->setBounds(20, y, browserWidth, h);
        }
        
        {
#if defined(SHOW_BOTTOM)
            const int32_t h = getHeight() - 80 - y;
#else
            const int32_t h = getHeight() - 80 - y + 20;
#endif
            practiceTable_->setBounds(20 + browserWidth + 20, y, getWidth() - (20 + browserWidth) - 40, h);
            addToListButton_->setBounds(getWidth() - 80 - 20, practiceTable_->getBottom() + 10, 80, 30);
            memoTextEditor_->setBounds(20 + browserWidth + 20, y, getWidth() - (20 + browserWidth) - 40, h + 40);
        }
    }
    
#if defined(SHOW_BOTTOM)
    // Bottom
    bottomComponent_->setBounds(0, getHeight() - 30, getWidth(), 30);
#endif
    
    // Section
    int32_t marginX = 60;
    const int sectionWidth = toHeadButton_->getX() - (marginX * 2);
    for (auto&& sec : sectionTitles_)
    {
        sec->setSize(sectionWidth, 30);
    }
    
    int y = 260; // 260;
    
    // Song Settings
    sectionTitles_[kSectionTitle_Settings]->setTopLeftPosition(marginX, y);
    volumeSlider_->setSize(200, 30);
    pitchButton_->setSize(200, 30);
#if !defined(ENABLE_SPEEDTRAINER)
    arrangeEvenly({ marginX, y + 60, sectionWidth, 30 }, {
        { volumeSlider_.get() },
        { pitchButton_.get() }
    });
#endif
    
    // A-B Loop
    sectionTitles_[kSectionTitle_Loop]->setTopRightPosition(getWidth() - marginX, y);
    aSetButton_->setSize(80, 30);
    aButton_->setSize(200, 30);
    bSetButton_->setSize(80, 30);
    bButton_->setSize(200, 30);
    resetButton_->setSize(100, 30);
    arrangeEvenly({ sectionTitles_[kSectionTitle_Loop]->getX(), y + 60, sectionWidth, 30 }, {
        { aSetButton_.get(), aButton_.get() },
        { bSetButton_.get(), bButton_.get() },
        { resetButton_.get() }
    });
    
    // Metronome
#if defined(ENABLE_METRONOME)
    y = 360;
    sectionTitles_[kSectionTitle_Metronome]->setTopLeftPosition(marginX, y);
    metronomeOnOffButton_->setSize(80, 30);
    bpmButton_->setSize(140, 30);
    metronomeOffsetButton_->setSize(140, 30);
    analyzeButton_->setSize(80, 30);
    arrangeEvenly({ marginX, y + 60, sectionWidth, 30 }, {
        { metronomeOnOffButton_.get() },
        { bpmButton_.get() },
        { metronomeOffsetButton_.get() },
        { analyzeButton_.get()}
    });
#endif
    
#if defined(ENABLE_SPEEDTRAINER)
    // Speed
    sectionTitles_[kSectionTitle_Speed]->setTopRightPosition(getWidth() - marginX, y);
    speedButton_->setSize(140, 30);
    speedIncValueButton_->setSize(140, 30);
    speedIncPerButton_->setSize(140, 30);
    speedIncMaxButton_->setSize(140, 30);
    arrangeEvenly({ sectionTitles_[kSectionTitle_Speed]->getX(), y + 60, sectionWidth, 30 }, {
        { speedButton_.get() },
        { speedIncValueButton_.get(), speedIncPerButton_.get(), speedIncMaxButton_.get() }
    });
#else
    speedButton_->setSize(200, 30);
    arrangeEvenly({ marginX, y + 60, sectionWidth, 30 }, {
        { volumeSlider_.get() },
        { pitchButton_.get() },
        { speedButton_.get() },
    });
#endif
    
    // Labels
    const Component* components[kNumOfLabels]
    {
#if defined(ENABLE_METRONOME)
        dynamic_cast<Component*>(metronomeOnOffButton_.get()),
        dynamic_cast<Component*>(bpmButton_.get()),
        dynamic_cast<Component*>(metronomeOffsetButton_.get()),
#endif
        dynamic_cast<Component*>(volumeSlider_.get()),
        dynamic_cast<Component*>(pitchButton_.get()),
        
        dynamic_cast<Component*>(aButton_.get()),
        dynamic_cast<Component*>(bButton_.get()),
        
        dynamic_cast<Component*>(speedButton_.get()),
#if defined(ENABLE_SPEEDTRAINER)
        dynamic_cast<Component*>(speedIncValueButton_.get()),
        dynamic_cast<Component*>(speedIncPerButton_.get()),
        dynamic_cast<Component*>(speedIncMaxButton_.get()),
#endif
    };
    
    for (size_t label_i = 0; label_i < kNumOfLabels; ++label_i )
    {
        if (label_i == kLabel_ATime || label_i == kLabel_BTime) continue;
        auto b = components[label_i]->getBoundsInParent();
        labels_[label_i]->setBounds(b.getX(), b.getY() - 20, b.getWidth(), 20);
    }
    labels_[kLabel_ATime]->setBounds(aSetButton_->getX(), aSetButton_->getY() - 20, aButton_->getRight() - aSetButton_->getX(), 20);
    labels_[kLabel_BTime]->setBounds(bSetButton_->getX(), bSetButton_->getY() - 20, bButton_->getRight() - bSetButton_->getX(), 20);
    
    for (auto&& tie : tie_) tie->updatePosition();
    
    if (modalDialog_ != nullptr) modalDialog_->setSize(getWidth(), getHeight());
}

bool MainComponent::isInterestedInFileDrag(const StringArray& files)
{
    return true;
}

void MainComponent::filesDropped(const StringArray& files, int x, int y)
{
    for (auto&& file : files)
    {
        if (loadFile(file)) break;
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

void MainComponent::setMelissaParameters(float aRatio, float bRatio, float speed)
{
    melissa_->setAPosRatio(aRatio);
    melissa_->setBPosRatio(bRatio);
    melissa_->setSpeed(speed);
    updateAll();
}

void MainComponent::getMelissaParameters(float* aRatio, float* bRatio, float* speed)
{
    *aRatio = melissa_->getAPosRatio();
    *bRatio = melissa_->getBPosRatio();
    *speed  = melissa_->getSpeed();
}

void MainComponent::updatePracticeList(const Array<var>& list)
{
    if (auto songs = setting_["songs"].getArray())
    {
        auto song = getSongSetting(fileFullPath_);
        song.getDynamicObject()->setProperty("list", list);
    }
}

void MainComponent::createSetlist(const String& name)
{
    setListComponent_->add(name, true);
}

bool MainComponent::loadFile(const String& filePath)
{
    return openFile(File(filePath));
}

void MainComponent::showModalDialog(std::shared_ptr<Component> component, const String& title)
{
    closeModalDialog();
    
    modalDialog_ = std::make_unique<MelissaModalDialog>(this, component, title);
    modalDialog_->setSize(getWidth(), getHeight());
    addAndMakeVisible(modalDialog_.get());
}

void MainComponent::showPreferencesDialog()
{
    auto component = std::make_shared<MelissaPreferencesComponent>(&deviceManager);
    showModalDialog(std::dynamic_pointer_cast<Component>(component), "Preferences");
}

void MainComponent::showAboutDialog()
{
    auto component = std::make_shared<MelissaAboutComponent>();
    showModalDialog(std::dynamic_pointer_cast<Component>(component), "About Melissa");
}

void MainComponent::closeModalDialog()
{
    if (modalDialog_ == nullptr) return;
    
    removeChildComponent(modalDialog_.get());
    modalDialog_ = nullptr;
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

StringArray MainComponent::getMenuBarNames()
{
    return { "File" };
}

PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
    PopupMenu menu;
    
    if (topLevelMenuIndex == 0)
    {
        menu.addItem(kMenuID_FileOpen, "Open");
    }
    
    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (menuItemID == kMenuID_FileOpen)
    {
        fileChooser_ = std::make_unique<FileChooser>("Choose a file to add to this set list...", File::getCurrentWorkingDirectory(), "*.mp3;*.wav;*.m4a", true);
        fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [&] (const FileChooser& chooser) {
            auto fileUrl = chooser.getURLResult();
            if (fileUrl.isLocalFile())
            {
                openFile(File(fileUrl.getLocalFile().getFullPathName()));
            }
        });
    }
}

void MainComponent::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
    midiControlManager_.processMIDIMessage(message);
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
    setListComponent_->setVisible(tab == kFileChooserTab_SetList);
    recentTable_->setVisible(tab == kFileChooserTab_Recent);
}

void MainComponent::updatePracticeMemo(PracticeMemoTab tab)
{
    practiceTable_->setVisible(tab == kPracticeMemoTab_Practice);
    addToListButton_->setVisible(tab == kPracticeMemoTab_Practice);
    memoTextEditor_->setVisible(tab == kPracticeMemoTab_Memo);
}

bool MainComponent::openFile(const File& file)
{
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return false;
    
    // read audio data from reader
    const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
    audioSampleBuf_ = std::make_shared<AudioSampleBuffer>(2, lengthInSamples);
    reader->read(audioSampleBuf_.get(), 0, lengthInSamples, 0, true, true);
    
    melissa_->reset();
    const float* buffer[] = { audioSampleBuf_->getReadPointer(0), audioSampleBuf_->getReadPointer(1) };
    melissa_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    waveformComponent_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    audioSampleBuf_ = nullptr;
    
    fileName_ = file.getFileNameWithoutExtension();
    fileFullPath_ = file.getFullPathName();
    fileNameLabel_->setText(fileName_.toStdString());
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
    song->setProperty("pitch", melissa_->getPitch());
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
    song->setProperty("pitch", melissa_->getPitch());
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
    recentTable_->selectRow(0);
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
    
#if defined(ENABLE_SPEEDTRAINER)
    const int32_t incPer = melissa_->getSpeedIncPer();
    speedIncPerButton_->setText(String(incPer));
    
    const int32_t incValue = melissa_->getSpeedIncValue();
    speedIncValueButton_->setText(String(incValue) + " %");
    
    const int32_t incMax = melissa_->getSpeedIncMax();
    speedIncMaxButton_->setText(String(incMax) + " %");
#endif
}

void MainComponent::updatePitchButtonLabel()
{
    pitchButton_->setText(MelissaUtility::getFormattedPitch(melissa_->getPitch()));
}

void MainComponent::updateBpm()
{
#if defined(ENABLE_SPEEDTRAINER)
    bpmButton_->setText(String(static_cast<uint32_t>(melissa_->getBpm())));
#endif
}

void MainComponent::updateMetronomeOffset()
{
#if defined(ENABLE_SPEEDTRAINER)
    metronomeOffsetButton_->setText(MelissaUtility::getFormattedTimeMSec(melissa_->getMetronomeOffsetSec() * 1000.f));
#endif
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
#if defined(ENABLE_METRONOME)
    current->setProperty("bpm", static_cast<float>(melissa_->getBpm()));
    current->setProperty("metronome_offset", melissa_->getMetronomeOffsetSec());
#endif
    current->setProperty("a", melissa_->getAPosRatio());
    current->setProperty("b", melissa_->getBPosRatio());
    current->setProperty("speed", melissa_->getSpeed());
    current->setProperty("pitch", melissa_->getPitch());
    all->setProperty("current", var(current));
    all->setProperty("recent", Array<var>());
    all->setProperty("setlist", Array<var>());
    all->setProperty("songs", Array<var>());
    
    settingsFile_.replaceWithText(JSON::toString(all));
}

void MainComponent::saveSettings()
{
    saveMemo();
    
    auto global = setting_["global"].getDynamicObject();
    global->setProperty("version", "1.0");
    global->setProperty("root_dir", fileBrowserComponent_->getRoot().getFullPathName());
    global->setProperty("width", getWidth());
    global->setProperty("height", getHeight());
    
    auto xml = deviceManager.createStateXml();
    if (xml != nullptr) global->setProperty("device", xml->toString());
    
    auto current = setting_["current"].getDynamicObject();
    current->setProperty("file", fileFullPath_);
    current->setProperty("volume", melissa_->getVolume());
#if defined(ENABLE_METRONOME)
    current->setProperty("bpm", static_cast<float>(melissa_->getBpm()));
    current->setProperty("metronome_offset", melissa_->getMetronomeOffsetSec());
#endif
    current->setProperty("a", melissa_->getAPosRatio());
    current->setProperty("b", melissa_->getBPosRatio());
    current->setProperty("speed", melissa_->getSpeed());
    current->setProperty("pitch", melissa_->getPitch());
    
    setting_.getDynamicObject()->setProperty("setlist", setListComponent_->getData());
    
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

void MainComponent::arrangeEvenly(const Rectangle<int> bounds, const std::vector<std::vector<Component*>>& components_, float widthRatio)
{
    // measure
    const int marginNarrow = 10 * widthRatio;
    int numOfGroups = 0;
    int totalWidthOfAllComponents = 0;
    int numOfComponents = 0;
    for (auto&& group : components_)
    {
        numOfGroups += group.size() - 1;
        for (auto&& component : group)
        {
            totalWidthOfAllComponents += component->getWidth() * widthRatio;
            ++numOfComponents;
        }
    }
    const float marginWide = (bounds.getWidth() - totalWidthOfAllComponents - marginNarrow * numOfGroups) / static_cast<float>(components_.size() - 1);
    
    // arrange
    if (marginWide < marginNarrow && 0.5f <= widthRatio)
    {
        arrangeEvenly(bounds, components_, widthRatio - 0.05);
    }
    else
    {
        float x = bounds.getX(), y = bounds.getY();
        for (auto&& group : components_)
        {
            for (auto&& component : group)
            {
                component->setTopLeftPosition(x, y);
                component->setSize(component->getWidth() * widthRatio, component->getHeight());
                x += component->getWidth();
                x += marginNarrow;
            }
            x += (marginWide - marginNarrow);
        }
    }
}
