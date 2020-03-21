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
    kMenuID_MainVersionCheck = 1001,
    kMenuID_MainPreferences = 1002,
    kMenuID_MainTutorial = 1003,
    kMenuID_FileOpen = 2000,
};

MainComponent::MainComponent() : Thread("MelissaProcessThread"),
status_(kStatus_Stop), shouldExit_(false)
{
    model_ = MelissaModel::getInstance();
    
    melissa_ = std::make_unique<Melissa>();
    model_->setMelissa(melissa_.get());
    model_->addListener(dynamic_cast<MelissaModelListener*>(melissa_.get()));    
    model_->addListener(this);
    
    getLookAndFeel().setDefaultSansSerifTypeface(Typeface::createSystemTypefaceFor(BinaryData::NotoSansCJKjpRegular_otf, BinaryData::NotoSansCJKjpRegular_otfSize));
    
    String localizedStrings = "";
    if (SystemStats::getDisplayLanguage() == "ja-JP")
    {
#ifdef DEBUG
        File file("../../../../Resource/Language/ja-JP.txt");
        if (file.exists())
        {
            localizedStrings = file.loadFileAsString();
        }
        else
#endif
        {
            localizedStrings = String::createStringFromData(BinaryData::jaJP_txt, BinaryData::jaJP_txtSize);
        }
    }
    else
    {
#ifdef DEBUG
        File file("../../../../Resource/Language/en-US.txt");
        if (file.exists())
        {
            localizedStrings = file.loadFileAsString();
        }
        else
#endif
        {
            localizedStrings = String::createStringFromData(BinaryData::enUS_txt, BinaryData::enUS_txtSize);
        }
    }
    LocalisedStrings::setCurrentMappings(new LocalisedStrings(localizedStrings, false));
    
    createUI();

    
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
    
    bool isFirstLaunch = false;
    {
        // load setting file
        settingsDir_ = (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Melissa"));
        if (!(settingsDir_.exists() && settingsDir_.isDirectory())) settingsDir_.createDirectory();
        
        settingsFile_ = settingsDir_.getChildFile("Settings.json");
        if (!settingsFile_.existsAsFile())
        {
            isFirstLaunch = true;
            createSettingsFile();
        }
        
        setting_ = JSON::parse(settingsFile_.loadFileAsString());
        if (!isSettingValid())
        {
            isFirstLaunch = true;
            createSettingsFile();
            setting_ = JSON::parse(settingsFile_.loadFileAsString());
        }
        
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
        
        if (setting_.hasProperty("recent"))
        {
            recent_ = setting_["recent"].getArray();
        }
        else
        {
            recent_ = new Array<var>();
        }
        recentTable_->setList(*recent_);
        
        if (setting_.hasProperty("playlist"))
        {
            setList_ = setting_["playlist"].getArray();
            setListComponent_->setData(*setList_);
        }
        
        if (setting_.hasProperty("current"))
        {
            auto current = setting_["current"];
            File file(current["file"].toString());
            if (file.existsAsFile())
            {
                openFile(file);
                
                model_->setVolume(static_cast<float>(current.getProperty("volume", 1.f)));
                model_->setLoopAPosRatio(static_cast<float>(current.getProperty("a", 0.f)));
                model_->setLoopBPosRatio(static_cast<float>(current.getProperty("b", 1.f)));
                model_->setSpeed(static_cast<float>(current.getProperty("speed", 100)));
                model_->setPitch(static_cast<float>(current.getProperty("pitch", 0.f)));
                updateAll();
            }
        }
    }
    
    deviceManager.addMidiInputCallback("", this);
    
    if (isFirstLaunch)
    {
        auto component = std::make_shared<MelissaOkCancelDialog>(this, TRANS("first_message"), [&](){
            showTutorial();
        });
        showModalDialog(component, TRANS("tutorial"));
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
    practiceTable_->setLookAndFeel(nullptr);
    tooltipWindow_->setLookAndFeel(nullptr);
    
#if JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr);
#endif
    
    stopThread(4000.f);
    stopTimer();
}

void MainComponent::createUI()
{
    setLookAndFeel(&lookAndFeel_);
    
    tooltipWindow_ = std::make_unique<TooltipWindow>(this, 0);
    tooltipWindow_->setLookAndFeel(&lookAndFeel_);
    
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
        menu.setLookAndFeel(&lookAndFeel_);
        menu.addItem(kMenuID_MainAbout, TRANS("about_melissa"));
        menu.addItem(kMenuID_MainVersionCheck, TRANS("check_update"));
        menu.addItem(kMenuID_MainPreferences, TRANS("preferences"));
        menu.addItem(kMenuID_MainTutorial, TRANS("tutorial"));
        const auto result = menu.show();
        if (result == kMenuID_MainAbout)
        {
            showAboutDialog();
        }
        else if (result == kMenuID_MainVersionCheck)
        {
            showUpdateDialog(true);
        }
        else if (result == kMenuID_MainPreferences)
        {
            showPreferencesDialog();
        }
        else if (result == kMenuID_MainTutorial)
        {
            showTutorial();
        }
    };
    addAndMakeVisible(menuButton_.get());
    
    waveformComponent_ = make_unique<MelissaWaveformControlComponent>();
    addAndMakeVisible(waveformComponent_.get());
    
    controlComponent_ = make_unique<Label>();
    controlComponent_->setOpaque(false);
    controlComponent_->setColour(Label::backgroundColourId, Colour(MelissaColourScheme::MainColour()).withAlpha(0.06f));
    addAndMakeVisible(controlComponent_.get());
    
#if defined(SHOW_BOTTOM)
    bottomComponent_ = make_unique<MelissaBottomControlComponent>();
    lookAndFeel_.setBottomComponent(bottomComponent_.get());
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
    timeLabel_->setFont(22);
    addAndMakeVisible(timeLabel_.get());
    
    fileNameLabel_ = make_unique<MelissaScrollLabel>(timeLabel_->getFont());
    addAndMakeVisible(fileNameLabel_.get());
    
#if defined(ENABLE_METRONOME)
    metronomeOnOffButton_ = make_unique<ToggleButton>();
    metronomeOnOffButton_->setClickingTogglesState(true);
    metronomeOnOffButton_->onClick = [this]()
    {
        model_->setMetoronome(metronomeOnOffButton_->getToggleState());
    };
    metronomeOnOffButton_->setButtonText("On");
    addAndMakeVisible(metronomeOnOffButton_.get());
    
    bpmButton_ = make_unique<MelissaIncDecButton>();
    bpmButton_->onClick_ = [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setBpm(model_->getBpm() + sign * (b ? 10 : 1));
        updateBpm();
    };
    addAndMakeVisible(bpmButton_.get());
    
    metronomeOffsetButton_ = make_unique<MelissaIncDecButton>();
    metronomeOffsetButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setMetronomeOffsetSec(model_->getMetronomeOffsetSec() + sign);
        updateMetronomeOffset();
    };
    addAndMakeVisible(metronomeOffsetButton_.get());
    
    analyzeButton_ = make_unique<TextButton>();
    analyzeButton_->setButtonText("Analyze");
    analyzeButton_->onClick = [this]()
    {
        model_->analyzeBpm();
        updateBpm();
        updateMetronomeOffset();
    };
    addAndMakeVisible(analyzeButton_.get());
#endif
    
    volumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
    volumeSlider_->setRange(0.01f, 2.0f);
    volumeSlider_->setValue(0.f);
    volumeSlider_->onValueChange = [this]()
    {
        model_->setVolume(volumeSlider_->getValue());
    };
    addAndMakeVisible(volumeSlider_.get());

    aButton_ = make_unique<MelissaIncDecButton>(TRANS("tooltip_loop_start_dec"), TRANS("tooltip_loop_start_inc"));
    aButton_->setText("-:--");
    aButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setLoopAPosMSec(model_->getLoopAPosMSec() + sign * (b ? 100 : 1000));
        updateAButtonLabel();
    };
    addAndMakeVisible(aButton_.get());

    aSetButton_ = make_unique<TextButton>();
    aSetButton_->setButtonText("A");
    aSetButton_->setTooltip(TRANS("tooltip_loop_start_set"));
    aSetButton_->onClick = [this]()
    {
        model_->setLoopAPosMSec(model_->getPlayingPosMSec());
        updateAButtonLabel();
    };
    addAndMakeVisible(aSetButton_.get());

    bButton_ = make_unique<MelissaIncDecButton>(TRANS("tooltip_loop_end_dec"), TRANS("tooltip_loop_end_inc"));
    bButton_->setText("-:--");
    bButton_->setBounds(0, 240, 140, 34);
    bButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setLoopBPosMSec(model_->getLoopBPosMSec() + sign * (b ? 100 : 1000));
        updateBButtonLabel();
    };
    addAndMakeVisible(bButton_.get());

    bSetButton_ = make_unique<TextButton>();
    bSetButton_->setButtonText("B");
    bSetButton_->setTooltip(TRANS("tooltip_loop_end_set"));
    bSetButton_->onClick = [this]()
    {
        model_->setLoopBPosMSec(model_->getPlayingPosMSec());
        updateBButtonLabel();
    };
    addAndMakeVisible(bSetButton_.get());

    resetButton_ = make_unique<TextButton>();
    resetButton_->setButtonText("Reset");
    resetButton_->setTooltip(TRANS("tooltip_loop_reset"));
    resetButton_->onClick = [this]() { resetLoop(); };
    addAndMakeVisible(resetButton_.get());

    tie_[0] = std::make_unique<MelissaTieComponent>(aSetButton_.get(), aButton_.get());
    addAndMakeVisible(tie_[0].get());
    tie_[1] = std::make_unique<MelissaTieComponent>(bSetButton_.get(), bButton_.get());
    addAndMakeVisible(tie_[1].get());

    speedButton_ = make_unique<MelissaIncDecButton>(TRANS("tooltip_speed_dec"), TRANS("tooltip_speed_inc"));
    speedButton_->setText("100 %");
    speedButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setSpeed(model_->getSpeed() + sign * (b ? 1 : 10));
        updateSpeedButtonLabel();
    };
    speedButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
    addAndMakeVisible(speedButton_.get());
    
#if defined(ENABLE_SPEEDTRAINER)
    speedIncPerButton_ = make_unique<MelissaIncDecButton>();
    speedIncPerButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setSpeedIncPer(model_->getSpeedIncPer() + sign);
        updateSpeedButtonLabel();
    };
    addAndMakeVisible(speedIncPerButton_.get());
    
    speedIncValueButton_ = make_unique<MelissaIncDecButton>();
    speedIncValueButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setSpeedIncValue(model_->getSpeedIncValue() + sign);
        updateSpeedButtonLabel();
    };
    addAndMakeVisible(speedIncValueButton_.get());
    
    speedIncMaxButton_ = make_unique<MelissaIncDecButton>();
    speedIncMaxButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setSpeedIncMax(model_->getSpeedIncMax() + sign);
        updateSpeedButtonLabel();
    };
    addAndMakeVisible(speedIncMaxButton_.get());
#endif
    
    pitchButton_ = make_unique<MelissaIncDecButton>(TRANS("tooltip_pitch_dec"), TRANS("tooltip_pitch_inc"));
    pitchButton_->setText("Original");
    pitchButton_->onClick_= [this](bool inc, bool b)
    {
        const int sign = inc ? 1 : -1;
        model_->setPitch(model_->getPitch() + sign);
        updatePitchButtonLabel();
    };
    pitchButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
    addAndMakeVisible(pitchButton_.get());
    
    browseToggleButton_ = make_unique<ToggleButton>();
    browseToggleButton_->setButtonText("Browse");
    browseToggleButton_->setLookAndFeel(&lookAndFeelTab_);
    browseToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    browseToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Browse); };
    browseToggleButton_->setToggleState(true, dontSendNotification);
    addAndMakeVisible(browseToggleButton_.get());
    
    setListToggleButton_ = make_unique<ToggleButton>();
    setListToggleButton_->setButtonText("Playlist");
    setListToggleButton_->setLookAndFeel(&lookAndFeelTab_);
    setListToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    setListToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Playlist); };
    setListToggleButton_->setToggleState(false, dontSendNotification);
    addAndMakeVisible(setListToggleButton_.get());
    
    recentToggleButton_ = make_unique<ToggleButton>();
    recentToggleButton_->setButtonText("Recent");
    recentToggleButton_->setLookAndFeel(&lookAndFeelTab_);
    recentToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    recentToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Recent); };
    recentToggleButton_->setToggleState(false, dontSendNotification);
    addAndMakeVisible(recentToggleButton_.get());

    recentTable_ = make_unique<MelissaFileListBox>(this);
    recentTable_->setLookAndFeel(&lookAndFeel_);
    addAndMakeVisible(recentTable_.get());

    practiceTable_ = make_unique<MelissaPracticeTableListBox>(this);
    addAndMakeVisible(practiceTable_.get());

    memoTextEditor_ = make_unique<TextEditor>();
    memoTextEditor_->setLookAndFeel(nullptr);
    memoTextEditor_->setFont(Font(22));
    memoTextEditor_->setMultiLine(true, false);
    memoTextEditor_->setLookAndFeel(&lookAndFeelMemo_);
    memoTextEditor_->onFocusLost = [&]()
    {
        saveMemo();
    };
    memoTextEditor_->setReturnKeyStartsNewLine(true);
    addAndMakeVisible(memoTextEditor_.get());

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

    addToListButton_ = make_unique<TextButton>();
    addToListButton_->setButtonText("Add");
    addToListButton_->onClick = [this]()
    {
        const String defaultName = MelissaUtility::getFormattedTimeSec(model_->getLoopAPosMSec() / 1000.f) + " - " + MelissaUtility::getFormattedTimeSec(model_->getLoopBPosMSec() / 1000.f);
        auto dialog = std::make_shared<MelissaInputDialog>(this, TRANS("enter_loop_name"), defaultName, [&](const String& text) {
            String name(text);
            if (name.isEmpty()) name = defaultName;
            addToPracticeList(name);
            if (auto songs = setting_["songs"].getArray())
            {
                auto song = getSongSetting(fileFullPath_);
                practiceTable_->setList(*(song.getProperty("list", Array<var>()).getArray()), model_->getLengthMSec());
                closeModalDialog();
            }
        });
        showModalDialog(std::dynamic_pointer_cast<Component>(dialog), TRANS("add_practice_list"));
        
    };
    addAndMakeVisible(addToListButton_.get());

    wildCardFilter_ = make_unique<WildcardFileFilter>("*.mp3;*.wav;*.m4a;*.flac;*.ogg;*.w", "*", "Music Files");
    fileBrowserComponent_ = make_unique<FileBrowserComponent>(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
                                                              File::getSpecialLocation(File::userHomeDirectory),
                                                              wildCardFilter_.get(),
                                                              nullptr);
    fileBrowserComponent_->setColour(ListBox::backgroundColourId, Colours::transparentWhite);
    fileBrowserComponent_->addListener(this);
    addAndMakeVisible(fileBrowserComponent_.get());
    
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
            
            "Start",
            "End",
            
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
            l->setFont(Font(20));
            l->setColour(Label::textColourId, Colours::white.withAlpha(0.6f));
            l->setJustificationType(Justification::centredTop);
            addAndMakeVisible(l.get());
            labels_[label_i] = std::move(l);
        }
    }
    
    // Set List
    setListComponent_ = make_unique<MelissaPlaylistComponent>(this);
    addChildComponent(setListComponent_.get());
    
    for (size_t sectionTitle_i = 0; sectionTitle_i < kNumOfSectionTitles; ++sectionTitle_i)
    {
        const String titles_[kNumOfSectionTitles] =
        {
            "Settings", "Loop",
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
    
    if (model_ == nullptr || status_ != kStatus_Playing) return;
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
    
    
    // left-bottom part (Browser / Playlist / Recent)
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
        { aSetButton_.get() },
        { aButton_.get() },
        { bSetButton_.get() },
        { bButton_.get() },
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
        labels_[label_i]->setBounds(b.getX(), b.getY() - 24, b.getWidth(), 24);
    }
    labels_[kLabel_ATime]->setBounds(aSetButton_->getX(), aSetButton_->getY() - 24, aButton_->getRight() - aSetButton_->getX(), 24);
    labels_[kLabel_BTime]->setBounds(bSetButton_->getX(), bSetButton_->getY() - 24, bButton_->getRight() - bSetButton_->getX(), 24);
    
    for (auto&& tie : tie_) tie->updatePosition();
    
    if (modalDialog_ != nullptr) modalDialog_->setSize(getWidth(), getHeight());
    if (tutorialComponent_ != nullptr) tutorialComponent_->setBounds(0, 0, getWidth(), getHeight());
}

bool MainComponent::isInterestedInFileDrag(const StringArray& files)
{
    return true;
}

void MainComponent::filesDropped(const StringArray& files, int x, int y)
{
    if (files.size() == 1)
    {
        loadFile(files[0]);
    }
    else
    {
        showModalDialog(std::make_shared<MelissaInputDialog>(this, TRANS("detect_multifiles_drop"),  "new playlist", [&, files](const String& playlistName) {
            createPlaylist(playlistName);
            for (auto&& file : files) setListComponent_->addToPlaylist(file);
            loadFile(files[0]);
            closeModalDialog();
        }), TRANS("new_playlist"));
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
        auto currentMSec = model_->getPlayingPosMSec();
        model_->setPlayingPosMSec(currentMSec - 1000);
        return true;
    }
    else if (keyCode == 63235)
    {
        auto currentMSec = model_->getPlayingPosMSec();
        model_->setPlayingPosMSec(currentMSec + 1000);
        return true;
    }
    
    return false;
}

void MainComponent::updatePracticeList(const Array<var>& list)
{
    if (auto songs = setting_["songs"].getArray())
    {
        auto song = getSongSetting(fileFullPath_);
        song.getDynamicObject()->setProperty("list", list);
    }
}

void MainComponent::createPlaylist(const String& name)
{
    setListComponent_->createPlaylist(name, true);
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
    showModalDialog(std::dynamic_pointer_cast<Component>(component), TRANS("preferences"));
}

void MainComponent::showAboutDialog()
{
    auto component = std::make_shared<MelissaAboutComponent>();
    showModalDialog(std::dynamic_pointer_cast<Component>(component), TRANS("about_melissa"));
}

void MainComponent::showTutorial()
{
    tutorialComponent_ = std::make_unique<MelissaTutorialComponent>(this);
    tutorialComponent_->setPages({
        { dynamic_cast<Component*>(menuButton_.get()), TRANS("explanation_menu") },
        { dynamic_cast<Component*>(waveformComponent_.get()), TRANS("explanation_waveform") },
        { dynamic_cast<Component*>(volumeSlider_.get()), TRANS("explanation_volume") },
        { dynamic_cast<Component*>(pitchButton_.get()), TRANS("explanation_pitch") },
        { dynamic_cast<Component*>(speedButton_.get()), TRANS("explanation_speed") },
        { dynamic_cast<Component*>(fileBrowserComponent_.get()), TRANS("explanation_browser") },
    });
    addAndMakeVisible(tutorialComponent_.get());
    resized();
}

void MainComponent::showUpdateDialog(bool showIfThereIsNoUpdate)
{
    const String latestVersionNumberString = MelissaUpdateChecker::getLatestVersionNumberString();
    
    if (latestVersionNumberString == (String("v") + ProjectInfo::versionString))
    {
        if (showIfThereIsNoUpdate)
        {
            NativeMessageBox::showMessageBox(AlertWindow::NoIcon, TRANS("update"), TRANS("there_is_no_update"), this);
        }
    }
    else if (latestVersionNumberString == "")
    {
        if (showIfThereIsNoUpdate)
        {
            NativeMessageBox::showMessageBox(AlertWindow::NoIcon, TRANS("update"), TRANS("failed_to_get_update"), this);
        }
    }
    else
    {
        const auto result = NativeMessageBox::showYesNoBox(AlertWindow::NoIcon, TRANS("update"), TRANS("there_is_update"), this);
        if (result == 1)
        {
            URL("https://github.com/mosynthkey/Melissa/releases").launchInDefaultBrowser();
        }
    }
}

void MainComponent::closeModalDialog()
{
    if (modalDialog_ == nullptr) return;
    
    removeChildComponent(modalDialog_.get());
    modalDialog_ = nullptr;
}

void MainComponent::closeTutorial()
{
    tutorialComponent_ = nullptr;
}

void MainComponent::fileDoubleClicked(const File& file)
{
    openFile(file);
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
        fileChooser_ = std::make_unique<FileChooser>("Open", File::getCurrentWorkingDirectory(), "*.mp3;*.wav;*.m4a", true);
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
        if (model_ != nullptr && melissa_->isBufferSet() && melissa_->needToProcess())
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
    if (model_ == nullptr) return;
    
    timeLabel_->setText(MelissaUtility::getFormattedTimeMSec(model_->getPlayingPosMSec()), dontSendNotification);
    waveformComponent_->setPlayPosition(model_->getPlayingPosRatio());
}

void MainComponent::updateFileChooserTab(FileChooserTab tab)
{
    fileBrowserComponent_->setVisible(tab == kFileChooserTab_Browse);
    setListComponent_->setVisible(tab == kFileChooserTab_Playlist);
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
    pause();
    toHead();
    
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
    model_->setLengthMSec(lengthInSamples / reader->sampleRate * 1000.f);
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
        practiceTable_->setList(*(song.getProperty("list", Array<var>()).getArray()), model_->getLengthMSec());
        memoTextEditor_->setText(song.getProperty("memo", "").toString());
    }
    
    addToRecent(fileFullPath_);
    recentTable_->setList(*(setting_.getProperty("recent", Array<var>()).getArray()));
    
    delete reader;
    
    return true;
}

void MainComponent::play()
{
    if (model_ == nullptr) return;
    status_ = kStatus_Playing;
    playPauseButton_->setMode(MelissaPlayPauseButton::kMode_Pause);
}

void MainComponent::pause()
{
    if (model_ == nullptr) return;
    status_ = kStatus_Pause;
    playPauseButton_->setMode(MelissaPlayPauseButton::kMode_Play);
}

void MainComponent::stop()
{
    if (model_ == nullptr) return;
    status_ = kStatus_Stop;
    model_->setPlayingPosMSec(0);
    playPauseButton_->setMode(MelissaPlayPauseButton::kMode_Play);
}

void MainComponent::toHead()
{
    if (model_ == nullptr) return;
    model_->setPlayingPosRatio(model_->getLoopAPosRatio());
}

void MainComponent::resetLoop()
{
    model_->setLoopPosRatio(0.f, 1.f);
    updateAButtonLabel();
    updateBButtonLabel();
}

void MainComponent::addToPracticeList(String name)
{
    auto addToList = [this, name](Array<var>* list)
    {
        auto prac = new DynamicObject();
        prac->setProperty("name", name);
        prac->setProperty("a", model_->getLoopAPosRatio());
        prac->setProperty("b", model_->getLoopBPosRatio());
        prac->setProperty("speed", model_->getSpeed());
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
    song->setProperty("volume", model_->getVolume());
#if defined(ENABLE_METRONOME)
    song->setProperty("bpm", static_cast<float>(model_->getBpm()));
    song->setProperty("metronome_offset", model_->getMetronomeOffsetSec());
#endif
    song->setProperty("pitch", model_->getPitch());
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
    song->setProperty("volume", model_->getVolume());
#if defined(ENABLE_METRONOME)
    song->setProperty("bpm", static_cast<float>(model_->getBpm()));
    song->setProperty("metronome_offset", model_->getMetronomeOffsetSec());
#endif
    song->setProperty("pitch", model_->getPitch());
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
    updateVolume();
}

void MainComponent::updateAButtonLabel()
{
    const auto LoopAPosMSec = model_->getLoopAPosMSec();
    aButton_->setText(MelissaUtility::getFormattedTimeMSec(LoopAPosMSec));
}

void MainComponent::updateBButtonLabel()
{
    const auto LoopBPosMSec = model_->getLoopBPosMSec();
    bButton_->setText(MelissaUtility::getFormattedTimeMSec(LoopBPosMSec));
}

void MainComponent::updateSpeedButtonLabel()
{
    const int32_t speed = model_->getSpeed();
    speedButton_->setText(String(speed) + "%");
    
#if defined(ENABLE_SPEEDTRAINER)
    const int32_t incPer = model_->getSpeedIncPer();
    speedIncPerButton_->setText(String(incPer));
    
    const int32_t incValue = model_->getSpeedIncValue();
    speedIncValueButton_->setText(String(incValue) + " %");
    
    const int32_t incMax = model_->getSpeedIncMax();
    speedIncMaxButton_->setText(String(incMax) + " %");
#endif
}

void MainComponent::updatePitchButtonLabel()
{
    pitchButton_->setText(MelissaUtility::getFormattedPitch(model_->getPitch()));
}

void MainComponent::updateBpm()
{
#if defined(ENABLE_SPEEDTRAINER)
    bpmButton_->setText(String(static_cast<uint32_t>(model_->getBpm())));
#endif
}

void MainComponent::updateMetronomeOffset()
{
#if defined(ENABLE_SPEEDTRAINER)
    metronomeOffsetButton_->setText(MelissaUtility::getFormattedTimeMSec(model_->getMetronomeOffsetSec() * 1000.f));
#endif
}

void MainComponent::updateVolume()
{
    volumeSlider_->setValue(model_->getVolume());
}

void MainComponent::createSettingsFile()
{
    auto all = new DynamicObject();
    
    auto global = new DynamicObject();
    global->setProperty("version", 1);
    all->setProperty("global", var(global));
    
    auto current = new DynamicObject();
    current->setProperty("file", fileFullPath_);
    current->setProperty("volume", model_->getVolume());
#if defined(ENABLE_METRONOME)
    current->setProperty("bpm", static_cast<float>(model_->getBpm()));
    current->setProperty("metronome_offset", model_->getMetronomeOffsetSec());
#endif
    current->setProperty("a", model_->getLoopAPosRatio());
    current->setProperty("b", model_->getLoopBPosRatio());
    current->setProperty("speed", model_->getSpeed());
    current->setProperty("pitch", model_->getPitch());
    all->setProperty("current", var(current));
    all->setProperty("recent", Array<var>());
    all->setProperty("playlist", Array<var>());
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
    current->setProperty("volume", model_->getVolume());
#if defined(ENABLE_METRONOME)
    current->setProperty("bpm", static_cast<float>(model_->getBpm()));
    current->setProperty("metronome_offset", model_->getMetronomeOffsetSec());
#endif
    current->setProperty("a", model_->getLoopAPosRatio());
    current->setProperty("b", model_->getLoopBPosRatio());
    current->setProperty("speed", model_->getSpeed());
    current->setProperty("pitch", model_->getPitch());
    
    setting_.getDynamicObject()->setProperty("playlist", setListComponent_->getData());
    
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

bool MainComponent::isSettingValid() const
{
    return (setting_.hasProperty("global") && setting_.hasProperty("current") &&setting_.hasProperty("recent") &&setting_.hasProperty("playlist") &&setting_.hasProperty("songs"));
}

void MainComponent::volumeChanged(float volume)
{
    updateVolume();
}

void MainComponent::pitchChanged(int semitone)
{
    updatePitchButtonLabel();
}

void MainComponent::speedChanged(int speed)
{
    updateSpeedButtonLabel();
}

void MainComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    updateAButtonLabel();
    updateBButtonLabel();
}

void MainComponent::playingPosChanged(float time, float ratio)
{
}

