//
//  MainComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <sstream>
#include "MainComponent.h"
#include "MelissaAboutComponent.h"
#include "MelissaDefinitions.h"
#include "MelissaInputDialog.h"
#include "MelissaOptionDialog.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"

using std::make_unique;

enum
{
    kFileChooserTabGroup = 1000,
    kListMemoTabGroup = 2000,
    kSpeedModeTabGroup = 3000,
    
    // UI
    kGradationHeight = 20,
    kScrollbarThichness = 4,
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

MainComponent::MainComponent() : Thread("MelissaProcessThread"), simpleTextButtonLaf_(MelissaUISettings::getFontSizeSub(), Justification::centredRight), shouldExit_(false)
{
    audioEngine_ = std::make_unique<MelissaAudioEngine>();
    
    model_ = MelissaModel::getInstance();
    model_->setMelissaAudioEngine(audioEngine_.get());
    model_->addListener(dynamic_cast<MelissaModelListener*>(audioEngine_.get()));    
    model_->addListener(this);
    
    dataSource_ = MelissaDataSource::getInstance();
    dataSource_->setMelissaAudioEngine(audioEngine_.get());
    dataSource_->addListener(this);
    
    bpmDetector_ = std::make_unique<MelissaBPMDetector>();
    
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
    resetButton_->setLookAndFeel(nullptr);
    volumeBalanceSlider_->setLookAndFeel(nullptr);
    metronomeOnOffButton_->setLookAndFeel(nullptr);
    eqSwitchButton_->setLookAndFeel(nullptr);
    analyzeButton_->setLookAndFeel(nullptr);
    speedModeBasicToggleButton_->setLookAndFeel(nullptr);
    speedModeTrainingToggleButton_->setLookAndFeel(nullptr);
    for (auto&& b : speedPresetButtons_)
    {
        b->setLookAndFeel(nullptr);
    }
    historyTable_->setLookAndFeel(nullptr);
    browseToggleButton_->setLookAndFeel(nullptr);
    playlistToggleButton_->setLookAndFeel(nullptr);
    historyToggleButton_->setLookAndFeel(nullptr);
    practiceListToggleButton_->setLookAndFeel(nullptr);
    markerListToggleButton_->setLookAndFeel(nullptr);
    memoToggleButton_->setLookAndFeel(nullptr);
    memoTextEditor_->setLookAndFeel(nullptr);
    practiceTable_->setLookAndFeel(nullptr);
    tooltipWindow_->setLookAndFeel(nullptr);
    metronomeOnOffButton_->setLookAndFeel(nullptr);
    volumeBalanceSlider_->setLookAndFeel(nullptr);
    
#if JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr);
#endif
    
    stopThread(4000.f);
    stopTimer();
}

void MainComponent::createUI()
{
    setLookAndFeel(&laf_);
    
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
        menu.setLookAndFeel(&laf_);
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
    
    messageComponent_ = MelissaMessageComponent::getInstance();
    addAndMakeVisible(messageComponent_);
    
    waveformComponent_ = make_unique<MelissaWaveformControlComponent>();
    addAndMakeVisible(waveformComponent_.get());
    
    controlComponent_ = make_unique<Label>();
    controlComponent_->setOpaque(false);
    controlComponent_->setColour(Label::backgroundColourId, Colour(MelissaUISettings::getMainColour()).withAlpha(0.06f));
    addAndMakeVisible(controlComponent_.get());
    
    bottomComponent_ = make_unique<MelissaBottomControlComponent>();
    laf_.setBottomComponent(bottomComponent_.get());
    addAndMakeVisible(bottomComponent_.get());
    
    String sectionTitles[kNumOfSections] =
    {
        "Song",
        "Loop",
        "Speed",
        "Metronome",
        "EQ",
        "Output"
    };
    for (size_t sectionIndex = 0; sectionIndex < kNumOfSections; ++sectionIndex)
    {
        auto s = std::make_unique<MelissaSectionComponent>(sectionTitles[sectionIndex]);
        addAndMakeVisible(s.get());
        sectionComponents_[sectionIndex] = std::move(s);
    }
    
    {
        auto section = sectionComponents_[kSection_Song].get();
    
        playPauseButton_ = make_unique<MelissaPlayPauseButton>("PlayButton");
        playPauseButton_->onClick = [this]() { model_->togglePlaybackStatus(); };
        section->addAndMakeVisible(playPauseButton_.get());
        
        toHeadButton_ = make_unique<MelissaToHeadButton>("ToHeadButton");
        toHeadButton_->onClick = [this]() { toHead(); };
        section->addAndMakeVisible(toHeadButton_.get());
        
        timeLabel_ = make_unique<Label>();
        timeLabel_->setJustificationType(Justification::centred);
        timeLabel_->setFont(MelissaUISettings::getFontSizeMain());
        section->addAndMakeVisible(timeLabel_.get());
        
        fileNameLabel_ = make_unique<MelissaScrollLabel>(timeLabel_->getFont());
        section->addAndMakeVisible(fileNameLabel_.get());
        
        pitchButton_ = make_unique<MelissaIncDecButton>(16, TRANS("tooltip_pitch_dec"), TRANS("tooltip_pitch_inc"));
        pitchButton_->setText("Original");
        pitchButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                model_->setPitch(0);
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setPitch(model_->getPitch() + sign);
            }
        };
        pitchButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        section->addAndMakeVisible(pitchButton_.get());
    }
    
    {
        auto section = sectionComponents_[kSection_Loop].get();
        
        aButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_loop_start_dec"), TRANS("tooltip_loop_start_inc"));
        aButton_->setText("-:--");
        aButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Left, "A", TRANS("tooltip_loop_start_set"));
        aButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                model_->setLoopAPosRatio(0.f);
            }
            else if (event == MelissaIncDecButton::kEvent_Func)
            {
                model_->setLoopAPosMSec(model_->getPlayingPosMSec());
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setLoopAPosMSec(model_->getLoopAPosMSec() + sign * (b ? 1000 : 100));
            }
        };
        section->addAndMakeVisible(aButton_.get());

        bButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_loop_end_dec"), TRANS("tooltip_loop_end_inc"));
        bButton_->setText("-:--");
        bButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Left, "B", TRANS("tooltip_loop_end_set"));
        bButton_->setBounds(0, 240, 140, 34);
        bButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                model_->setLoopBPosRatio(1.f);
            }
            else if (event == MelissaIncDecButton::kEvent_Func)
            {
                model_->setLoopBPosMSec(model_->getPlayingPosMSec());
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setLoopBPosMSec(model_->getLoopBPosMSec() + sign * (b ? 1000 : 100));
            }
        };
        section->addAndMakeVisible(bButton_.get());
        
        leftArrowImage_             = Drawable::createFromImageData(BinaryData::arrow_left_svg, BinaryData::arrow_left_svgSize);
        rightArrowImage_            = Drawable::createFromImageData(BinaryData::arrow_right_svg, BinaryData::arrow_right_svgSize);
        leftArrowHighlightedImage_  = Drawable::createFromImageData(BinaryData::arrow_left_highlighted_svg, BinaryData::arrow_left_highlighted_svgSize);
        rightArrowHighlightedImage_ = Drawable::createFromImageData(BinaryData::arrow_right_highlighted_svg, BinaryData::arrow_right_highlighted_svgSize);
        
        aResetButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        aResetButton_->setImages(leftArrowImage_.get(), leftArrowHighlightedImage_.get());
        aResetButton_->onClick = [&]()
        {
            model_->setLoopAPosRatio(0.f);
        };
        section->addAndMakeVisible(aResetButton_.get());
        
        bResetButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        bResetButton_->setImages(rightArrowImage_.get(), rightArrowHighlightedImage_.get());
        bResetButton_->onClick = [&]()
        {
            model_->setLoopBPosRatio(1.f);
        };
        section->addAndMakeVisible(bResetButton_.get());

        resetButton_ = make_unique<TextButton>();
        resetButton_->setButtonText("Reset");
        resetButton_->setTooltip(TRANS("tooltip_loop_reset"));
        resetButton_->onClick = [this]() { resetLoop(); };
        resetButton_->setLookAndFeel(&simpleTextButtonLaf_);
        section->addAndMakeVisible(resetButton_.get());
    }
    
    {
        auto section = sectionComponents_[kSection_Speed].get();
        
        speedModeNormalComponent_ = make_unique<Component>();
        section->addAndMakeVisible(speedModeNormalComponent_.get());
        
        speedModeTrainingComponent_ = make_unique<Component>();
        section->addAndMakeVisible(speedModeTrainingComponent_.get());
        
        speedModeBasicToggleButton_ = make_unique<ToggleButton>();
        speedModeBasicToggleButton_->setButtonText("Basic");
        speedModeBasicToggleButton_->setLookAndFeel(&selectorLaf_);
        speedModeBasicToggleButton_->setRadioGroupId(kSpeedModeTabGroup);
        speedModeBasicToggleButton_->onClick = [&]()
        {
            model_->setSpeedMode(kSpeedMode_Basic);
            updateSpeedModeTab(kSpeedModeTab_Basic);
        };
        speedModeBasicToggleButton_->setToggleState(true, dontSendNotification);
        section->addAndMakeVisible(speedModeBasicToggleButton_.get());
        
        speedModeTrainingToggleButton_ = make_unique<ToggleButton>();
        speedModeTrainingToggleButton_->setButtonText("Training");
        speedModeTrainingToggleButton_->setLookAndFeel(&selectorLaf_);
        speedModeTrainingToggleButton_->setRadioGroupId(kSpeedModeTabGroup);
        speedModeTrainingToggleButton_->onClick = [&]()
        {
            model_->setSpeedMode(kSpeedMode_Training);
            updateSpeedModeTab(kSpeedModeTab_Training);
        };
        speedModeTrainingToggleButton_->setToggleState(false, dontSendNotification);
        section->addAndMakeVisible(speedModeTrainingToggleButton_.get());

        speedButton_ = make_unique<MelissaIncDecButton>(2, TRANS("tooltip_speed_dec"), TRANS("tooltip_speed_inc"));
        speedButton_->setText("100 %");
        speedButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                model_->setSpeed(100);
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setSpeed(model_->getSpeed() + sign * (b ? 10 : 1));
            }
        };
        speedButton_->setColour(Label::textColourId, Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        speedModeNormalComponent_->addAndMakeVisible(speedButton_.get());
        
        speedPresetViewport_ = make_unique<Viewport>();
        speedPresetViewport_->setScrollBarThickness(kScrollbarThichness);
        speedModeNormalComponent_->addAndMakeVisible(speedPresetViewport_.get());
        
        speedPresetComponent_ = make_unique<Component>();
        speedModeNormalComponent_->addAndMakeVisible(speedPresetComponent_.get());
        const int speedPresets[kNumOfSpeedPresets] = { 50, 60, 70, 75, 80, 85, 90, 95, 100 };
        constexpr int presetButtonMargin = 4;
        constexpr int controlHeight = 30;
        int speedButtonX = 0;
        int speedButtonWidthSum = 0;
        for (size_t speedPresetIndex = 0; speedPresetIndex < kNumOfSpeedPresets; ++speedPresetIndex)
        {
            auto b = make_unique<TextButton>();
            b->setLookAndFeel(&simpleTextButtonLaf_);
            const int speed = speedPresets[speedPresetIndex];
            b->setButtonText(String(speed) + "%");
            b->onClick = [&, speed]() { model_->setSpeed(speed); };
            const auto width = MelissaUtility::getStringSize(simpleTextButtonLaf_.getFontSize(), b->getButtonText()).first;
            speedButtonWidthSum += width;
            b->setBounds(speedButtonX, 0, width, 30);
            speedButtonX += (width + presetButtonMargin);
            speedPresetComponent_->addAndMakeVisible(b.get());
            speedPresetButtons_[speedPresetIndex] = std::move(b);
        }
        
        speedPresetComponent_->setSize(speedButtonWidthSum + presetButtonMargin * (kNumOfSpeedPresets - 1), controlHeight - kScrollbarThichness);
        speedPresetViewport_->setViewedComponent(speedPresetComponent_.get(), false);
        
        speedIncStartButton_ = make_unique<MelissaIncDecButton>(1, TRANS("todo"), TRANS("todo"));
        speedIncStartButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setSpeedIncStart(model_->getSpeedIncStart() + sign * (b ? 10 : 1));
            }
        };
        speedModeTrainingComponent_->addAndMakeVisible(speedIncStartButton_.get());

        speedIncPerButton_ = make_unique<MelissaIncDecButton>(1, TRANS("todo"), TRANS("todo"));
        speedIncPerButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setSpeedIncPer(model_->getSpeedIncPer() + sign * (b ? 10 : 1));
            }
        };
        speedModeTrainingComponent_->addAndMakeVisible(speedIncPerButton_.get());
        
        speedIncValueButton_ = make_unique<MelissaIncDecButton>(1, TRANS("todo"), TRANS("todo"));
        speedIncValueButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setSpeedIncValue(model_->getSpeedIncValue() + sign * (b ? 10 : 1));
            }
        };
        speedModeTrainingComponent_->addAndMakeVisible(speedIncValueButton_.get());
        
        speedIncGoalButton_ = make_unique<MelissaIncDecButton>(1, TRANS("todo"), TRANS("todo"));
        speedIncGoalButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setSpeedIncGoal(model_->getSpeedIncGoal() + sign * (b ? 10 : 1));
            }
        };
        speedModeTrainingComponent_->addAndMakeVisible(speedIncGoalButton_.get());
    }
    
    {
        auto section = sectionComponents_[kSection_Metronome].get();
        
        metronomeOnOffButton_ = make_unique<ToggleButton>();
        metronomeOnOffButton_->setClickingTogglesState(true);
        metronomeOnOffButton_->setLookAndFeel(&slideToggleLaf_);
        metronomeOnOffButton_->onClick = [this]()
        {
            const auto on = metronomeOnOffButton_->getToggleState();
            model_->setMetronomeSwitch(on);
        };
        section->addAndMakeVisible(metronomeOnOffButton_.get());
        
        bpmButton_ = make_unique<MelissaIncDecButton>(1, TRANS("bpm"), TRANS("bpm"));
        bpmButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setBpm(model_->getBpm() + sign);
            }
        };
        section->addAndMakeVisible(bpmButton_.get());
        
        beatPositionButton_ = make_unique<MelissaIncDecButton>(1, TRANS("metronome"), TRANS("metronome"));
        beatPositionButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Right, "Set", TRANS("todo"));
        beatPositionButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else if (event == MelissaIncDecButton::kEvent_Func)
            {
                model_->setBeatPositionMSec(model_->getPlayingPosMSec());
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setBeatPositionMSec(model_->getBeatPositionMSec() + sign * 100);
            }
        };
        section->addAndMakeVisible(beatPositionButton_.get());
        
        accentButton_ = make_unique<MelissaIncDecButton>(1, TRANS("todo"), TRANS("todo"));
        accentButton_->onClick_= [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setAccent(model_->getAccent() + sign);
            }
        };
        section->addAndMakeVisible(accentButton_.get());
        
        analyzeButton_ = make_unique<TextButton>();
        analyzeButton_->setLookAndFeel(&simpleTextButtonLaf_);
        analyzeButton_->setButtonText("Auto detect");
        analyzeButton_->onClick = [this]()
        {
            bpmDetector_->start();
        };
        section->addAndMakeVisible(analyzeButton_.get());
    }
    
    {
        auto section = sectionComponents_[kSection_Eq].get();
        
        eqSwitchButton_ = std::make_unique<ToggleButton>();
        eqSwitchButton_->setClickingTogglesState(true);
        eqSwitchButton_->setLookAndFeel(&slideToggleLaf_);
        eqSwitchButton_->onClick = [this]()
        {
            const auto on = eqSwitchButton_->getToggleState();
            model_->setEqSwitch(on);
        };
        section->addAndMakeVisible(eqSwitchButton_.get());
        
        for (size_t bandIndex = 0; bandIndex < kNumOfEqBands; ++bandIndex)
        {
            auto freqKnob = std::make_unique<Slider>();
            freqKnob->setSliderStyle(Slider::RotaryVerticalDrag);
            freqKnob->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            freqKnob->setRange(kEqFreqMin, kEqFreqMax);
            freqKnob->onValueChange = [&, bandIndex]()
            {
                auto value = eqFreqKnobs_[bandIndex]->getValue();
                model_->setEqFreq(0, value);
            };
            section->addAndMakeVisible(freqKnob.get());
            eqFreqKnobs_[bandIndex] = std::move(freqKnob);
            
            auto qKnob = std::make_unique<Slider>();
            qKnob->setSliderStyle(Slider::RotaryVerticalDrag);
            qKnob->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            qKnob->setRange(kEqQMin, kEqQMax);
            qKnob->onValueChange = [&, bandIndex]()
            {
                auto value = eqQKnobs_[bandIndex]->getValue();
                model_->setEqQ(0, value);
            };
            section->addAndMakeVisible(qKnob.get());
            eqQKnobs_[bandIndex] = std::move(qKnob);
            
            auto gainKnob = std::make_unique<Slider>();
            gainKnob->setSliderStyle(Slider::RotaryVerticalDrag);
            gainKnob->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            gainKnob->setRange(kEqGainMin, kEqGainMax);
            gainKnob->onValueChange = [&, bandIndex]()
            {
                auto value = eqGainKnobs_[bandIndex]->getValue();
                model_->setEqGain(0, value);
            };
            section->addAndMakeVisible(gainKnob.get());
            eqGainKnobs_[bandIndex] = std::move(gainKnob);
        }
        
        constexpr int numOfControls = 3;
        const static String labelTitles[] = { "Freq", "Q", "Gain" };
        for (size_t labelIndex = 0; labelIndex < kNumOfEqBands * numOfControls; ++labelIndex)
        {
            auto l = std::make_unique<Label>();
            l->setText(labelTitles[labelIndex % numOfControls], dontSendNotification);
            l->setJustificationType(Justification::centred);
            section->addAndMakeVisible(l.get());
            knobLabels_[labelIndex] = std::move(l);
        }
    }
    
    {
        auto section = sectionComponents_[kSection_Output].get();
        
        oututModeComboBox_ = make_unique<ComboBox>();
        oututModeComboBox_->setJustificationType(Justification::centred);
        oututModeComboBox_->addItem("L - R", kOutputMode_LR + 1);
        oututModeComboBox_->addItem("L - L", kOutputMode_LL + 1);
        oututModeComboBox_->addItem("R - R", kOutputMode_RR + 1);
        oututModeComboBox_->onChange = [&]()
        {
            OutputMode mode = static_cast<OutputMode>(oututModeComboBox_->getSelectedId() - 1);
            model_->setOutputMode(mode);
        };
        oututModeComboBox_->setSelectedId(kOutputMode_LR + 1);
        section->addAndMakeVisible(oututModeComboBox_.get());
    
        musicVolumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        musicVolumeSlider_->setRange(0.01f, 2.0f);
        musicVolumeSlider_->setDoubleClickReturnValue(true, 1.f);
        musicVolumeSlider_->setValue(1.f);
        musicVolumeSlider_->onValueChange = [this]()
        {
            model_->setMusicVolume(musicVolumeSlider_->getValue());
        };
        section->addAndMakeVisible(musicVolumeSlider_.get());
        
        volumeBalanceSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        volumeBalanceSlider_->setRange(0.f, 1.0f);
        volumeBalanceSlider_->setDoubleClickReturnValue(true, 0.5f);
        volumeBalanceSlider_->setValue(0.5);
        volumeBalanceSlider_->setLookAndFeel(&crossFaderLaf_);
        volumeBalanceSlider_->onValueChange = [this]()
        {
            model_->setMusicMetronomeBalance(volumeBalanceSlider_->getValue());
        };
        section->addAndMakeVisible(volumeBalanceSlider_.get());
        
        metronomeVolumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        metronomeVolumeSlider_->setRange(0.f, 1.0f);
        metronomeVolumeSlider_->setDoubleClickReturnValue(true, 1.f);
        metronomeVolumeSlider_->setValue(1.f);
        metronomeVolumeSlider_->onValueChange = [this]()
        {
            model_->setMetronomeVolume(metronomeVolumeSlider_->getValue());
        };
        section->addAndMakeVisible(metronomeVolumeSlider_.get());
    }
    
    browseToggleButton_ = make_unique<ToggleButton>();
    browseToggleButton_->setButtonText("File browser");
    browseToggleButton_->setLookAndFeel(&tabLaf_);
    browseToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    browseToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Browse); };
    browseToggleButton_->setToggleState(true, dontSendNotification);
    addAndMakeVisible(browseToggleButton_.get());
    
    playlistToggleButton_ = make_unique<ToggleButton>();
    playlistToggleButton_->setButtonText("Playlist");
    playlistToggleButton_->setLookAndFeel(&tabLaf_);
    playlistToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    playlistToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_Playlist); };
    playlistToggleButton_->setToggleState(false, dontSendNotification);
    addAndMakeVisible(playlistToggleButton_.get());
    
    historyToggleButton_ = make_unique<ToggleButton>();
    historyToggleButton_->setButtonText("History");
    historyToggleButton_->setLookAndFeel(&tabLaf_);
    historyToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    historyToggleButton_->onClick = [&]() { updateFileChooserTab(kFileChooserTab_History); };
    historyToggleButton_->setToggleState(false, dontSendNotification);
    addAndMakeVisible(historyToggleButton_.get());

    historyTable_ = make_unique<MelissaFileListBox>();
    historyTable_->setTarget(MelissaFileListBox::kTarget_History);
    historyTable_->setLookAndFeel(&laf_);
    addAndMakeVisible(historyTable_.get());

    practiceTable_ = make_unique<MelissaPracticeTableListBox>();
    addAndMakeVisible(practiceTable_.get());
    
    markerTable_ = make_unique<MelissaMarkerListBox>();
    addAndMakeVisible(markerTable_.get());

    memoTextEditor_ = make_unique<TextEditor>();
    memoTextEditor_->setLookAndFeel(nullptr);
    memoTextEditor_->setFont(Font(MelissaUISettings::getFontSizeMain()));
    memoTextEditor_->setMultiLine(true, false);
    memoTextEditor_->setLookAndFeel(&memoLaf_);
    memoTextEditor_->onFocusLost = [&]()
    {
        dataSource_->saveMemo(memoTextEditor_->getText());
    };
    memoTextEditor_->setReturnKeyStartsNewLine(true);
    addAndMakeVisible(memoTextEditor_.get());
    
    auto createAndAddTab = [&](const String& title, ListMemoTab tab)
    {
        auto b  = make_unique<ToggleButton>();
        b->setButtonText(title);
        b->setLookAndFeel(&tabLaf_);
        b->setRadioGroupId(kListMemoTabGroup);
        b->onClick = [&, tab]() { updateListMemoTab(tab); };
        addAndMakeVisible(b.get());
        return b;
    };

    practiceListToggleButton_ = createAndAddTab("Practice list", kListMemoTab_Practice);
    markerListToggleButton_   = createAndAddTab("Marker", kListMemoTab_Marker);
    memoToggleButton_         = createAndAddTab("Memo", kListMemoTab_Memo);
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
    
    
    labelInfo_[kLabel_MetronomeBpm]     = { "BPM",           bpmButton_.get() };
    labelInfo_[kLabel_MetronomeOffset]  = { "Beat position", beatPositionButton_.get() };
    labelInfo_[kLabel_MetronomeAccent]  = { "Accent",        accentButton_.get() };
    labelInfo_[kLabel_MusicVolume]      = { "Music",         musicVolumeSlider_.get() };
    labelInfo_[kLabel_MetronomeVolume]  = { "Metronome",     metronomeVolumeSlider_.get() };
    labelInfo_[kLabel_Pitch]            = { "Pitch",         pitchButton_.get() };
    labelInfo_[kLabel_OutputMode]       = { "Output",        oututModeComboBox_.get() };
    labelInfo_[kLabel_ATime]            = { "Start",         aButton_.get() };
    labelInfo_[kLabel_BTime]            = { "End",           bButton_.get() };
    labelInfo_[kLabel_Speed]            = { "Speed",         speedButton_.get() };
    labelInfo_[kLabel_SpeedPresets]     = { "Presets",       speedPresetViewport_.get() };
    labelInfo_[kLabel_SpeedBegin]       = { "Begin",         speedIncStartButton_.get() };
    labelInfo_[kLabel_SpeedPlus]        = { "+",             speedIncValueButton_.get() };
    labelInfo_[kLabel_SpeedPer]         = { "Per",           speedIncPerButton_.get() };
    labelInfo_[kLabel_SpeedEnd]         = { "End",           speedIncGoalButton_.get() };
    
    for (size_t label_i = 0; label_i < kNumOfLabels; ++label_i)
    {
        auto l = make_unique<Label>();
        l->setLookAndFeel(nullptr);
        l->setText(labelInfo_[label_i].first, dontSendNotification);
        l->setFont(Font(MelissaUISettings::getFontSizeSub()));
        l->setColour(Label::textColourId, Colours::white.withAlpha(0.6f));
        l->setInterceptsMouseClicks(false, true);
        l->setJustificationType(Justification::centred);
        labelInfo_[label_i].second->getParentComponent()->addAndMakeVisible(l.get());
        labels_[label_i] = std::move(l);
    }
    
    // Set List
    playlistComponent_ = make_unique<MelissaPlaylistComponent>();
    addChildComponent(playlistComponent_.get());
    
    tooltipWindow_ = std::make_unique<TooltipWindow>(bottomComponent_.get(), 0);
    tooltipWindow_->setLookAndFeel(&laf_);
    
    updateSpeedModeTab(kSpeedModeTab_Basic);
    updateListMemoTab(kListMemoTab_Practice);
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
    messageComponent_->setBounds(50, 0, getWidth() - 100, 30);
    
    waveformComponent_->setBounds(60, 40, getWidth() - 60 * 2, 160);
    
    controlComponent_->setBounds(0, waveformComponent_->getBottom() + 10, getWidth(), 230);
    
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
        markerListToggleButton_->setBounds(x, y, w, 30);
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
            markerTable_->setBounds(20 + browserWidth + 20, y, getWidth() - (20 + browserWidth) - 40, h + 40);
            addToListButton_->setBounds(getWidth() - 30 - 20, practiceTable_->getBottom() + 10, 30, 30);
            memoTextEditor_->setBounds(20 + browserWidth + 20, y, getWidth() - (20 + browserWidth) - 40, h + 40);
        }
    }
    
    // Bottom
    bottomComponent_->setBounds(0, getHeight() - 30, getWidth(), 30);
    
    int y = controlComponent_->getY() + 10;
    
    // TODO : adjust
    const int sectionMarginX = 10;
    const int sectionMarginY = 10;
    const int totalSectionWidth = getWidth() - sectionMarginX * 4;
    const int songWidth  = totalSectionWidth * 0.35;
    const int loopWidth  = totalSectionWidth * 0.25;
    const int speedWidth = totalSectionWidth * 0.4;
    const int metronomeWidth = songWidth;
    const int eqWidth        = loopWidth;
    const int outputWidth    = speedWidth;
    constexpr int controlHeight = 30;
    constexpr int controlAWidthMin = 120; // incDecButton etc..
    constexpr int controlBWidthMin = controlAWidthMin + 20;
    constexpr int controlAWidthMax = controlAWidthMin + 100;
    constexpr int controlBWidthMax = controlBWidthMin + 100;
    
    // Song
    {
        auto section = sectionComponents_[kSection_Song].get();
        section->setBounds(10, y, songWidth, 100);
        
        int x = 10;
        const int centerY = 30 + (section->getHeight() - 30) / 2;
        
        toHeadButton_->setSize(16, 16);
        x += (toHeadButton_->getWidth() / 2);
        toHeadButton_->setCentrePosition(x, centerY);
        
        x += 20;
        playPauseButton_->setSize(52, 52);
        x += (playPauseButton_->getWidth() / 2);
        playPauseButton_->setCentrePosition(x, centerY);
        
        x = section->getWidth() - 10;
        pitchButton_->setSize(controlAWidthMin, controlHeight);
        x -= pitchButton_->getWidth() / 2;
        pitchButton_->setCentrePosition(x, centerY + 14);
        
        x = playPauseButton_->getRight() + 10;
        const int labelWidth = (pitchButton_->getX() - 10) - (playPauseButton_->getRight() + 20);
        fileNameLabel_->setBounds(x, centerY - 20, labelWidth, 20);
        timeLabel_->setBounds(x, centerY, labelWidth, 20);
    }
    
    // Loop
    {
        auto section = sectionComponents_[kSection_Loop].get();
        section->setBounds(sectionComponents_[kSection_Song]->getRight() + sectionMarginX, y, loopWidth, 100);
        
        const int buttonWidth = std::clamp((section->getWidth() - 10 - 10 - 10) / 2, controlBWidthMin, controlBWidthMax);
        const int y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;
        
        aButton_->setBounds(10, y, buttonWidth, controlHeight);
        bButton_->setBounds(section->getWidth() - 10 - buttonWidth, y, buttonWidth, controlHeight);
        
        aResetButton_->setBounds(aButton_->getX() + 10, aButton_->getY() - 24 + 2, 20, 14);
        bResetButton_->setBounds(bButton_->getRight() - 20 - 10, bButton_->getY() - 24 + 2, 20, 14);
        
        const int resetButtonWidth = MelissaUtility::getStringSize(MelissaUISettings::getFontSizeSub(), resetButton_->getButtonText()).first;
        resetButton_->setSize(resetButtonWidth, 30);
        resetButton_->setTopRightPosition(section->getWidth() - 10, 0);
    }
    
    // Speed
    {
        auto section = sectionComponents_[kSection_Speed].get();
        section->setBounds(sectionComponents_[kSection_Loop]->getRight() + sectionMarginX, y, speedWidth, 100);
        speedModeNormalComponent_->setBounds(section->getLocalBounds());
        speedModeTrainingComponent_->setBounds(section->getLocalBounds());
        
        int x = 10;
        int y = 30 + (section->getHeight() - 30) / 2;
        speedModeBasicToggleButton_->setBounds(x, y - 30, 100, controlHeight);
        speedModeTrainingToggleButton_->setBounds(x, y, 100, controlHeight);
        
        x = speedModeBasicToggleButton_->getRight() + 10;
        y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;
        speedButton_->setBounds(x, y, controlAWidthMin, controlHeight);
        
        const int viewportWidth = std::clamp((section->getWidth() - 10) - (speedButton_->getRight() + 10), controlAWidthMin, speedPresetComponent_->getWidth());
        x = (speedButton_->getRight() + 10) + ((section->getWidth() - 10) - (speedButton_->getRight() + 10)) / 2 - viewportWidth / 2;
        speedPresetViewport_->setBounds(x, y, viewportWidth, controlHeight);
        
        x = speedModeBasicToggleButton_->getRight() + 10;
        speedIncStartButton_->setBounds(x, y, 120, controlHeight);
        speedIncPerButton_->setBounds(speedIncStartButton_->getRight() + 20, y, 120, controlHeight);
        speedIncValueButton_->setBounds(speedIncPerButton_->getRight() + 20, y, 120, controlHeight);
        speedIncGoalButton_->setBounds(speedIncValueButton_->getRight() + 20, y, 120, controlHeight);
    }
    
    y = sectionComponents_[kSection_Song]->getBottom() + sectionMarginY;
    
    // Metronome
    {
        auto section = sectionComponents_[kSection_Metronome].get();
        section->setBounds(10, y, metronomeWidth, 100);
        
        const int buttonWidth = std::clamp((section->getWidth() - 10 - 10 - 10 - 10) / 3, controlBWidthMin, controlBWidthMax);
        const int y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;
        
        metronomeOnOffButton_->setBounds(10, 5, 40, 20);
        const int analyzeButtonWidth = MelissaUtility::getStringSize(MelissaUISettings::getFontSizeSub(), analyzeButton_->getButtonText()).first;
        analyzeButton_->setSize(analyzeButtonWidth, 30);
        analyzeButton_->setTopRightPosition(section->getWidth() - 10, 0);
        
        bpmButton_->setBounds(10, y, buttonWidth, controlHeight);
        accentButton_->setBounds((section->getWidth() - buttonWidth) / 2, y, buttonWidth, controlHeight);
        beatPositionButton_->setBounds(section->getWidth() - buttonWidth - 10, y, buttonWidth, controlHeight);
    }
    
    // EQ
    {
        auto section = sectionComponents_[kSection_Eq].get();
        section->setBounds(sectionComponents_[kSection_Metronome]->getRight() + sectionMarginX, y, eqWidth, 100);
        
        eqSwitchButton_->setBounds(10, 5, 40, 20);
        
        constexpr int knobSize = 40;
        const int y = 30 + (section->getHeight() - 30) / 2 - knobSize / 2 - 6;
        const int interval = (section->getWidth() - knobSize * kNumOfEqBands * 3) / (kNumOfEqBands * 3 + 1);
        
        int x = interval;
        const int expandWidth = 40;
        for (size_t bandIndex = 0; bandIndex < kNumOfEqBands; ++bandIndex)
        {
            eqFreqKnobs_[bandIndex]->setBounds(x, y, knobSize, knobSize);
            knobLabels_[bandIndex * 3 + 0]->setBounds(x - expandWidth / 2, y + knobSize - 8, knobSize + expandWidth, 30);
            x += knobSize + interval;
            
            eqQKnobs_   [bandIndex]->setBounds(x, y, knobSize, knobSize);
            knobLabels_[bandIndex * 3 + 1]->setBounds(x - expandWidth / 2, y + knobSize - 8, knobSize + expandWidth, 30);
            x += knobSize + interval;
            
            eqGainKnobs_[bandIndex]->setBounds(x, y, knobSize, knobSize);
            knobLabels_[bandIndex * 3 + 2]->setBounds(x - expandWidth / 2, y + knobSize - 8, knobSize + expandWidth, 30);
            x += knobSize + interval;
        }
    }
    
    // Output
    {
        auto section = sectionComponents_[kSection_Output].get();
        section->setBounds(sectionComponents_[kSection_Eq]->getRight() + sectionMarginX, y, outputWidth, 100);
        
        const int controlWidth = (section->getWidth() - 10 * 5) / 4;
        const int y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;
        
        oututModeComboBox_->setBounds(10, y, controlWidth, controlHeight);
        musicVolumeSlider_->setBounds(oututModeComboBox_->getRight() + 10, y, controlWidth, controlHeight);
        volumeBalanceSlider_->setBounds(musicVolumeSlider_->getRight() + 10, y, controlWidth, controlHeight);
        metronomeVolumeSlider_->setBounds(volumeBalanceSlider_->getRight() + 10, y, controlWidth, controlHeight);
    }
    
    for (size_t label_i = 0; label_i < kNumOfLabels; ++label_i )
    {
        auto b = labelInfo_[label_i].second->getBoundsInParent();
        labels_[label_i]->setBounds(b.getX(), b.getY() - 30, b.getWidth(), 30);
    }
    
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
        case 77: // m
        {
            MelissaDataSource::Song::Marker marker;
            marker.position_ = model_->getPlayingPosRatio();
            Colour colour = Colour::fromRGB(255, 160, 160);
            colour = colour.withHue(marker.position_);
            marker.colourR_  = colour.getRed();
            marker.colourG_  = colour.getGreen();
            marker.colourB_  = colour.getBlue();
            marker.memo_     = "marker_test";
            dataSource_->addMarker(marker);
            break;
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
        { dynamic_cast<Component*>(musicVolumeSlider_.get()), TRANS("explanation_volume") },
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

void MainComponent::updateSpeedModeTab(SpeedModeTab tab)
{
    speedModeBasicToggleButton_->setToggleState(tab == kSpeedModeTab_Basic, dontSendNotification);
    speedModeTrainingToggleButton_->setToggleState(tab == kSpeedModeTab_Training, dontSendNotification);
    speedModeNormalComponent_->setVisible(tab == kSpeedModeTab_Basic);
    speedModeTrainingComponent_->setVisible(tab == kSpeedModeTab_Training);
}

void MainComponent::updateFileChooserTab(FileChooserTab tab)
{
    fileBrowserComponent_->setVisible(tab == kFileChooserTab_Browse);
    playlistComponent_->setVisible(tab == kFileChooserTab_Playlist);
    historyTable_->setVisible(tab == kFileChooserTab_History);
}

void MainComponent::updateListMemoTab(ListMemoTab tab)
{
    practiceTable_->setVisible(tab == kListMemoTab_Practice);
    addToListButton_->setVisible(tab == kListMemoTab_Practice);
    markerTable_->setVisible(tab == kListMemoTab_Marker);
    memoTextEditor_->setVisible(tab == kListMemoTab_Memo);
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

void MainComponent::musicVolumeChanged(float volume)
{
    musicVolumeSlider_->setValue(volume);
    const float db = 20 * log10(volume);
    String dbStr = String::formatted("%+1.1f dB", db);
    musicVolumeSlider_->setTooltip(TRANS("volume") + " : " + dbStr);
}

void MainComponent::pitchChanged(int semitone)
{
    pitchButton_->setText(MelissaUtility::getFormattedPitch(semitone));
}

void MainComponent::speedModeChanged(SpeedMode mode)
{
    updateSpeedModeTab((mode == kSpeedMode_Basic) ? kSpeedModeTab_Basic : kSpeedModeTab_Training);
}

void MainComponent::speedChanged(int speed)
{
    speedButton_->setText(String(speed) + "%");
}

void MainComponent::speedIncStartChanged(int speedIncStart)
{
    speedIncStartButton_->setText(String(speedIncStart) + "%");
}

void MainComponent::speedIncValueChanged(int speedIncValue)
{
    speedIncValueButton_->setText(String("+") + String(speedIncValue));
}

void MainComponent::speedIncPerChanged(int speedIncPer)
{
    speedIncPerButton_->setText(String(speedIncPer));
}

void MainComponent::speedIncGoalChanged(int speedIncGoal)
{
    speedIncGoalButton_->setText(String(speedIncGoal) + "%");
}

void MainComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    aButton_->setText(MelissaUtility::getFormattedTimeMSec(aTimeMSec));
    bButton_->setText(MelissaUtility::getFormattedTimeMSec(bTimeMSec));
}

void MainComponent::metronomeSwitchChanged(bool on)
{
    metronomeOnOffButton_->setToggleState(on, dontSendNotification);
    auto children = sectionComponents_[kSection_Metronome]->getChildren();
    for (auto&& c : children)
    {
        if (c != metronomeOnOffButton_.get()) c->setEnabled(on);
    }
}

void MainComponent::bpmChanged(float bpm)
{
    bpmButton_->setText(String(static_cast<uint32_t>(model_->getBpm())));
}

void MainComponent::beatPositionChanged(float beatPositionMSec)
{
    beatPositionButton_->setText(MelissaUtility::getFormattedTimeMSec(model_->getBeatPositionMSec()));
}

void MainComponent::accentUpdated(int accent)
{
    accentButton_->setText(String(accent));
}

void MainComponent::metronomeVolumeUpdated(float volume)
{
    metronomeVolumeSlider_->setValue(volume);
}

void MainComponent::musicMetronomeBalanceUpdated(float balance)
{
    volumeBalanceSlider_->setValue(balance);
}

void MainComponent::outputModeChanged(OutputMode outputMode)
{
    oututModeComboBox_->setSelectedId(outputMode + 1);
}

void MainComponent::eqSwitchChanged(bool on)
{
    eqSwitchButton_->setToggleState(on, dontSendNotification);
    auto children = sectionComponents_[kSection_Eq]->getChildren();
    for (auto&& c : children)
    {
        if (c != eqSwitchButton_.get()) c->setEnabled(on);
    }
}

void MainComponent::eqFreqChanged(size_t band, float freq)
{
    eqFreqKnobs_[band]->setValue(freq, dontSendNotification);
    knobLabels_[0]->setText(String::formatted("%d Hz", static_cast<int>(freq)), dontSendNotification);
}

void MainComponent::eqGainChanged(size_t band, float gain)
{
    eqGainKnobs_[band]->setValue(gain, dontSendNotification);
    knobLabels_[2]->setText(String::formatted("Gain: %2.1f", gain), dontSendNotification);
}

void MainComponent::eqQChanged(size_t band, float q)
{
    eqQKnobs_[band]->setValue(q, dontSendNotification);
    knobLabels_[1]->setText(String::formatted("Q:%1.2f", q), dontSendNotification);
}
