#include <sstream>
#include "MainComponent.h"
#include "MelissaAboutComponent.h"
#include "MelissaUISettings.h"
#include "MelissaOptionDialog.h"
#include "MelissaInputDialog.h"
#include "MelissaUtility.h"

using std::make_unique;

enum
{
    kFileChooserTabGroup = 1001,
    kPracticeMemoTabGroup = 1002,
    
    // UI
    kGradationHeight = 20,
};

enum
{
    kMenuID_MainAbout = 1000,
    kMenuID_Manual,
    kMenuID_MainVersionCheck,
    kMenuID_MainPreferences,
    kMenuID_MainTutorial,
    kMenuID_FileOpen = 2000,
};

MainComponent::MainComponent() : Thread("MelissaProcessThread"), shouldExit_(false)
{
    audioEngine_ = std::make_unique<MelissaAudioEngine>();
    
    model_ = MelissaModel::getInstance();
    model_->setMelissaAudioEngine(audioEngine_.get());
    model_->addListener(dynamic_cast<MelissaModelListener*>(audioEngine_.get()));    
    model_->addListener(this);
    
    dataSource_ = MelissaDataSource::getInstance();
    dataSource_->setMelissaAudioEngine(audioEngine_.get());
    dataSource_->addListener(this);
    
    MelissaUISettings::isJa  = (SystemStats::getDisplayLanguage() == "ja-JP" && MelissaUISettings::isJapaneseFontAvailable());
    getLookAndFeel().setDefaultSansSerifTypefaceName(MelissaUISettings::getFontName());
    
    String localizedStrings = "";
    if (MelissaUISettings::isJa)
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
    
    // load setting file
    settingsDir_ = (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Melissa"));
    if (!(settingsDir_.exists() && settingsDir_.isDirectory())) settingsDir_.createDirectory();
    
    settingsFile_ = settingsDir_.getChildFile("Settings.json");
    if (!settingsFile_.existsAsFile())
    {
        isFirstLaunch = true;
    }
    dataSource_->loadSettingsFile(settingsFile_);
    
    auto rootDir = File(dataSource_->global_.rootDir_);
    rootDir.setAsCurrentWorkingDirectory();
    fileBrowserComponent_->setRoot(rootDir);
    
    auto width  = dataSource_->global_.width_;
    auto height = dataSource_->global_.height_;
    setSize(width, height);
    
    deviceManager.initialise(0, 2, XmlDocument::parse(dataSource_->global_.device_).get(), true);
    
    dataSource_->restorePreviousState();
    deviceManager.addMidiInputCallback("", this);
    
    if (isFirstLaunch)
    {
        const std::vector<String> options = { TRANS("ok") };
        auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("first_launch"), options, [&](size_t yesno) { showFileChooser(); });
        MelissaModalDialog::show(dialog, "Melissa", false);
    }
    
#if defined(ENABLE_TUTORIAL)
    if (isFirstLaunch)
    {
        auto component = std::make_shared<MelissaOkCancelDialog>(this, TRANS("first_message"), [&](){
            showTutorial();
        });
        MelissaModalDialog::show(component, TRANS("tutorial"));
    }
#endif
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    
    setLookAndFeel(nullptr);
    historyTable_->setLookAndFeel(nullptr);
    browseToggleButton_->setLookAndFeel(nullptr);
    playlistToggleButton_->setLookAndFeel(nullptr);
    historyToggleButton_->setLookAndFeel(nullptr);
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
    
    MelissaModalDialog::setParentComponent(this);
    
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
        menu.addItem(kMenuID_Manual, TRANS("open_manual"));
        menu.addItem(kMenuID_MainVersionCheck, TRANS("check_update"));
        menu.addItem(kMenuID_MainPreferences, TRANS("preferences"));
#if defined(ENABLE_TUTORIAL)
        menu.addItem(kMenuID_MainTutorial, TRANS("tutorial"));
#endif
        const auto result = menu.show();
        model_->setPlaybackStatus(kPlaybackStatus_Stop);
        if (result == kMenuID_MainAbout)
        {
            showAboutDialog();
        }
        else if (result == kMenuID_Manual)
        {
            URL("https://github.com/mosynthkey/Melissa/wiki").launchInDefaultBrowser();
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
    controlComponent_->setColour(Label::backgroundColourId, Colour(MelissaUISettings::getMainColour()).withAlpha(0.06f));
    addAndMakeVisible(controlComponent_.get());
    
    bottomComponent_ = make_unique<MelissaBottomControlComponent>();
    lookAndFeel_.setBottomComponent(bottomComponent_.get());
    addAndMakeVisible(bottomComponent_.get());
    
    playPauseButton_ = make_unique<MelissaPlayPauseButton>("PlayButton");
    playPauseButton_->onClick = [this]() { model_->togglePlaybackStatus(); };
    addAndMakeVisible(playPauseButton_.get());
    
    toHeadButton_ = make_unique<MelissaToHeadButton>("ToHeadButton");
    toHeadButton_->onClick = [this]() { toHead(); };
    addAndMakeVisible(toHeadButton_.get());
    
    timeLabel_ = make_unique<Label>();
    timeLabel_->setJustificationType(Justification::centred);
    timeLabel_->setFont(MelissaUISettings::getFontSizeMain());
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
    volumeSlider_->setDoubleClickReturnValue(true, 1.f);
    volumeSlider_->setValue(1.f);
    volumeSlider_->onValueChange = [this]()
    {
        model_->setVolume(volumeSlider_->getValue());
    };
    addAndMakeVisible(volumeSlider_.get());
    
    aButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_loop_start_dec"), TRANS("tooltip_loop_start_inc"));
    aButton_->setText("-:--");
    aButton_->onClick_= [this](MelissaIncDecButton::ClickEvent event, bool b)
    {
        if (event == MelissaIncDecButton::kClickEvent_Double)
        {
            model_->setLoopAPosRatio(0.f);
        }
        else
        {
            const int sign = (event == MelissaIncDecButton::kClickEvent_Inc) ? 1 : -1;
            model_->setLoopAPosMSec(model_->getLoopAPosMSec() + sign * (b ? 1000 : 100));
        }
    };
    addAndMakeVisible(aButton_.get());

    aSetButton_ = make_unique<TextButton>();
    aSetButton_->setButtonText("A");
    aSetButton_->setTooltip(TRANS("tooltip_loop_start_set"));
    aSetButton_->onClick = [this]()
    {
        model_->setLoopAPosMSec(model_->getPlayingPosMSec());
    };
    addAndMakeVisible(aSetButton_.get());

    bButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_loop_end_dec"), TRANS("tooltip_loop_end_inc"));
    bButton_->setText("-:--");
    bButton_->setBounds(0, 240, 140, 34);
    bButton_->onClick_= [this](MelissaIncDecButton::ClickEvent event, bool b)
    {
        if (event == MelissaIncDecButton::kClickEvent_Double)
        {
            model_->setLoopBPosRatio(1.f);
        }
        else
        {
            const int sign = (event == MelissaIncDecButton::kClickEvent_Inc) ? 1 : -1;
            model_->setLoopBPosMSec(model_->getLoopBPosMSec() + sign * (b ? 1000 : 100));
        }
    };
    addAndMakeVisible(bButton_.get());

    bSetButton_ = make_unique<TextButton>();
    bSetButton_->setButtonText("B");
    bSetButton_->setTooltip(TRANS("tooltip_loop_end_set"));
    bSetButton_->onClick = [this]()
    {
        model_->setLoopBPosMSec(model_->getPlayingPosMSec());
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

    speedButton_ = make_unique<MelissaIncDecButton>(2, TRANS("tooltip_speed_dec"), TRANS("tooltip_speed_inc"));
    speedButton_->setText("100 %");
    speedButton_->onClick_= [this](MelissaIncDecButton::ClickEvent event, bool b)
    {
        if (event == MelissaIncDecButton::kClickEvent_Double)
        {
            model_->setSpeed(100);
        }
        else
        {
            const int sign = (event == MelissaIncDecButton::kClickEvent_Inc) ? 1 : -1;
            model_->setSpeed(model_->getSpeed() + sign * (b ? 10 : 1));
        }
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
    
    pitchButton_ = make_unique<MelissaIncDecButton>(16, TRANS("tooltip_pitch_dec"), TRANS("tooltip_pitch_inc"));
    pitchButton_->setText("Original");
    pitchButton_->onClick_= [this](MelissaIncDecButton::ClickEvent event, bool b)
    {
        if (event == MelissaIncDecButton::kClickEvent_Double)
        {
            model_->setPitch(0);
        }
        else
        {
            const int sign = (event == MelissaIncDecButton::kClickEvent_Inc) ? 1 : -1;
            model_->setPitch(model_->getPitch() + sign);
        }
    };
    pitchButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
    addAndMakeVisible(pitchButton_.get());
    
    browseToggleButton_ = make_unique<ToggleButton>();
    browseToggleButton_->setButtonText("File browser");
    browseToggleButton_->setLookAndFeel(&lookAndFeelTab_);
    browseToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    browseToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Browse); };
    browseToggleButton_->setToggleState(true, dontSendNotification);
    addAndMakeVisible(browseToggleButton_.get());
    
    playlistToggleButton_ = make_unique<ToggleButton>();
    playlistToggleButton_->setButtonText("Playlist");
    playlistToggleButton_->setLookAndFeel(&lookAndFeelTab_);
    playlistToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    playlistToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Playlist); };
    playlistToggleButton_->setToggleState(false, dontSendNotification);
    addAndMakeVisible(playlistToggleButton_.get());
    
    historyToggleButton_ = make_unique<ToggleButton>();
    historyToggleButton_->setButtonText("History");
    historyToggleButton_->setLookAndFeel(&lookAndFeelTab_);
    historyToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    historyToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_History); };
    historyToggleButton_->setToggleState(false, dontSendNotification);
    addAndMakeVisible(historyToggleButton_.get());

    historyTable_ = make_unique<MelissaFileListBox>();
    historyTable_->setTarget(MelissaFileListBox::kTarget_History);
    historyTable_->setLookAndFeel(&lookAndFeel_);
    addAndMakeVisible(historyTable_.get());

    practiceTable_ = make_unique<MelissaPracticeTableListBox>();
    addAndMakeVisible(practiceTable_.get());

    memoTextEditor_ = make_unique<TextEditor>();
    memoTextEditor_->setLookAndFeel(nullptr);
    memoTextEditor_->setFont(Font(MelissaUISettings::getFontSizeMain()));
    memoTextEditor_->setMultiLine(true, false);
    memoTextEditor_->setLookAndFeel(&lookAndFeelMemo_);
    memoTextEditor_->onFocusLost = [&]()
    {
        dataSource_->saveMemo(memoTextEditor_->getText());
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

    addToListButton_ = make_unique<MelissaAddButton>();
    addToListButton_->onClick = [this]()
    {
        const String defaultName = MelissaUtility::getFormattedTimeSec(model_->getLoopAPosMSec() / 1000.f) + " - " + MelissaUtility::getFormattedTimeSec(model_->getLoopBPosMSec() / 1000.f);
        auto dialog = std::make_shared<MelissaInputDialog>(TRANS("enter_loop_name"), defaultName, [&](const String& text) {
            String name(text);
            if (name.isEmpty()) name = defaultName;
            
            dataSource_->addPracticeList(name);
        });
        MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(dialog), TRANS("add_practice_list"));
        
    };
    addAndMakeVisible(addToListButton_.get());

    wildCardFilter_ = make_unique<WildcardFileFilter>(MelissaDataSource::getCompatibleFileExtensions(), "*", "Music Files");
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
            l->setFont(Font(MelissaUISettings::getFontSizeSub()));
            l->setColour(Label::textColourId, Colours::white.withAlpha(0.6f));
            l->setJustificationType(Justification::centredTop);
            addAndMakeVisible(l.get());
            labels_[label_i] = std::move(l);
        }
    }
    
    // Set List
    playlistComponent_ = make_unique<MelissaPlaylistComponent>();
    addChildComponent(playlistComponent_.get());
    
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
    
    tooltipWindow_ = std::make_unique<TooltipWindow>(bottomComponent_.get(), 0);
    tooltipWindow_->setLookAndFeel(&lookAndFeel_);
    
    updatePracticeMemo(kPracticeMemoTab_Practice);
    updateFileChooserTab(kFileChooserTab_Browse);
}

void MainComponent::showFileChooser()
{
    fileChooser_ = std::make_unique<FileChooser>("Open", File::getCurrentWorkingDirectory(), MelissaDataSource::getCompatibleFileExtensions(), true);
    fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [&] (const FileChooser& chooser) {
        auto fileUrl = chooser.getURLResult();
        if (fileUrl.isLocalFile())
        {
            dataSource_->loadFileAsync(fileUrl.getLocalFile().getFullPathName());
        }
    });
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioEngine_->setOutputSampleRate(sampleRate);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    if (model_->getPlaybackStatus() != kPlaybackStatus_Playing) return;
    float* buffer[] = { bufferToFill.buffer->getWritePointer(0), bufferToFill.buffer->getWritePointer(1) };
    audioEngine_->render(buffer, bufferToFill.numSamples);
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
    const int h = getHeight();
    const int center = w / 2;
    const auto gradationColour = MelissaUISettings::getBackGroundGradationColour();
    const int playButtonCenterY = playPauseButton_->getY() + playPauseButton_->getHeight() / 2;
    g.setGradientFill(ColourGradient(Colour(gradationColour.first), center, playButtonCenterY, Colour(gradationColour.second), 0, getHeight(), true));
    g.fillRect(0, 0, w / 2, h);
    g.setGradientFill(ColourGradient(Colour(gradationColour.first), center, playButtonCenterY, Colour(gradationColour.second), w, getHeight(), true));
    g.fillRect(w / 2, 0, w / 2, h);
    
    constexpr int interval = 6;
    bool offset = true;
    g.setColour(Colours::white.withAlpha(0.08f));
    for (int y_i = 0; y_i < getHeight(); y_i += interval)
    {
        if (y_i < controlComponent_->getY() || (controlComponent_->getBottom() <= y_i && y_i < bottomComponent_->getY()))
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
    
    y = bottomComponent_->getY() - kGradationHeight;
    g.setGradientFill(ColourGradient(colours[0], center, y, colours[1], center, y + kGradationHeight, false));
    g.fillRect(0, y, w, kGradationHeight);
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
    
    
    // left-bottom part (Browser / Playlist / History)
    {
        constexpr int32_t tabMargin = 2;
        int32_t browserWidth = 480;
        int32_t w = (browserWidth - tabMargin * (kNumOfFileChooserTabs - 1)) / kNumOfFileChooserTabs;
        int32_t y = controlComponent_->getBottom() + kGradationHeight - 10;
        
        int32_t x = 20;
        browseToggleButton_ ->setBounds(x, y, w, 30);
        x += (w + tabMargin);
        playlistToggleButton_->setBounds(x, y, w, 30);
        x += (w + tabMargin);
        historyToggleButton_ ->setBounds(x, y, w, 30);
        
        w = 180;
        x = 20 + browserWidth + 20;
        practiceListToggleButton_->setBounds(x, y, w, 30);
        x += (w + 2);
        memoToggleButton_->setBounds(x, y, w, 30);
        
        y += 40;
        {
            const int32_t h = getHeight() - 40 - y;
            fileBrowserComponent_->setBounds(20, y, browserWidth, h);
            playlistComponent_->setBounds(20, y, browserWidth, h);
            historyTable_->setBounds(20, y, browserWidth, h);
        }
        
        {
            const int32_t h = getHeight() - 80 - y;
            practiceTable_->setBounds(20 + browserWidth + 20, y, getWidth() - (20 + browserWidth) - 40, h);
            addToListButton_->setBounds(getWidth() - 30 - 20, practiceTable_->getBottom() + 10, 30, 30);
            memoTextEditor_->setBounds(20 + browserWidth + 20, y, getWidth() - (20 + browserWidth) - 40, h + 40);
        }
    }
    
    // Bottom
    bottomComponent_->setBounds(0, getHeight() - 30, getWidth(), 30);
    
    // Section
    const int32_t marginSideX = 40;
    const int32_t marginCenterX = 40;
    const int sectionWidth = toHeadButton_->getX() - (marginSideX + marginCenterX);
    for (auto&& sec : sectionTitles_)
    {
        sec->setSize(sectionWidth, 30);
    }
    
    int y = 260; // 260;
    
    // Song Settings
    sectionTitles_[kSectionTitle_Settings]->setTopLeftPosition(marginSideX, y);
    volumeSlider_->setSize(200, 30);
    pitchButton_->setSize(200, 30);
    
    // A-B Loop
    sectionTitles_[kSectionTitle_Loop]->setTopRightPosition(getWidth() - marginSideX, y);
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
    arrangeEvenly({ marginSideX, y + 60, sectionWidth, 30 }, {
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
        dataSource_->loadFileAsync(files[0]);
    }
    else
    {
        MelissaModalDialog::show(std::make_shared<MelissaInputDialog>(TRANS("detect_multifiles_drop"),  "new playlist", [&, files](const String& playlistName) {
            if (!playlistName.isEmpty())
            {
                const auto index = dataSource_->createPlaylist(playlistName);
                for (auto file : files) dataSource_->addToPlaylist(index, file);
                playlistComponent_->select(index);
            }
        }), TRANS("new_playlist"));
    }
}

bool MainComponent::keyPressed(const KeyPress &key, Component* originatingComponent)
{
    const auto keyCode = key.getKeyCode();
    
    switch (keyCode)
    {
        case 32: // space
        {
            model_->togglePlaybackStatus();
            return true;
        }
        case 44: // ,
        {
            model_->setPlayingPosRatio(model_->getLoopAPosRatio());
            return true;
        }
        case 65: // a
        {
            model_->setLoopAPosRatio(model_->getPlayingPosRatio());
            return true;
        }
        case 66: // b
        {
            model_->setLoopBPosRatio(model_->getPlayingPosRatio());
            return true;
        }
        case 8: // delete
        case 127:
        {
            model_->setLoopPosRatio(0.f, 1.f);
            return true;
        }
        case 63232: // up
        case 65574:
        {
            model_->setSpeed(model_->getSpeed() + 1);
            return true;
        }
        case 63233: // down
        case 65576:
        {
            model_->setSpeed(model_->getSpeed() - 1);
            return true;
        }
        case 63234: // left
        case 65573:
        {
            auto currentMSec = model_->getPlayingPosMSec();
            model_->setPlayingPosMSec(currentMSec - 1000);
            return true;
        }
        case 63235: // right
        case 65575:
        {
            auto currentMSec = model_->getPlayingPosMSec();
            model_->setPlayingPosMSec(currentMSec + 1000);
            return true;
        }
    };
    
    return false;
}

void MainComponent::showPreferencesDialog()
{
    auto component = std::make_shared<MelissaPreferencesComponent>(&deviceManager);
    MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("preferences"));
}

void MainComponent::showAboutDialog()
{
    auto component = std::make_shared<MelissaAboutComponent>();
    MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("about_melissa"));
}

void MainComponent::showTutorial()
{
#if defined(ENABLE_TUTORIAL)
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
#endif
}

void MainComponent::showUpdateDialog(bool showIfThereIsNoUpdate)
{
    const auto updateStatus = MelissaUpdateChecker::getUpdateStatus();
    
    if (updateStatus == MelissaUpdateChecker::kUpdateStatus_IsLatest)
    {
        if (showIfThereIsNoUpdate)
        {
            const std::vector<String> options = { TRANS("ok") };
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("there_is_no_update"), options, [&](size_t index){ });
            MelissaModalDialog::show(dialog, TRANS("update"));
        }
    }
    else if (updateStatus == MelissaUpdateChecker::kUpdateStatus_Failed)
    {
        if (showIfThereIsNoUpdate)
        {
            const std::vector<String> options = { TRANS("ok") };
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("failed_to_get_update"), options, [&](size_t index){ });
            MelissaModalDialog::show(dialog, TRANS("update"));
        }
    }
    else
    {
        MelissaUpdateChecker::showUpdateDialog();
    }
    
}

void MainComponent::closeTutorial()
{
    tutorialComponent_ = nullptr;
}

void MainComponent::songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate)
{
    memoTextEditor_->setText(dataSource_->getMemo());
    auto parentDir = File(filePath).getParentDirectory();
    parentDir.setAsCurrentWorkingDirectory();
    fileBrowserComponent_->setRoot(parentDir);
}

void MainComponent::fileLoadStatusChanged(FileLoadStatus status, const String& filePath)
{
    if (status == kFileLoadStatus_Success)
    {
        fileNameLabel_->setText(File(filePath).getFileNameWithoutExtension());
    }
    else if (status == kFileLoadStatus_Failed)
    {
        fileNameLabel_->setText(File(dataSource_->getCurrentSongFilePath()).getFileNameWithoutExtension());
        const std::vector<String> options = { TRANS("ok") };
        MelissaModalDialog::show(std::make_shared<MelissaOptionDialog>(TRANS("load_failed") + "\n" + filePath, options, [&](size_t) {}), "Melissa", false);
    }
    else if (status == kFileLoadStatus_Loading)
    {
        fileNameLabel_->setText("Loading...");
    }
}

void MainComponent::fileDoubleClicked(const File& file)
{
    dataSource_->loadFileAsync(file);
}

void MainComponent::browserRootChanged(const File& newRoot)
{
    newRoot.setAsCurrentWorkingDirectory();
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
    if (menuItemID == kMenuID_FileOpen) showFileChooser();
}

void MainComponent::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message)
{
    midiControlManager_.processMIDIMessage(message);
}

void MainComponent::run()
{
    while (!shouldExit_)
    {
        if (model_ != nullptr && audioEngine_->isBufferSet() && audioEngine_->needToProcess())
        {
            audioEngine_->process();
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
    dataSource_->saveMemo(memoTextEditor_->getText());
    dataSource_->saveSongState();
    dataSource_->global_.rootDir_ = File::getCurrentWorkingDirectory().getFullPathName();
    dataSource_->global_.width_   = getWidth();
    dataSource_->global_.height_  = getHeight();
    const auto stateXml = deviceManager.createStateXml();
    if (stateXml != nullptr) dataSource_->global_.device_  = stateXml->toString();
    
    dataSource_->saveSettingsFile();
    dataSource_->disposeBuffer();
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
    playlistComponent_->setVisible(tab == kFileChooserTab_Playlist);
    historyTable_->setVisible(tab == kFileChooserTab_History);
}

void MainComponent::updatePracticeMemo(PracticeMemoTab tab)
{
    practiceTable_->setVisible(tab == kPracticeMemoTab_Practice);
    addToListButton_->setVisible(tab == kPracticeMemoTab_Practice);
    memoTextEditor_->setVisible(tab == kPracticeMemoTab_Memo);
}

void MainComponent::toHead()
{
    if (model_ == nullptr) return;
    model_->setPlayingPosRatio(model_->getLoopAPosRatio());
}

void MainComponent::resetLoop()
{
    model_->setLoopPosRatio(0.f, 1.f);
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


void MainComponent::arrangeEvenly(const juce::Rectangle<int> bounds, const std::vector<std::vector<Component*>>& components_, float widthRatio)
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

void MainComponent::volumeChanged(float volume)
{
    volumeSlider_->setValue(volume);
    const float db = 20 * log10(volume);
    String dbStr = String::formatted("%+1.1f dB", db);
    volumeSlider_->setTooltip(TRANS("volume") + " : " + dbStr);
}

void MainComponent::pitchChanged(int semitone)
{
    pitchButton_->setText(MelissaUtility::getFormattedPitch(semitone));
}

void MainComponent::speedChanged(int speed)
{
    speedButton_->setText(String(speed) + "%");
}

void MainComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    aButton_->setText(MelissaUtility::getFormattedTimeMSec(aTimeMSec));
    bButton_->setText(MelissaUtility::getFormattedTimeMSec(bTimeMSec));
}
