//
//  MainComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <sstream>
#include "MainComponent.h"
#include "MelissaAboutComponent.h"
#include "MelissaBPMSettingComponent.h"
#include "MelissaDefinitions.h"
#include "MelissaInputDialog.h"
#include "MelissaMobileSupport.h"
#include "MelissaOptionDialog.h"
#include "MelissaShortcutComponent.h"
#include "MelissaStemProvider.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"
#include <float.h>

using namespace juce;

#ifdef MELISSA_USE_STEM_SEPARATION
static constexpr bool isFullVersion = true;
#else
static constexpr bool isFullVersion = false;
#endif

#ifdef JUCE_IOS
static constexpr bool isDesktop = false;
#else
static constexpr bool isDesktop = true;
#include "MelissaExportComponent.h"
#endif

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
    kMenuID_About = 1000,
    kMenuID_Website,
    kMenuID_Manual,
    kMenuID_VersionCheck,
    kMenuID_Preferences,
    kMenuID_Shortcut,
    kMenuID_UITheme_Auto,
    kMenuID_UITheme_Dark,
    kMenuID_UITheme_Light,
    kMenuID_RevealSettingsFile,
    kMenuID_Tutorial,
    kMenuID_TwitterShare,
    kMenuID_FileOpen = 2000,
};

class MainComponent::HeaderComponent : public Component
{
public:
    HeaderComponent() 
    {
    }
    
    void paint(Graphics &g)
    {
        const bool isDark = MelissaUISettings::isDarkMode;

        const auto backgroundColour = isDark ? MelissaUISettings::getSubColour() : MelissaUISettings::getMainColour();
        const auto borderColour = isDark ? MelissaUISettings::getMainColour() : MelissaUISettings::getSubColour();

        g.fillAll(backgroundColour);

        g.setColour(borderColour);

        // lines
        constexpr int lineWidth = 3;
        g.fillRect(210, 9, lineWidth, 32);
        g.fillRect(210 + 190, 9, lineWidth, 32);
        g.fillRect(getWidth() - 470, 9, lineWidth, 32);

        if (!isDark)
            g.fillRect(0, getHeight() - lineWidth, getWidth(), lineWidth);

        // main volume background
        g.fillRoundedRectangle(getWidth() - 151, 15, 141, 19, 19 / 2);
    }
    
private:
};

#if defined(ENABLE_SPEED_TRAINING)
class MainComponent::SlashComponent : public Component
{
public:
    void paint(Graphics &g)
    {
        g.setColour(Colours::white.withAlpha(0.4f));
        g.drawLine(getWidth() - 4, 4, 4, getHeight() - 4, 2);
    }
};
#endif

class MainComponent::QIconComponent : public Component
{
public:
    QIconComponent(float ratio = 0.2f) : ratio_(ratio) {}

    void paint(Graphics &g)
    {
        const auto lineWidth = 2.f;
        const auto w = getWidth();
        const auto h = getHeight();
        const int sinStartX = w * (1 - ratio_) / 2;
        const int sinWidth = w * ratio_;

        Path path;
        path.startNewSubPath(0, h - lineWidth / 2);
        path.lineTo(sinStartX, h - lineWidth / 2);
        for (float rad = 0; rad < 2 * M_PI; rad += 0.01f)
        {
            auto x = sinStartX + rad / (2 * M_PI) * sinWidth;
            auto y = (1.f + cos(rad)) / 2.f * (h - lineWidth) + lineWidth / 2;
            path.lineTo(x, y);
        }
        path.lineTo(w, h - lineWidth / 2);

        g.setColour(MelissaUISettings::getTextColour(0.4f));
        g.strokePath(path, juce::PathStrokeType(lineWidth));
    }

private:
    float ratio_;
};

class MainComponent::RoundedComponent : public Component
{
public:
    RoundedComponent(const Colour &colour) : colour_(colour) {}

private:
    void paint(Graphics &g)
    {
        g.setColour(colour_);
        g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 6);
    }

    Colour colour_;
};

class MenuOverlayComponent::MenuComponent : public Component
{
public:
    class MenuButtonComponent : public Component
    {
    public:
        MenuButtonComponent(const String &text, std::function<void()> onClick)
            : text_(text)
        {
            button_ = std::make_unique<TextButton>(text);
            button_->onClick = onClick;
            button_->setLookAndFeel(&menuButtonLaf_);
            addAndMakeVisible(button_.get());
        }

        ~MenuButtonComponent() override
        {
            button_->setLookAndFeel(nullptr);
        }

        void paint(Graphics &g) override {}

        void resized() override
        {
            button_->setBounds(0, 0, getWidth(), getHeight());
        }

    private:
        std::unique_ptr<TextButton> button_;
        String text_;
        MelissaLookAndFeel_MenuButton menuButtonLaf_;
    };

    MenuComponent(bool updateAvailable) : updateAvailable_(updateAvailable)
    {
        circleToggleLaf_.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));

        addMenuLabel(TRANS("about"));
        addMenuButton(TRANS("about_melissa"), [this]()
                          {
            if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_About); });

        addMenuButton(TRANS("open_manual"), [this]()
                          {
            if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_Manual); });
        addMenuButton(TRANS("website"), [this]()
                          {
            if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_Website); });

        addMenuButton(updateAvailable_ ? TRANS("update_exists") : TRANS("check_update"), [this]()
                          {
            if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_VersionCheck); });
        addSeparator();

        addMenuLabel(TRANS("preferences"));
        addMenuButton(TRANS("shortcut_settings"), [this]()
                          {
                if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_Shortcut); });

        addMenuButton(TRANS("audio_midi_settings"), [this]()
                          {
            if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_Preferences); });

        addSeparator();

        addMenuLabel(TRANS("advanced_settings"));
        addMenuButton(TRANS("reveal_settings_file"), [this]()
                          {
            if (onMenuItemSelected != nullptr) onMenuItemSelected(kMenuID_RevealSettingsFile); });
        addSeparator();

        addMenuLabel(TRANS("ui_theme"));
        addThemeToggleButtons();

        versionLabel_ = std::make_unique<Label>();
        versionLabel_->setText("Melissa ver " + JUCEApplication::getInstance()->getApplicationVersion(), dontSendNotification);
        versionLabel_->setFont(Font(16.f));
        versionLabel_->setColour(Label::textColourId, Colours::white);
        versionLabel_->setJustificationType(Justification::centredRight);
        addAndMakeVisible(versionLabel_.get());
    }

    ~MenuComponent() override
    {
        for (auto &item : menuItems_)
        {
            if (!item.isLabel)
            {
                if (auto *button = dynamic_cast<TextButton *>(item.component.get()))
                {
                    button->setLookAndFeel(nullptr);
                }
            }
        }

        if (lightButton_ != nullptr) lightButton_->setLookAndFeel(nullptr);
        if (darkButton_ != nullptr) darkButton_->setLookAndFeel(nullptr);
    }

    void paint(Graphics &g) override
    {
        g.fillAll(MelissaUISettings::getSubColour());

        g.setColour(MelissaUISettings::getTextColour(0.3f));
        for (auto y : separatorPositions_)
        {
            g.fillRect(20, y, getWidth() - 40, 1);
        }
    }

    void resized() override
    {
        const int buttonHeight = 40;
        const int labelHeight = 30;
        const int buttonMargin = 10;
        const int menuWidth = getWidth() - 20;
        const int submenuIndent = 20;
        const int separatorHeight = 1;
        const int separatorMargin = 15;

        separatorPositions_.clear();

        int y = 20;

        for (auto &item : menuItems_)
        {
            if (item.isSeparator)
            {
                separatorPositions_.push_back(y + separatorMargin / 2);
                y += separatorMargin;
            }
            else if (item.isLabel)
            {
                item.component->setBounds(10, y, menuWidth, labelHeight);
                y += labelHeight;
            }
            else if (item.isSubmenu)
            {
                item.component->setBounds(10 + submenuIndent, y, menuWidth - submenuIndent, buttonHeight);
                y += buttonHeight + buttonMargin;
            }
            else
            {
                item.component->setBounds(10, y, menuWidth, buttonHeight);
                y += buttonHeight + buttonMargin;
            }
        }

        const int footerHeight = 20;
        versionLabel_->setBounds(10, getHeight() - footerHeight - 10, menuWidth, footerHeight);
    }

    std::function<void(int)> onMenuItemSelected;

private:
    struct MenuItem
    {
        std::unique_ptr<Component> component;
        bool isLabel;
        bool isSubmenu;
        bool isSeparator = false;
    };

    void addMenuButton(const String &text, std::function<void()> onClick)
    {
        auto button = std::make_unique<TextButton>(text);
        button->onClick = onClick;
        button->setLookAndFeel(&menuButtonLaf_);
        addAndMakeVisible(button.get());

        MenuItem item;
        item.component = std::move(button);
        item.isLabel = false;
        item.isSubmenu = false;
        menuItems_.push_back(std::move(item));
    }

    void addMenuLabel(const String &text)
    {
        auto label = std::make_unique<Label>();
        label->setText(text, dontSendNotification);
        label->setFont(Font(16.f).boldened());
        label->setColour(Label::textColourId, MelissaUISettings::getTextColour());
        label->setJustificationType(Justification::left);
        addAndMakeVisible(label.get());

        MenuItem item;
        item.component = std::move(label);
        item.isLabel = true;
        item.isSubmenu = false;
        menuItems_.push_back(std::move(item));
    }

    void addSubmenuButton(const String &text, std::function<void()> onClick)
    {
        auto button = std::make_unique<TextButton>(text);
        button->onClick = onClick;
        button->setLookAndFeel(&menuButtonLaf_);
        addAndMakeVisible(button.get());

        MenuItem item;
        item.component = std::move(button);
        item.isLabel = false;
        item.isSubmenu = true;
        menuItems_.push_back(std::move(item));
    }

    void addThemeToggleButtons()
    {
        auto themeRow = std::make_unique<Component>();

        const bool isDarkMode = MelissaUISettings::isDarkMode;

        lightButton_ = std::make_unique<ToggleButton>(TRANS("ui_theme_light"));
        lightButton_->setToggleState(!isDarkMode, dontSendNotification);
        lightButton_->onClick = [this]()
        {
            if (lightButton_->getToggleState())
            {
                darkButton_->setToggleState(false, dontSendNotification);
                if (onMenuItemSelected != nullptr)
                    onMenuItemSelected(kMenuID_UITheme_Light);
            }
            else
            {
                lightButton_->setToggleState(true, dontSendNotification);
            }
        };
        lightButton_->setBounds(10, 0, 80, 30);
        lightButton_->setLookAndFeel(&circleToggleLaf_);
        themeRow->addAndMakeVisible(lightButton_.get());

        darkButton_ = std::make_unique<ToggleButton>(TRANS("ui_theme_dark"));
        darkButton_->setToggleState(isDarkMode, dontSendNotification); // ダークモードならこちらを選択
        darkButton_->onClick = [this]()
        {
            if (darkButton_->getToggleState())
            {
                lightButton_->setToggleState(false, dontSendNotification);
                if (onMenuItemSelected != nullptr)
                    onMenuItemSelected(kMenuID_UITheme_Dark);
            }
            else
            {
                darkButton_->setToggleState(true, dontSendNotification);
            }
        };
        darkButton_->setBounds(100, 0, 80, 30);
        darkButton_->setLookAndFeel(&circleToggleLaf_);
        themeRow->addAndMakeVisible(darkButton_.get());

        themeRow->setSize(getWidth(), 40);
        addAndMakeVisible(themeRow.get());

        themeRow->setSize(getWidth(), 40);
        addAndMakeVisible(themeRow.get());

        MenuItem item;
        item.component = std::move(themeRow);
        item.isLabel = false;
        item.isSubmenu = false;
        menuItems_.push_back(std::move(item));
    }

    std::vector<MenuItem> menuItems_;
    std::unique_ptr<Label> versionLabel_;
    std::unique_ptr<ToggleButton> lightButton_;
    std::unique_ptr<ToggleButton> darkButton_;
    bool updateAvailable_;
    MelissaLookAndFeel_MenuButton menuButtonLaf_;
    MelissaLookAndFeel_CircleToggleButton circleToggleLaf_;
    std::vector<int> separatorPositions_;

    void addSeparator()
    {
        MenuItem item;
        item.component = nullptr;
        item.isLabel = false;
        item.isSubmenu = false;
        item.isSeparator = true;
        menuItems_.push_back(std::move(item));
    }
};

MenuOverlayComponent::MenuOverlayComponent() : menuVisible_(false), menuPosX_(-300)
{
    setVisible(false);

    menuComponent_ = std::make_unique<MenuComponent>(MelissaUpdateChecker::getUpdateStatus() == MelissaUpdateChecker::kUpdateStatus_UpdateExists);
    menuComponent_->onMenuItemSelected = [this](int menuId)
    {
        if (onMenuItemSelected != nullptr)
            onMenuItemSelected(menuId);
        showMenu(false);
    };
    addAndMakeVisible(menuComponent_.get());

    startTimerHz(60);
}

MenuOverlayComponent::~MenuOverlayComponent()
{
    stopTimer();
}

void MenuOverlayComponent::paint(Graphics &g)
{
    g.fillAll(Colours::black.withAlpha(0.6f));
}

void MenuOverlayComponent::resized()
{
    menuArea_ = getLocalBounds().withWidth(300);
    menuComponent_->setBounds(static_cast<int>(menuPosX_), 0, menuArea_.getWidth(), getHeight());
}

void MenuOverlayComponent::mouseDown(const MouseEvent &e)
{
    if (!menuArea_.contains(e.getPosition()))
    {
        showMenu(false);
    }
}

void MenuOverlayComponent::showMenu(bool show)
{
    menuVisible_ = show;
    setVisible(true);

    if (!show && onMenuClosed != nullptr)
    {
        Timer::callAfterDelay(300, [this]()
                              { if (onMenuClosed != nullptr) onMenuClosed(); });
    }
}

void MenuOverlayComponent::timerCallback()
{
    const float targetX = menuVisible_ ? 0 : -menuArea_.getWidth();
    const float speed = 0.3f;

    menuPosX_ = menuPosX_ * (1.f - speed) + targetX * speed;

    if (!menuVisible_ && std::abs(menuPosX_ - targetX) < 1.f)
    {
        menuPosX_ = targetX;
        setVisible(false);
    }

    menuComponent_->setBounds(static_cast<int>(menuPosX_), 0, menuArea_.getWidth(), getHeight());
}

MainComponent::MainComponent(const String &commandLine) : Thread("MelissaProcessThread"), mainVolume_(1.f),
#ifdef JUCE_IOS
                                                          showAllControlIcons_(false), currentControlPage_(kControlPage_List),
#endif
                                                          nextFileNameShown_(false), shouldExit_(false), isLangJapanese_(false), requestedKeyboardFocusOnFirstLaunch_(false), prepareingNextSong_(false)
{
    audioEngine_ = std::make_unique<MelissaAudioEngine>();
    metronome_ = std::make_unique<MelissaMetronome>();

    model_ = MelissaModel::getInstance();
    model_->setMelissaAudioEngine(audioEngine_.get());
    model_->addListener(dynamic_cast<MelissaModelListener *>(audioEngine_.get()));
    model_->addListener(this);

    isLangJapanese_ = (SystemStats::getDisplayLanguage() == "ja-JP");

    dataSource_ = MelissaDataSource::getInstance();
    dataSource_->setMelissaAudioEngine(audioEngine_.get());
    dataSource_->addListener(this);

    // load setting file
    settingsDir_ = (File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Melissa"));
    if (!(settingsDir_.exists() && settingsDir_.isDirectory()))
        settingsDir_.createDirectory();

    bool isFirstLaunch = false;
    settingsFile_ = settingsDir_.getChildFile("Settings.json");
    if (!settingsFile_.existsAsFile())
    {
        isFirstLaunch = true;
    }
    dataSource_->loadSettingsFile(settingsFile_);

    auto xmlDocument = XmlDocument::parse(dataSource_->global_.device_);
    deviceManager.initialise(0, 2, xmlDocument.get(), true);
    deviceManager.addMidiInputDeviceCallback("", this);
    deviceManager.addChangeListener(this);

    bpmDetector_ = std::make_unique<MelissaBPMDetector>();
    analyzedBpm_ = -1.f;
    analyzedBeatPosMSec_ = 0.f;
    bpmAnalyzeFinished_ = true;
    shouldInitializeBpmDetector_ = false;
    shouldUpdateBpm_ = false;

    MelissaUISettings::isDarkMode = dataSource_->getUITheme() == "System_Dark";

    simpleTextButtonLaf_.setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub));
    laf_.updateColour();
    browserLaf_.updateColour();

    getLookAndFeel().setDefaultSansSerifTypefaceName(dataSource_->getFontName());

    String localizedStrings = "";
    if (isLangJapanese_)
    {
#ifdef DEBUG
        File file = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getChildFile("Resource/Language/ja-JP.txt");
        printf("%s\n", file.getFullPathName().toRawUTF8());
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

    setAudioChannels(0, 2, xmlDocument.get());
    Thread::addListener(this);
    startThread();
    startTimer(1000 / 10);

    addKeyListener(this);

    auto rootDir = File(dataSource_->global_.rootDir_);
    rootDir.setAsCurrentWorkingDirectory();
    fileBrowserComponent_->setRoot(rootDir);

    auto width = dataSource_->global_.width_;
    auto height = dataSource_->global_.height_;
    setSize(width, height);

    model_->setPlaybackMode(static_cast<PlaybackMode>(dataSource_->global_.playMode_));
    MelissaShortcutManager::getInstance()->addListener(this);
    MelissaStemProvider::getInstance()->addListener(this);

    File fileToOpen(commandLine);
    if (fileToOpen.existsAsFile())
    {
        dataSource_->loadFileAsync(commandLine);
    }
    else
    {
        if (commandLine != "--relaunch" && commandLine.isNotEmpty())
        {
            const std::vector<String> options = {TRANS("ok")};
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("file_not_found") + "\n" + commandLine, options, nullptr);
            MelissaModalDialog::show(dialog, "Melissa", false);
        }
        else
        {
            dataSource_->restorePreviousState();
        }
    }

    uiState_ = dataSource_->getPreviousUIState();
    updateFileChooserTab(static_cast<FileChooserTab>(uiState_.selectedFileBrowserTab_));
    playlistComponent_->select(uiState_.selectedPlaylist_);

    if (isFirstLaunch)
    {
        dataSource_->setDefaultShortcuts(true);
        const std::vector<String> options = {TRANS("ok")};
        auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("first_launch"), options, [&](size_t yesno) { /* showFileChooser(); */ });
        MelissaModalDialog::show(dialog, "Melissa", false);
    }

#if defined(ENABLE_TUTORIAL)
    if (isFirstLaunch)
    {
        auto component = std::make_shared<MelissaOkCancelDialog>(this, TRANS("first_message"), [&]()
                                                                 { showTutorial(); });
        MelissaModalDialog::show(component, TRANS("tutorial"));
    }
#endif

    updatePlayBackModeButton();

    auto currentAudioDevice = deviceManager.getCurrentAudioDevice();
    if (currentAudioDevice != nullptr)
        audioDeviceButton_->setAudioDeviceName(currentAudioDevice->getName());
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();

    setLookAndFeel(nullptr);
    resetButton_->setLookAndFeel(nullptr);
    songDetailButton_->setLookAndFeel(nullptr);
    volumeBalanceSlider_->setLookAndFeel(nullptr);
    metronomeOnOffButton_->setLookAndFeel(nullptr);
    eqSwitchButton_->setLookAndFeel(nullptr);
#if defined(ENABLE_SPEED_TRAINING)
    speedModeBasicToggleButton_->setLookAndFeel(nullptr);
    speedModeTrainingToggleButton_->setLookAndFeel(nullptr);
#endif
    for (auto &&b : speedPresetButtons_)
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
    fileBrowserComponent_->setLookAndFeel(nullptr);

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

    {
        iconImages_[kIcon_Prev] = Drawable::createFromImageData(BinaryData::prev_button_svg, BinaryData::prev_button_svgSize);
        iconHighlightedImages_[kIcon_Prev] = Drawable::createFromImageData(BinaryData::prev_button_svg, BinaryData::prev_button_svgSize);
        iconColorInfo_[kIcon_Prev] = kColorInfo_None;

        iconImages_[kIcon_Next] = Drawable::createFromImageData(BinaryData::next_button_svg, BinaryData::next_button_svgSize);
        iconHighlightedImages_[kIcon_Next] = Drawable::createFromImageData(BinaryData::next_button_svg, BinaryData::next_button_svgSize);
        iconColorInfo_[kIcon_Next] = kColorInfo_None;

        iconImages_[kIcon_LoopOneSong] = Drawable::createFromImageData(BinaryData::loop_onesong_svg, BinaryData::loop_onesong_svgSize);
        iconHighlightedImages_[kIcon_LoopOneSong] = Drawable::createFromImageData(BinaryData::loop_onesong_svg, BinaryData::loop_onesong_svgSize);
        iconColorInfo_[kIcon_LoopOneSong] = kColorInfo_None;

        iconImages_[kIcon_LoopPlaylist] = Drawable::createFromImageData(BinaryData::loop_playlist_svg, BinaryData::loop_playlist_svgSize);
        iconHighlightedImages_[kIcon_LoopPlaylist] = Drawable::createFromImageData(BinaryData::loop_playlist_svg, BinaryData::loop_playlist_svgSize);
        iconColorInfo_[kIcon_LoopPlaylist] = kColorInfo_None;

        iconImages_[kIcon_ArrowLeft] = Drawable::createFromImageData(BinaryData::arrow_left_svg, BinaryData::arrow_left_svgSize);
        iconHighlightedImages_[kIcon_ArrowLeft] = Drawable::createFromImageData(BinaryData::arrow_left_svg, BinaryData::arrow_left_svgSize);
        iconColorInfo_[kIcon_ArrowLeft] = kColorInfo_WhiteToAccent;

        iconImages_[kIcon_ArrowRight] = Drawable::createFromImageData(BinaryData::arrow_right_svg, BinaryData::arrow_right_svgSize);
        iconHighlightedImages_[kIcon_ArrowRight] = Drawable::createFromImageData(BinaryData::arrow_right_svg, BinaryData::arrow_right_svgSize);
        iconColorInfo_[kIcon_ArrowRight] = kColorInfo_WhiteToAccent;

        iconImages_[kIcon_Add] = Drawable::createFromImageData(BinaryData::add_svg, BinaryData::add_svgSize);
        iconHighlightedImages_[kIcon_Add] = Drawable::createFromImageData(BinaryData::add_svg, BinaryData::add_svgSize);
        iconColorInfo_[kIcon_Add] = kColorInfo_WhiteToAccent;

        iconImages_[kIcon_Up] = Drawable::createFromImageData(BinaryData::up_svg, BinaryData::up_svgSize);
        iconHighlightedImages_[kIcon_Up] = Drawable::createFromImageData(BinaryData::up_svg, BinaryData::up_svgSize);
        iconColorInfo_[kIcon_Up] = kColorInfo_WhiteToMain;

        iconImages_[kIcon_Down] = Drawable::createFromImageData(BinaryData::down_svg, BinaryData::down_svgSize);
        iconHighlightedImages_[kIcon_Down] = Drawable::createFromImageData(BinaryData::down_svg, BinaryData::down_svgSize);
        iconColorInfo_[kIcon_Down] = kColorInfo_WhiteToMain;

        iconImages_[kIcon_Detail] = Drawable::createFromImageData(BinaryData::detail_svg, BinaryData::detail_svgSize);
        iconHighlightedImages_[kIcon_Detail] = Drawable::createFromImageData(BinaryData::detail_svg, BinaryData::detail_svgSize);
        iconColorInfo_[kIcon_Detail] = kColorInfo_WhiteToAccent;

        iconImages_[kIcon_Select] = Drawable::createFromImageData(BinaryData::select_svg, BinaryData::select_svgSize);
        iconHighlightedImages_[kIcon_Select] = Drawable::createFromImageData(BinaryData::select_svg, BinaryData::select_svgSize);
        iconColorInfo_[kIcon_Select] = kColorInfo_WhiteToAccent;

        iconImages_[kIcon_Export] = Drawable::createFromImageData(BinaryData::export_svg, BinaryData::export_svgSize);
        iconHighlightedImages_[kIcon_Export] = Drawable::createFromImageData(BinaryData::export_svg, BinaryData::export_svgSize);
        iconColorInfo_[kIcon_Export] = kColorInfo_None;

        iconImages_[kIcon_Trim] = Drawable::createFromImageData(BinaryData::trim_svg, BinaryData::trim_svgSize);
        iconHighlightedImages_[kIcon_Trim] = Drawable::createFromImageData(BinaryData::trim_svg, BinaryData::trim_svgSize);
        iconColorInfo_[kIcon_Trim] = kColorInfo_None;

        for (int iconIndex = 0; iconIndex < kNumOfIcons; ++iconIndex)
        {
            if (iconColorInfo_[iconIndex] == kColorInfo_None)
            {
                iconImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getTextColour(0.8f));
                iconHighlightedImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getTextColour());
            }
            else if (iconColorInfo_[iconIndex] == kColorInfo_WhiteToAccent)
            {
                iconImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getAccentColour(0.8f));
                iconHighlightedImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getAccentColour());
            }
            else if (iconColorInfo_[iconIndex] == kColorInfo_WhiteToMain)
            {
                iconImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getMainColour());
                iconImages_[iconIndex]->replaceColour(Colours::black, MelissaUISettings::getTextColour(0.6f));
                iconHighlightedImages_[iconIndex]->replaceColour(Colours::white, MelissaUISettings::getMainColour());
                iconHighlightedImages_[iconIndex]->replaceColour(Colours::black, MelissaUISettings::getTextColour());
            }
        }
    }

    menuBar_ = make_unique<MenuBarComponent>(this);
    addAndMakeVisible(menuBar_.get());

#if JUCE_MAC
    extraAppleMenuItems_ = make_unique<PopupMenu>();
    extraAppleMenuItems_->addItem(TRANS("about_melissa"), [&]()
                                  { showAboutDialog(); });
    extraAppleMenuItems_->addItem(TRANS("preferences"), [&]()
                                  { showAudioMidiSettingsDialog(); });

    MenuBarModel::setMacMainMenu(this, extraAppleMenuItems_.get());
#elif JUCE_IOS
    safeAreaComponent_ = std::make_unique<Label>();
    // safeAreaComponent_->setColour(Label::outlineColourId, Colours::pink.withAlpha(0.5f));
    safeAreaComponent_->toBack();
    addAndMakeVisible(safeAreaComponent_.get());
#endif

    headerComponent_ = std::make_unique<HeaderComponent>();
    addAndMakeVisible(headerComponent_.get());

    createMenu();

    {
        // Header
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(headerComponent_.get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());
        componentToAdd->addAndMakeVisible(menuButton_.get());

        playbackModeButton_ = make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        playbackModeButton_->setImages(iconImages_[kIcon_LoopOneSong].get(), iconHighlightedImages_[kIcon_LoopOneSong].get());
        playbackModeButton_->onClick = [&]()
        {
            if (model_->getPlaybackMode() == kPlaybackMode_LoopOneSong)
            {
                model_->setLoopPosRatio(0.f, 1.f);
                model_->setPlaybackMode(kPlaybackMode_LoopPlaylistSongs);
            }
            else
            {
                model_->setPlaybackMode(kPlaybackMode_LoopOneSong);
            }
            updatePlayBackModeButton();
        };
        componentToAdd->addAndMakeVisible(playbackModeButton_.get());

        playPauseButton_ = make_unique<MelissaPlayPauseButton>("PlayButton");
        playPauseButton_->onClick = [this]()
        { model_->togglePlaybackStatus(); };
        componentToAdd->addAndMakeVisible(playPauseButton_.get());

        prevButton_ = make_unique<DrawableButton>("PrevButton", DrawableButton::ImageRaw);
        prevButton_->setImages(iconImages_[kIcon_Prev].get(), iconHighlightedImages_[kIcon_Prev].get());
        prevButton_->onClick = [this]()
        { prev(); };
        componentToAdd->addAndMakeVisible(prevButton_.get());

        nextButton_ = make_unique<DrawableButton>("NextButton", DrawableButton::ImageRaw);
        nextButton_->setImages(iconImages_[kIcon_Next].get(), iconHighlightedImages_[kIcon_Next].get());
        nextButton_->onClick = [this]()
        { next(); };
        componentToAdd->addAndMakeVisible(nextButton_.get());

        timeLabel_ = make_unique<Label>();
        timeLabel_->setColour(Label::textColourId, MelissaUISettings::getTextColour());
        timeLabel_->setJustificationType(Justification::centred);
        timeLabel_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Main));
        componentToAdd->addAndMakeVisible(timeLabel_.get());

        fileNameLabel_ = make_unique<Label>();
        fileNameLabel_->setJustificationType(Justification::centred);
        fileNameLabel_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Large));
        componentToAdd->addAndMakeVisible(fileNameLabel_.get());

        audioDeviceButton_ = make_unique<MelissaAudioDeviceButton>();
        audioDeviceButton_->onClick = [&]()
        {
            showAudioMidiSettingsDialog();
        };
        componentToAdd->addAndMakeVisible(audioDeviceButton_.get());

        trimButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        trimButton_->setTooltip(TRANS("trim"));
        trimButton_->setImages(iconImages_[kIcon_Trim].get(), iconHighlightedImages_[kIcon_Trim].get());
        trimButton_->onClick = [&]() {};
        // componentToAdd->addAndMakeVisible(trimButton_.get());

        exportButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        exportButton_->setTooltip(TRANS("export"));
        exportButton_->setImages(iconImages_[kIcon_Export].get(), iconHighlightedImages_[kIcon_Export].get());
        exportButton_->onClick = [&]()
        {
#ifndef JUCE_IOS
            auto component = std::make_shared<MelissaExportComponent>();
            component->setSize(800, 220);
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("export"));
#endif
        };
        componentToAdd->addAndMakeVisible(exportButton_.get());

        exportProgressBar_ = std::make_unique<MelissaProgressBarComponent>();
        componentToAdd->addChildComponent(exportProgressBar_.get());

        mainVolumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        mainVolumeSlider_->setTooltip(TRANS("volume_main"));
        mainVolumeSlider_->setRange(0.01f, 1.0f);
        mainVolumeSlider_->setDoubleClickReturnValue(true, 1.f);
        mainVolumeSlider_->setValue(1.f);
        mainVolumeSlider_->onValueChange = [this]()
        {
            model_->setMainVolume(mainVolumeSlider_->getValue());
        };
        componentToAdd->addAndMakeVisible(mainVolumeSlider_.get());
    }

    {
        Component *componentToAdd = isDesktop ? this : reinterpret_cast<Component *>(safeAreaComponent_.get());

        waveformHolderComponent_ = make_unique<Component>();
        // componentToAdd->addAndMakeVisible(waveformHolderComponent_.get());

        waveformComponent_ = make_unique<MelissaWaveformControlComponent>();
        waveformHolderComponent_->addAndMakeVisible(waveformComponent_.get());

        markerMemoComponent_ = make_unique<MelissaMarkerMemoComponent>();
        markerMemoComponent_->setMarkerListener(this);
        markerMemoComponent_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Main));
        waveformHolderComponent_->addAndMakeVisible(markerMemoComponent_.get());

        waveformViewport_ = make_unique<Viewport>();
        waveformViewport_->setScrollBarThickness(kScrollbarThichness);
        componentToAdd->addAndMakeVisible(waveformViewport_.get());
        waveformViewport_->setViewedComponent(waveformHolderComponent_.get(), false);

        popupMessage_ = std::make_unique<MelissaPopupMessageComponent>();
        componentToAdd->addChildComponent(popupMessage_.get());

        controlComponent_ = make_unique<Label>();
        controlComponent_->setOpaque(false);
        controlComponent_->setColour(Label::backgroundColourId, MelissaUISettings::getSubColour());
        componentToAdd->addAndMakeVisible(controlComponent_.get());

        String sectionTitles[kNumOfSections] =
            {
                "Song",
                "Loop",
                "Speed",
                "Metronome",
                "EQ",
                "Mixer"};
        for (size_t sectionIndex = 0; sectionIndex < kNumOfSections; ++sectionIndex)
        {
            auto s = std::make_unique<MelissaSectionComponent>(sectionTitles[sectionIndex]);
            componentToAdd->addAndMakeVisible(s.get());
            sectionComponents_[sectionIndex] = std::move(s);
        }
    }

    {
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(sectionComponents_[kSection_Song].get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        pitchButton_ = make_unique<MelissaIncDecButton>(16, TRANS("tooltip_pitch_dec"), TRANS("tooltip_pitch_inc"));
        pitchButton_->setText("Original");
        pitchButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                model_->setPitch(0);
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setPitch(model_->getPitch() + sign * (b ? 0.1 : 1));
            }
        };
        componentToAdd->addAndMakeVisible(pitchButton_.get());

        // Stem Button
        stemControlComponent_ = make_unique<MelissaStemControlComponent>();
        componentToAdd->addAndMakeVisible(stemControlComponent_.get());

        songDetailButton_ = make_unique<TextButton>();
        songDetailButton_->setButtonText("...");
        songDetailButton_->setTooltip(TRANS("song_detail_menu"));
        songDetailButton_->onClick = [this]()
        {
            const std::vector<String> confirmOptions = { TRANS("ok"), TRANS("cancel") };
            auto confirmDialog = std::make_shared<MelissaOptionDialog>(TRANS("confirm_remove_stems"), confirmOptions, [this](size_t confirmIndex) {
                if (confirmIndex == 0)
                {
                    MelissaStemProvider::getInstance()->deleteStems();
                    dataSource_->loadFileAsync(dataSource_->getCurrentSongFilePath());
                }
            });
            MelissaModalDialog::show(confirmDialog, TRANS("confirm"));
        };
        songDetailButton_->setLookAndFeel(&simpleTextButtonLaf_);
        componentToAdd->addAndMakeVisible(songDetailButton_.get());
    }

    {
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(sectionComponents_[kSection_Loop].get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        aButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_loop_start_dec"), TRANS("tooltip_loop_start_inc"));
        aButton_->setText("-:--");
        aButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Left, "A", TRANS("tooltip_loop_start_set"));
        aButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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
                model_->setLoopAPosMSec(model_->getLoopAPosMSec() + sign * (b ? 100 : 1000));
            }
        };
        componentToAdd->addAndMakeVisible(aButton_.get());

        bButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_loop_end_dec"), TRANS("tooltip_loop_end_inc"));
        bButton_->setText("-:--");
        bButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Left, "B", TRANS("tooltip_loop_end_set"));
        bButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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
                model_->setLoopBPosMSec(model_->getLoopBPosMSec() + sign * (b ? 100 : 1000));
            }
        };
        componentToAdd->addAndMakeVisible(bButton_.get());

        aResetButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        aResetButton_->setTooltip(TRANS("tooltip_loop_start_reset"));
        aResetButton_->setImages(iconImages_[kIcon_ArrowLeft].get(), iconHighlightedImages_[kIcon_ArrowLeft].get());
        aResetButton_->onClick = [&]()
        {
            model_->setLoopAPosRatio(0.f);
        };
        componentToAdd->addAndMakeVisible(aResetButton_.get());

        bResetButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
        bResetButton_->setTooltip(TRANS("tooltip_loop_end_reset"));
        bResetButton_->setImages(iconImages_[kIcon_ArrowRight].get(), iconHighlightedImages_[kIcon_ArrowRight].get());
        bResetButton_->onClick = [&]()
        {
            model_->setLoopBPosRatio(1.f);
        };
        componentToAdd->addAndMakeVisible(bResetButton_.get());

        preCountOnOffButton_ = make_unique<ToggleButton>();
#if defined(ENABLE_PRECOUNT)
        preCountOnOffButton_->setTooltip(TRANS("precount_switch"));
        preCountOnOffButton_->setClickingTogglesState(true);
        preCountOnOffButton_->setLookAndFeel(&slideToggleLaf_);
        preCountOnOffButton_->onClick = [this]()
        {
            const auto on = preCountOnOffButton_->getToggleState();
            model_->setPreCountSwitch(on);
        };
        componentToAdd->addAndMakeVisible(preCountOnOffButton_.get());
#endif

        preCountSettingButton_ = make_unique<TextButton>();
#if defined(ENABLE_PRECOUNT)
        preCountSettingButton_->setButtonText("Pre-count");
        preCountSettingButton_->setTooltip(TRANS("precount_setting"));
        preCountSettingButton_->onClick = [this]()
        { resetLoop(); };
        preCountSettingButton_->setLookAndFeel(&simpleTextButtonLaf_);
        preCountSettingButton_->onClick = [this]()
        {
            auto component = std::make_shared<MelissaPreCountSettingComponent>();
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("precount_settings"));
        };
        componentToAdd->addAndMakeVisible(preCountSettingButton_.get());
#endif

        resetButton_ = make_unique<TextButton>();
        resetButton_->setButtonText("Reset");
        resetButton_->setTooltip(TRANS("tooltip_loop_reset"));
        resetButton_->onClick = [this]()
        { resetLoop(); };
        resetButton_->setLookAndFeel(&simpleTextButtonLaf_);
        componentToAdd->addAndMakeVisible(resetButton_.get());
    }

    {
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(sectionComponents_[kSection_Speed].get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        speedModeNormalComponent_ = make_unique<Component>();
        componentToAdd->addAndMakeVisible(speedModeNormalComponent_.get());

#if defined(ENABLE_SPEED_TRAINING)
        speedModeTrainingComponent_ = make_unique<Component>();
        componentToAdd->addAndMakeVisible(speedModeTrainingComponent_.get());

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
        componentToAdd->addAndMakeVisible(speedModeBasicToggleButton_.get());

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
        componentToAdd->addAndMakeVisible(speedModeTrainingToggleButton_.get());
#endif

        componentToAdd = isDesktop ? reinterpret_cast<Component *>(speedModeNormalComponent_.get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        speedButton_ = make_unique<MelissaIncDecButton>(2, TRANS("tooltip_speed_dec"), TRANS("tooltip_speed_inc"));
        speedButton_->setText("100 %");
        speedButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                model_->setSpeed(100);
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setSpeed(model_->getSpeed() + (b ? sign * 10 : sign));
            }
        };
        componentToAdd->addAndMakeVisible(speedButton_.get());

        speedPresetViewport_ = make_unique<Viewport>();
        speedPresetViewport_->setScrollBarThickness(kScrollbarThichness);
        componentToAdd->addAndMakeVisible(speedPresetViewport_.get());

        speedPresetComponent_ = make_unique<Component>();
        componentToAdd->addAndMakeVisible(speedPresetComponent_.get());
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
            b->onClick = [&, speed]()
            { model_->setSpeed(speed); };
            const auto width = MelissaUtility::getStringSize(simpleTextButtonLaf_.getFont(), b->getButtonText()).first;
            speedButtonWidthSum += width;
            b->setBounds(speedButtonX, 0, width, 30);
            speedButtonX += (width + presetButtonMargin);
            speedPresetComponent_->addAndMakeVisible(b.get());
            speedPresetButtons_[speedPresetIndex] = std::move(b);
        }

        speedPresetComponent_->setSize(speedButtonWidthSum + presetButtonMargin * (kNumOfSpeedPresets - 1), controlHeight - kScrollbarThichness);
        speedPresetViewport_->setViewedComponent(speedPresetComponent_.get(), false);

#if defined(ENABLE_SPEED_TRAINING)
        speedIncStartButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_speed_training_start_dec"), TRANS("tooltip_speed_training_start_inc"));
        speedIncStartButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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

        speedIncPerButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_speed_training_per_dec"), TRANS("tooltip_speed_training_per_inc"));
        speedIncPerButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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

        slashComponent_ = make_unique<SlashComponent>();
        speedModeTrainingComponent_->addAndMakeVisible(slashComponent_.get());

        speedIncValueButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_speed_training_inc_dec"), TRANS("tooltip_speed_training_inc_inc"));
        speedIncValueButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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

        speedIncGoalButton_ = make_unique<MelissaIncDecButton>(1, TRANS("tooltip_speed_training_goal_dec"), TRANS("tooltip_speed_training_goal_inc"));
        speedIncGoalButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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

        resetSpeedTrainingButton_ = make_unique<TextButton>();
        resetSpeedTrainingButton_->setButtonText("Reset");
        resetSpeedTrainingButton_->setLookAndFeel(&simpleTextButtonLaf_);
        resetSpeedTrainingButton_->setTooltip(TRANS("tooltip_reset_speed_training"));
        resetSpeedTrainingButton_->onClick = [this]() { model_->resetSpeedTraining(); };
        speedModeTrainingComponent_->addAndMakeVisible(resetSpeedTrainingButton_.get());

        speedProgressComponent_ = make_unique<MelissaSpeedTrainingProgressComponent>();
        speedProgressComponent_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Small));
        speedModeTrainingComponent_->addAndMakeVisible(speedProgressComponent_.get());
#endif
    }

    {
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(sectionComponents_[kSection_Metronome].get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        metronomeOnOffButton_ = make_unique<ToggleButton>();
        metronomeOnOffButton_->setTooltip(TRANS("metronome_switch"));
        metronomeOnOffButton_->setClickingTogglesState(true);
        metronomeOnOffButton_->setLookAndFeel(&slideToggleLaf_);
        metronomeOnOffButton_->onClick = [this]()
        {
            const auto on = metronomeOnOffButton_->getToggleState();
            model_->setMetronomeSwitch(on);
        };
        componentToAdd->addAndMakeVisible(metronomeOnOffButton_.get());

        bpmButton_ = make_unique<MelissaIncDecButton>(1, TRANS("metronome_bpm_dec"), TRANS("metronome_bpm_inc"));
        bpmButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Right, "Edit", TRANS("metronome_bpm_edit"));
        bpmButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
        {
            if (event == MelissaIncDecButton::kEvent_Double)
            {
                shouldInitializeBpmDetector_ = true;
                model_->setBpm(kBpmShouldMeasure);
            }
            else if (event == MelissaIncDecButton::kEvent_Func)
            {
                showBPMSettingDialog();
            }
            else
            {
                const int sign = (event == MelissaIncDecButton::kEvent_Inc) ? 1 : -1;
                model_->setBpm(std::clamp<int>(model_->getBpm() + sign, kBpmMin, kBpmMax));
            }
        };
        componentToAdd->addAndMakeVisible(bpmButton_.get());

        accentPositionButton_ = make_unique<MelissaIncDecButton>(1, TRANS("metronome_accent_position_dec"), TRANS("metronome_accent_position_inc"));
        accentPositionButton_->addFunctionButton(MelissaIncDecButton::kButtonPosition_Right, "Set", TRANS("metronome_accent_position_set"));
        accentPositionButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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
        componentToAdd->addAndMakeVisible(accentPositionButton_.get());

        accentButton_ = make_unique<MelissaIncDecButton>(1, TRANS("metronome_accent_dec"), TRANS("metronome_accent_inc"));
        accentButton_->onClick_ = [this](MelissaIncDecButton::Event event, bool b)
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
        componentToAdd->addAndMakeVisible(accentButton_.get());
    }

    {
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(sectionComponents_[kSection_Eq].get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        eqSwitchButton_ = std::make_unique<ToggleButton>();
        eqSwitchButton_->setTooltip(TRANS("eq_switch"));
        eqSwitchButton_->setClickingTogglesState(true);
        eqSwitchButton_->setLookAndFeel(&slideToggleLaf_);
        eqSwitchButton_->onClick = [this]()
        {
            const auto on = eqSwitchButton_->getToggleState();
            model_->setEqSwitch(on);
        };
        componentToAdd->addAndMakeVisible(eqSwitchButton_.get());

        for (size_t bandIndex = 0; bandIndex < kNumOfEqBands; ++bandIndex)
        {
            auto freqKnob = std::make_unique<Slider>();
            freqKnob->setTooltip(TRANS("eq_freq"));
            freqKnob->setSliderStyle(Slider::RotaryVerticalDrag);
            freqKnob->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            freqKnob->setRange(0.f, 1.f);
            freqKnob->onValueChange = [&, bandIndex]()
            {
                auto value = eqFreqKnobs_[bandIndex]->getValue();
                model_->setEqFreq(0, 20 * std::pow(1000, value));
            };
            componentToAdd->addAndMakeVisible(freqKnob.get());
            eqFreqKnobs_[bandIndex] = std::move(freqKnob);

            auto qKnob = std::make_unique<Slider>();
            qKnob->setTooltip(TRANS("eq_q"));
            qKnob->setSliderStyle(Slider::RotaryVerticalDrag);
            qKnob->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            qKnob->setRange(kEqQMin, kEqQMax);
            qKnob->onValueChange = [&, bandIndex]()
            {
                auto value = eqQKnobs_[bandIndex]->getValue();
                model_->setEqQ(0, value);
            };
            componentToAdd->addAndMakeVisible(qKnob.get());
            eqQKnobs_[bandIndex] = std::move(qKnob);

            auto gainKnob = std::make_unique<Slider>();
            gainKnob->setTooltip(TRANS("eq_gain"));
            gainKnob->setSliderStyle(Slider::RotaryVerticalDrag);
            gainKnob->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            gainKnob->setRange(kEqGainMin, kEqGainMax);
            gainKnob->onValueChange = [&, bandIndex]()
            {
                auto value = eqGainKnobs_[bandIndex]->getValue();
                model_->setEqGain(0, value);
            };
            componentToAdd->addAndMakeVisible(gainKnob.get());
            eqGainKnobs_[bandIndex] = std::move(gainKnob);
        }

        constexpr int numOfControls = 3;
        const static String labelTitles[] = {"Freq", "Gain", "Q"};
        for (size_t labelIndex = 0; labelIndex < kNumOfEqBands * numOfControls; ++labelIndex)
        {
            auto l = std::make_unique<Label>();
            l->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub));
            l->setText(labelTitles[labelIndex % numOfControls], dontSendNotification);
            l->setJustificationType(Justification::centred);
            l->setColour(Label::textColourId, MelissaUISettings::getTextColour());
            l->toBack();
            componentToAdd->addAndMakeVisible(l.get());
            knobLabels_[labelIndex] = std::move(l);
        }

        for (size_t qIconIndex = 0; qIconIndex < 2; ++qIconIndex)
        {
            qIconComponents_[qIconIndex] = std::make_unique<QIconComponent>(0.25 + 0.5 * qIconIndex);
            componentToAdd->addAndMakeVisible(qIconComponents_[qIconIndex].get());
        }
    }

    {
        Component *componentToAdd = isDesktop ? reinterpret_cast<Component *>(sectionComponents_[kSection_Mixer].get()) : reinterpret_cast<Component *>(safeAreaComponent_.get());

        outputModeComboBox_ = make_unique<ComboBox>();
        outputModeComboBox_->setTooltip(TRANS("output_mode"));
        outputModeComboBox_->setJustificationType(Justification::centred);
        outputModeComboBox_->addItem("Original", kOutputMode_LR + 1);
        outputModeComboBox_->addItem("L - L", kOutputMode_LL + 1);
        outputModeComboBox_->addItem("R - R", kOutputMode_RR + 1);
        outputModeComboBox_->addItem("Center Cancel", kOutputMode_CenterCancel + 1);
        outputModeComboBox_->onChange = [&]()
        {
            OutputMode mode = static_cast<OutputMode>(outputModeComboBox_->getSelectedId() - 1);
            model_->setOutputMode(mode);
        };
        outputModeComboBox_->setSelectedId(kOutputMode_LR + 1);
        outputModeComboBox_->setWantsKeyboardFocus(false);
        componentToAdd->addAndMakeVisible(outputModeComboBox_.get());

        musicVolumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        musicVolumeSlider_->setTooltip(TRANS("volume_music"));
        musicVolumeSlider_->setRange(0.01f, 2.0f);
        musicVolumeSlider_->setDoubleClickReturnValue(true, 1.f);
        musicVolumeSlider_->setValue(1.f);
        musicVolumeSlider_->onValueChange = [this]()
        {
            model_->setMusicVolume(musicVolumeSlider_->getValue());
        };
        componentToAdd->addAndMakeVisible(musicVolumeSlider_.get());

        volumeBalanceSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        volumeBalanceSlider_->setTooltip(TRANS("volume_balance"));
        volumeBalanceSlider_->setRange(0.f, 1.0f);
        volumeBalanceSlider_->setDoubleClickReturnValue(true, 0.5f);
        volumeBalanceSlider_->setValue(0.5);
        volumeBalanceSlider_->setLookAndFeel(&crossFaderLaf_);
        volumeBalanceSlider_->onValueChange = [this]()
        {
            model_->setMusicMetronomeBalance(volumeBalanceSlider_->getValue());
        };
        componentToAdd->addAndMakeVisible(volumeBalanceSlider_.get());

        metronomeVolumeSlider_ = make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
        metronomeVolumeSlider_->setTooltip(TRANS("volume_metronome"));
        metronomeVolumeSlider_->setRange(0.01f, 1.0f);
        metronomeVolumeSlider_->setDoubleClickReturnValue(true, 1.f);
        metronomeVolumeSlider_->setValue(1.f);
        metronomeVolumeSlider_->onValueChange = [this]()
        {
            model_->setMetronomeVolume(metronomeVolumeSlider_->getValue());
        };
        componentToAdd->addAndMakeVisible(metronomeVolumeSlider_.get());
    }

    fileComponent_ = std::make_unique<RoundedComponent>(MelissaUISettings::getSubColour());
    addAndMakeVisible(fileComponent_.get());

    listComponent_ = std::make_unique<RoundedComponent>(MelissaUISettings::getSubColour());
    addAndMakeVisible(listComponent_.get());

    browseToggleButton_ = make_unique<ToggleButton>();
    browseToggleButton_->setButtonText("File browser");
    browseToggleButton_->setLookAndFeel(&tabLaf_);
    browseToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    browseToggleButton_->onClick = [&]()
    { updateFileChooserTab(kFileChooserTab_Browse); };
    browseToggleButton_->setToggleState(true, dontSendNotification);
    fileComponent_->addAndMakeVisible(browseToggleButton_.get());

    playlistToggleButton_ = make_unique<ToggleButton>();
    playlistToggleButton_->setButtonText("Playlist");
    playlistToggleButton_->setLookAndFeel(&tabLaf_);
    playlistToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    playlistToggleButton_->onClick = [&]()
    { updateFileChooserTab(kFileChooserTab_Playlist); };
    playlistToggleButton_->setToggleState(false, dontSendNotification);
    fileComponent_->addAndMakeVisible(playlistToggleButton_.get());

    historyToggleButton_ = make_unique<ToggleButton>();
    historyToggleButton_->setButtonText("History");
    historyToggleButton_->setLookAndFeel(&tabLaf_);
    historyToggleButton_->setRadioGroupId(kFileChooserTabGroup);
    historyToggleButton_->onClick = [&]()
    { updateFileChooserTab(kFileChooserTab_History); };
    historyToggleButton_->setToggleState(false, dontSendNotification);
    fileComponent_->addAndMakeVisible(historyToggleButton_.get());

    historyTable_ = make_unique<MelissaFileListBox>();
    historyTable_->setTarget(MelissaFileListBox::kTarget_History);
    historyTable_->setLookAndFeel(&laf_);
    fileComponent_->addAndMakeVisible(historyTable_.get());

    practiceTable_ = make_unique<MelissaPracticeTableListBox>();
    listComponent_->addAndMakeVisible(practiceTable_.get());

    markerTable_ = make_unique<MelissaMarkerListBox>();
    listComponent_->addAndMakeVisible(markerTable_.get());

    memoTextEditor_ = make_unique<TextEditor>();
    memoTextEditor_->setLookAndFeel(nullptr);
    memoTextEditor_->setFont(Font(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Main)));
    memoTextEditor_->setMultiLine(true, false);
    memoTextEditor_->setLookAndFeel(&memoLaf_);
    memoTextEditor_->onFocusLost = [&]()
    {
        dataSource_->saveMemo(memoTextEditor_->getText());
    };
    memoTextEditor_->setReturnKeyStartsNewLine(true);
    listComponent_->addAndMakeVisible(memoTextEditor_.get());

    browserComponent_ = make_unique<MelissaBrowserComponent>();
    listComponent_->addAndMakeVisible(browserComponent_.get());

    auto createAndAddTab = [&](const String &title, ListMemoTab tab)
    {
        auto b = make_unique<ToggleButton>();
        b->setButtonText(title);
        b->setLookAndFeel(&tabLaf_);
        b->setRadioGroupId(kListMemoTabGroup);
        b->onClick = [&, tab]()
        { updateListMemoTab(tab); };
        listComponent_->addAndMakeVisible(b.get());
        return b;
    };

    practiceListToggleButton_ = createAndAddTab("Practice list", kListMemoTab_Practice);
    markerListToggleButton_ = createAndAddTab("Marker", kListMemoTab_Marker);
    memoToggleButton_ = createAndAddTab("Memo", kListMemoTab_Memo);
    browserToggleButton_ = createAndAddTab("Browser", kListMemoTab_Browser);
    practiceListToggleButton_->setToggleState(true, dontSendNotification);

    addToPracticeButton_ = make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    addToPracticeButton_->setTooltip(TRANS("add_practice_list"));
    addToPracticeButton_->setImages(iconImages_[kIcon_Add].get(), iconHighlightedImages_[kIcon_Add].get());
    addToPracticeButton_->onClick = [this]()
    {
        practiceListToggleButton_->setToggleState(true, sendNotification);

        const String defaultName = MelissaUtility::getFormattedTimeSec(model_->getLoopAPosMSec() / 1000.f) + " - " + MelissaUtility::getFormattedTimeSec(model_->getLoopBPosMSec() / 1000.f);
        auto dialog = std::make_shared<MelissaInputDialog>(TRANS("enter_loop_name"), defaultName, [&](const String &text)
                                                           {
            String name(text);
            if (name.isEmpty()) name = defaultName;
            
            dataSource_->addPracticeList(name); }, std::vector<String>{
                                                                    "Guitar",
                                                                    "Bass",
                                                                    "Drums",
                                                                    "Piano",
                                                                    "Strings",
                                                                    "Synth",
                                                                    "Organ",
                                                                    "Brass",
                                                                    "Intro",
                                                                    "1st Verse",
                                                                    "2nd Verse",
                                                                    "Chorus",
                                                                    "Bridge",
                                                                    "Outro",
                                                                    "Solo",
                                                                    "Backing",
                                                                });
        MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(dialog), TRANS("add_practice_list"));
    };
    listComponent_->addAndMakeVisible(addToPracticeButton_.get());

    practiceListUpButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    practiceListUpButton_->setTooltip(TRANS("practice_list_up"));
    practiceListUpButton_->setImages(iconImages_[kIcon_Up].get(), iconHighlightedImages_[kIcon_Up].get());
    practiceListUpButton_->onClick = [&]()
    {
        practiceTable_->moveSelected(-1);
    };
    listComponent_->addAndMakeVisible(practiceListUpButton_.get());

    practiceListDownButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    practiceListDownButton_->setTooltip(TRANS("practice_list_down"));
    practiceListDownButton_->setImages(iconImages_[kIcon_Down].get(), iconHighlightedImages_[kIcon_Down].get());
    practiceListDownButton_->onClick = [&]()
    {
        practiceTable_->moveSelected(+1);
    };
    listComponent_->addAndMakeVisible(practiceListDownButton_.get());

    addMarkerButton_ = make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    addMarkerButton_->setTooltip(TRANS("add_marker"));
    addMarkerButton_->setImages(iconImages_[kIcon_Add].get(), iconImages_[kIcon_Add].get());
    addMarkerButton_->onClick = [this]()
    {
        markerListToggleButton_->setToggleState(true, sendNotification);
        dataSource_->addDefaultMarker(model_->getPlayingPosRatio());
    };
    listComponent_->addAndMakeVisible(addMarkerButton_.get());

    wildCardFilter_ = make_unique<WildcardFileFilter>(MelissaDataSource::getCompatibleFileExtensions(), "*", "Music Files");
    fileBrowserComponent_ = make_unique<FileBrowserComponent>(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
                                                              File::getSpecialLocation(File::userHomeDirectory),
                                                              wildCardFilter_.get(),
                                                              nullptr);
    fileBrowserComponent_->setLookAndFeel(&browserLaf_);
    fileBrowserComponent_->addListener(this);
    fileComponent_->addAndMakeVisible(fileBrowserComponent_.get());

    labelInfo_[kLabel_MetronomeBpm] = {"BPM", bpmButton_.get()};
    labelInfo_[kLabel_AccentPosition] = {"Accent position", accentPositionButton_.get()};
    labelInfo_[kLabel_MetronomeAccent] = {"Accent", accentButton_.get()};
    labelInfo_[kLabel_MusicVolume] = {"Music", musicVolumeSlider_.get()};
    labelInfo_[kLabel_MetronomeVolume] = {"Metronome", metronomeVolumeSlider_.get()};
    labelInfo_[kLabel_Pitch] = {"Pitch", pitchButton_.get()};
    labelInfo_[kLabel_OutputMode] = {"Output", outputModeComboBox_.get()};
    labelInfo_[kLabel_ATime] = {"Start", aButton_.get()};
    labelInfo_[kLabel_BTime] = {"End", bButton_.get()};
    labelInfo_[kLabel_Speed] = {"Speed", speedButton_.get()};
    labelInfo_[kLabel_SpeedPresets] = {"Presets", speedPresetViewport_.get()};
#if defined(ENABLE_SPEED_TRAINING)
    labelInfo_[kLabel_SpeedStart] = {"Start", speedIncStartButton_.get()};
    labelInfo_[kLabel_SpeedGoal] = {"Goal", speedIncGoalButton_.get()};
#endif

    for (size_t label_i = 0; label_i < kNumOfLabels; ++label_i)
    {
        auto l = make_unique<Label>();
        l->setLookAndFeel(nullptr);
        l->setText(labelInfo_[label_i].first, dontSendNotification);
        l->setFont(Font(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub)));
        l->setColour(Label::textColourId, MelissaUISettings::getTextColour());
        l->setInterceptsMouseClicks(false, true);
        l->setJustificationType(Justification::centred);
        labelInfo_[label_i].second->getParentComponent()->addAndMakeVisible(l.get());
        labels_[label_i] = std::move(l);
    }

    // Set List
    playlistComponent_ = make_unique<MelissaPlaylistComponent>();
    fileComponent_->addChildComponent(playlistComponent_.get());

    tooltipWindow_ = std::make_unique<TooltipWindow>(this, 0);
    tooltipWindow_->setLookAndFeel(&laf_);
    tooltipWindow_->setMillisecondsBeforeTipAppears(1000);

#ifdef JUCE_IOS
    importButton_ = std::make_unique<TextButton>();
    importButton_->setButtonText("IMP");
    importButton_->onClick = [&]()
    {
        showFileChooser();
    };
    safeAreaComponent_->addAndMakeVisible(importButton_.get());

    debugButton_ = std::make_unique<TextButton>();
    debugButton_->setButtonText("DBG");
    debugButton_->onClick = [&]()
    {
        MelissaDataSource::Song::Marker marker;

        marker.position_ = 0.5f;
        Colour colour = Colour::fromRGB(255, 160, 160);
        colour = colour.withHue(marker.position_);
        marker.colourR_ = colour.getRed();
        marker.colourG_ = colour.getGreen();
        marker.colourB_ = colour.getBlue();
        marker.memo_ = "Melissa Mobile Coming soon...";
        dataSource_->addMarker(marker);

        marker.position_ = 0.f;
        colour = colour.withHue(marker.position_);
        marker.colourR_ = colour.getRed();
        marker.colourG_ = colour.getGreen();
        marker.colourB_ = colour.getBlue();
        marker.memo_ = "Start";
        dataSource_->addMarker(marker);

        marker.position_ = 1.f;
        colour = colour.withHue(marker.position_);
        marker.colourR_ = colour.getRed();
        marker.colourG_ = colour.getGreen();
        marker.colourB_ = colour.getBlue();
        marker.memo_ = "End";
        dataSource_->addMarker(marker);
    };
    safeAreaComponent_->addAndMakeVisible(debugButton_.get());

    {
        iconTextButtons_[kIconText_Pitch] = std::make_unique<MelissaIconTextButton>(BinaryData::pitch_svg, BinaryData::pitch_svgSize, MelissaUISettings::getAccentColour().withHue(0.f), TRANS("pitch_short"));
        safeAreaComponent_->addAndMakeVisible(iconTextButtons_[kIconText_Pitch].get());

        iconTextButtons_[kIconText_Loop] = std::make_unique<MelissaIconTextButton>(BinaryData::loop_svg, BinaryData::loop_svgSize, MelissaUISettings::getAccentColour().withHue(0.2f), TRANS("loop_short"));
        safeAreaComponent_->addAndMakeVisible(iconTextButtons_[kIconText_Loop].get());

        iconTextButtons_[kIconText_Speed] = std::make_unique<MelissaIconTextButton>(BinaryData::speed_svg, BinaryData::pitch_svgSize, MelissaUISettings::getAccentColour().withHue(0.4f), TRANS("speed_short"));
        safeAreaComponent_->addAndMakeVisible(iconTextButtons_[kIconText_Speed].get());

        const String names[kNumControlPages] = {"Home", "Current", ">>", "List", "Marker", "Mixer", "EQ", "Metro", "Note"};
        for (int pageIndex = kControlPage_Home; pageIndex < kNumControlPages; ++pageIndex)
        {
            const int buttonIndex = kIconText_Control_Begin + pageIndex;
            iconTextButtons_[buttonIndex] = std::make_unique<MelissaIconTextButton>(BinaryData::temp_svg, BinaryData::temp_svgSize, MelissaUISettings::getAccentColour(), names[pageIndex]);
            iconTextButtons_[buttonIndex]->onClick = [&, this, buttonIndex, pageIndex, names]()
            {
                if (pageIndex == kControlPage_Expand_Shrink)
                {
                    showAllControlIcons_ = !showAllControlIcons_;
                    iconTextButtons_[kIconText_Control_Begin + kControlPage_Expand_Shrink]->setText(showAllControlIcons_ ? "<<" : ">>");
                    iconTextButtons_[kIconText_Control_Begin + kControlPage_Current]->setVisible(!showAllControlIcons_);

                    resized();
                }
                else if (pageIndex == kControlPage_Current)
                {
                    updateControlPage(static_cast<ControlPage>(currentControlPage_));
                }
                else
                {
                    if (kControlPage_List <= pageIndex && pageIndex <= kControlPage_Note)
                    {
                        iconTextButtons_[kIconText_Control_Begin + kControlPage_Current]->setText(names[pageIndex]);
                        currentControlPage_ = static_cast<ControlPage>(pageIndex);
                    }
                    updateControlPage(static_cast<ControlPage>(static_cast<ControlPage>(pageIndex)));
                }
            };
            safeAreaComponent_->addAndMakeVisible(iconTextButtons_[buttonIndex].get());

            if (pageIndex == kControlPage_Current || pageIndex == kControlPage_Expand_Shrink)
                continue;
            pages_[pageIndex] = std::make_unique<Label>();
            pages_[pageIndex]->setColour(Label::backgroundColourId, MelissaUISettings::getMainColour());
            // pages_[pageIndex]->setColour(Label::outlineColourId, Colours::pink);
            // pages_[pageIndex]->setText(names[pageIndex], dontSendNotification);
            safeAreaComponent_->addChildComponent(pages_[pageIndex].get());
        }
        for (auto &&page : pages_)
            if (page != nullptr)
                page->toFront(false);

        for (int buttonIndex = 0; buttonIndex < kNumControlButtons; ++buttonIndex)
        {
            controlButtons_[buttonIndex] = std::make_unique<MelissaControlButton>();
            pages_[kControlPage_Home]->addAndMakeVisible(controlButtons_[buttonIndex].get());
        }

        controlButtons_[0]->setText("<|");
        controlButtons_[0]->onClick = [&]()
        { MelissaCommand::getInstance()->excuteCommand("Back", 1.f); };

        controlButtons_[1]->setText("-5s");
        controlButtons_[1]->onClick = [&]()
        { MelissaCommand::getInstance()->excuteCommand("PlaybackPosition_Minus5Sec", 1.f); };

        controlButtons_[2]->setText("-1s");
        controlButtons_[2]->onClick = [&]()
        { MelissaCommand::getInstance()->excuteCommand("PlaybackPosition_Minus1Sec", 1.f); };

        controlButtons_[3]->setText("+1s");
        controlButtons_[3]->onClick = [&]()
        { MelissaCommand::getInstance()->excuteCommand("PlaybackPosition_Plus1Sec", 1.f); };

        controlButtons_[4]->setText("+5s");
        controlButtons_[4]->onClick = [&]()
        { MelissaCommand::getInstance()->excuteCommand("PlaybackPosition_Plus5Sec", 1.f); };

        controlButtons_[5]->setText("Reset\nLoop");
        controlButtons_[5]->onClick = [&]()
        { model_->setLoopPosRatio(0.f, 1.f); };

        controlButtons_[6]->setText("Set Marker");
        controlButtons_[6]->onClick = [&]()
        { dataSource_->addDefaultMarker(model_->getPlayingPosRatio()); };

        controlButtons_[7]->setText("Prev Marker");
        controlButtons_[7]->onClick = [&]()
        {
            float position = 0.f;
            std::vector<MelissaDataSource::Song::Marker> markers;
            dataSource_->getMarkers(markers);
            for (auto &&m : markers)
            {
                if (model_->getPlayingPosRatio() < m.position_)
                    break;
                position = m.position_;
            }
            model_->setPlayingPosRatio(position);
        };

        for (int speedButtonIndex = 0; speedButtonIndex < 8; ++speedButtonIndex)
        {
            const int speed = 65 + speedButtonIndex * 5;
            controlButtons_[8 + speedButtonIndex]->setText("Speed\n" + String(speed) + "%");
            controlButtons_[8 + speedButtonIndex]->onClick = [&, speed]()
            { model_->setSpeed(speed); };
        }
    }

#ifdef ENABLE_MOBILEAD
    adComponent_ = std::make_unique<MelissaAdComponent>();
    safeAreaComponent_->addAndMakeVisible(adComponent_.get());
#endif

    for (int separatorIndex = 0; separatorIndex < kNumSeparators; ++separatorIndex)
    {
        separators_[separatorIndex] = std::make_unique<MelissaSeparatorComponent>();
        safeAreaComponent_->addAndMakeVisible(separators_[separatorIndex].get());
    }
#endif

    updateSpeedModeTab(kSpeedModeTab_Basic);
    updateListMemoTab(kListMemoTab_Practice);
    updateFileChooserTab(kFileChooserTab_Browse);
    updateControlPage(kControlPage_Home);

    waveformComponent_->setMarkerListener(this);

    if (!isFullVersion)
        stemControlComponent_->setVisible(false);

    menuOverlay_ = std::make_unique<MenuOverlayComponent>();
    menuOverlay_->onMenuItemSelected = [this](int menuItemID)
    {
        this->menuItemSelected(menuItemID, 0);
    };
    addChildComponent(menuOverlay_.get());
}

void MainComponent::createMenu()
{
    menuButton_ = std::make_unique<MelissaMenuButton>();
    menuButton_->setBudgeVisibility(MelissaUpdateChecker::getUpdateStatus() == MelissaUpdateChecker::kUpdateStatus_UpdateExists);
    menuButton_->onClick = [&]()
    {
        menuOverlay_->showMenu(true);
    };

#ifdef JUCE_IOS
    menuComponentAnimator_ = std::make_unique<ComponentAnimator>();
#endif
}

void MainComponent::showFileChooser()
{
    fileChooser_ = std::make_unique<FileChooser>("Open", File::getCurrentWorkingDirectory(), MelissaDataSource::getCompatibleFileExtensions(), true);
    fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [&, this](const FileChooser &chooser)
                              {
        auto fileUrl = chooser.getURLResult();

        if (fileUrl.isLocalFile())
        {

#ifdef JUCE_IOS
            // Copy this file to sandbox
            dataSource_->loadFileAsync(MelissaMobileSupport::importFile(fileUrl).getFullPathName());
#else
            dataSource_->loadFileAsync(fileUrl.getLocalFile().getFullPathName());
#endif
        } });
}

void MainComponent::resized_Desktop()
{
    constexpr int kHeaderHeight = 50;
    headerComponent_->setBounds(0, 0, getWidth(), kHeaderHeight);

    {
        menuButton_->setBounds(0, 0, 200, 50);

        int x = 230;
        int centerY = kHeaderHeight / 2;
        playbackModeButton_->setSize(28, 22);
        x += (playbackModeButton_->getWidth() / 2);
        playbackModeButton_->setCentrePosition(x, centerY);
        x = playbackModeButton_->getRight() + 20;

        prevButton_->setSize(16, 17);
        x += (prevButton_->getWidth() / 2);
        prevButton_->setCentrePosition(x, centerY);
        x = prevButton_->getRight() + 10;

        playPauseButton_->setSize(52, 52);
        x += (playPauseButton_->getWidth() / 2);
        playPauseButton_->setCentrePosition(x, centerY);
        x = playPauseButton_->getRight() + 10;

        nextButton_->setSize(16, 17);
        x += (nextButton_->getWidth() / 2);
        nextButton_->setCentrePosition(x, centerY);
        x = nextButton_->getRight() + 10;

        const int labelWidth = getWidth() / 2 - (nextButton_->getRight() + 40);
        fileNameLabel_->setBounds(getWidth() / 2 - labelWidth / 2, 0, labelWidth, kHeaderHeight / 2);
        timeLabel_->setBounds(getWidth() / 2 - labelWidth / 2, kHeaderHeight / 2, labelWidth, kHeaderHeight / 2);

        constexpr int kMainVolumeWidth = 140;
        mainVolumeSlider_->setBounds(getWidth() - kMainVolumeWidth - 10, (kHeaderHeight - 30) / 2, kMainVolumeWidth, 30);

        constexpr int kAudioDeviceButtonWidth = 300;
        audioDeviceButton_->setBounds(mainVolumeSlider_->getX() - kAudioDeviceButtonWidth - 10, 0, kAudioDeviceButtonWidth, kHeaderHeight);

        exportButton_->setBounds(audioDeviceButton_->getX() - 50, (kHeaderHeight - 26) / 2, 26, 26);
        trimButton_->setBounds(exportButton_->getX() - 36, (kHeaderHeight - 26) / 2, 26, 26);
        constexpr int kExportBarWidth = 32;
        exportProgressBar_->setBounds(exportButton_->getX() + exportButton_->getWidth() / 2 - kExportBarWidth / 2, exportButton_->getBottom() + 2, kExportBarWidth, 4);
    }

    popupMessage_->setBounds(0, 10 + kHeaderHeight, getWidth(), 30);

    constexpr int kOffset = 20;
    waveformHolderComponent_->setSize(getWidth() - 30 * 2, 160 + 36);
    waveformComponent_->setBounds(0, 36, waveformHolderComponent_->getWidth(), 160);
    markerMemoComponent_->setBounds(kOffset, 0, waveformHolderComponent_->getWidth() - kOffset * 2, 30);

    waveformViewport_->setBounds(30, headerComponent_->getBottom() + 4, getWidth() - 30 * 2, 160 + 36 + kScrollbarThichness);

    controlComponent_->setBounds(0, waveformViewport_->getBottom() + 10, getWidth(), 230);

    int y = controlComponent_->getY() + 10;

    const int sectionMarginX = 10;
    const int sectionMarginY = 10;
    const int totalSectionWidth = getWidth() - sectionMarginX * 4;
#ifdef MELISSA_USE_STEM_SEPARATION
    const int songWidth = totalSectionWidth * 0.35;
    const int loopWidth = totalSectionWidth * 0.25;
    const int speedWidth = totalSectionWidth * 0.4;
#else
    const int songWidth = totalSectionWidth * 0.3;
    const int loopWidth = totalSectionWidth * 0.3;
    const int speedWidth = totalSectionWidth * 0.4;
#endif
    const int metronomeWidth = songWidth;
    const int eqWidth = loopWidth;
    const int mixerWidth = speedWidth;
    constexpr int controlHeight = 30;
    constexpr int controlAWidthMin = 120; // incDecButton etc..
    constexpr int controlBWidthMin = controlAWidthMin + 20;
    constexpr int controlBWidthMax = controlBWidthMin + 100;
    constexpr int pitchWidth = isFullVersion ? 120 : 160;
    constexpr int speedOutputWidth = 120;

    // Song
    {
        auto section = sectionComponents_[kSection_Song].get();
        section->setBounds(10, y, songWidth, 100);

        const int y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;

        pitchButton_->setBounds(isFullVersion ? 10 : ((section->getWidth() - pitchWidth) / 2), y, pitchWidth, controlHeight);

        const int stemControlWidth = songWidth - 10 * 3 - pitchButton_->getWidth();
        stemControlComponent_->setBounds(pitchButton_->getRight() + 10, y - controlHeight, stemControlWidth, controlHeight * 2);
        
        const int songDetailButtonWidth = MelissaUtility::getStringSize(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub), songDetailButton_->getButtonText()).first;
        songDetailButton_->setSize(songDetailButtonWidth, 30);
        songDetailButton_->setTopRightPosition(section->getWidth() - 10, 0);
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

        preCountOnOffButton_->setBounds(10, 5, 40, 20);

        {
            const int buttonWidth = MelissaUtility::getStringSize(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub), preCountSettingButton_->getButtonText()).first;
            preCountSettingButton_->setSize(buttonWidth, 30);
            preCountSettingButton_->setTopLeftPosition(preCountOnOffButton_->getRight() + 4, 0);
        }

        const int resetButtonWidth = MelissaUtility::getStringSize(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub), resetButton_->getButtonText()).first;
        resetButton_->setSize(resetButtonWidth, 30);
        resetButton_->setTopRightPosition(section->getWidth() - 10, 0);
    }

    // Speed
    {
        auto section = sectionComponents_[kSection_Speed].get();
        section->setBounds(sectionComponents_[kSection_Loop]->getRight() + sectionMarginX, y, speedWidth, 100);
        speedModeNormalComponent_->setBounds(section->getLocalBounds());
#if defined(ENABLE_SPEED_TRAINING)
        speedModeTrainingComponent_->setBounds(section->getLocalBounds());
#endif

        int x = 10;
        int y = 30 + (section->getHeight() - 30) / 2;
#if defined(ENABLE_SPEED_TRAINING)
        speedModeBasicToggleButton_->setBounds(x, y - 30, 100, controlHeight);
        speedModeTrainingToggleButton_->setBounds(x, y, 100, controlHeight);

        x = speedModeBasicToggleButton_->getRight() + 10;
#endif
        y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;
        speedButton_->setBounds(x, y, speedOutputWidth, controlHeight);

        const int viewportWidth = std::clamp((section->getWidth() - 10) - (speedButton_->getRight() + 10), controlAWidthMin, speedPresetComponent_->getWidth());
        x = (speedButton_->getRight() + 10) + ((section->getWidth() - 10) - (speedButton_->getRight() + 10)) / 2 - viewportWidth / 2;
        speedPresetViewport_->setBounds(x, y, viewportWidth, controlHeight);

#if defined(ENABLE_SPEED_TRAINING)
        x = speedModeBasicToggleButton_->getRight() + 10;
        const int buttonWidth = ((section->getWidth() - 20) - x - 20 * 3) / 4;
        speedIncStartButton_->setBounds(x, y, buttonWidth, controlHeight);
        speedIncValueButton_->setBounds(speedIncStartButton_->getRight() + 20, y, buttonWidth, controlHeight);
        speedIncPerButton_->setBounds(speedIncValueButton_->getRight() + 20, y, buttonWidth, controlHeight);
        speedIncGoalButton_->setBounds(speedIncPerButton_->getRight() + 20, y, buttonWidth, controlHeight);
        speedProgressComponent_->setBounds(speedIncValueButton_->getX(), y - 30 + 2, speedIncPerButton_->getRight() - speedIncValueButton_->getX(), 30 - 2);

        slashComponent_->setBounds(speedIncValueButton_->getRight(), y, speedIncPerButton_->getX() - speedIncValueButton_->getRight(), controlHeight);

        const int resetButtonWidth = MelissaUtility::getStringSize(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Sub), resetSpeedTrainingButton_->getButtonText()).first;
        resetSpeedTrainingButton_->setSize(resetButtonWidth, 30);
        resetSpeedTrainingButton_->setTopRightPosition(section->getWidth() - 10, 0);
#endif
    }

    y = sectionComponents_[kSection_Song]->getBottom() + sectionMarginY;

    // Metronome
    {
        auto section = sectionComponents_[kSection_Metronome].get();
        section->setBounds(10, y, metronomeWidth, 100);

        const int totalControlWidth = (section->getWidth() - 10 - 10 - 10 - 10);
        const int bpmAccentPosButtonWidth = totalControlWidth / (7 + 5 + 7) * 7;
        const int accentButtonWidth = totalControlWidth / (7 + 5 + 7) * 5;
        const int y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;

        metronomeOnOffButton_->setBounds(10, 5, 40, 20);

        bpmButton_->setBounds(10, y, bpmAccentPosButtonWidth, controlHeight);
        accentButton_->setBounds((section->getWidth() - accentButtonWidth) / 2, y, accentButtonWidth, controlHeight);
        accentPositionButton_->setBounds(section->getWidth() - bpmAccentPosButtonWidth - 10, y, bpmAccentPosButtonWidth, controlHeight);
    }

    // EQ
    {
        auto section = sectionComponents_[kSection_Eq].get();
        section->setBounds(sectionComponents_[kSection_Metronome]->getRight() + sectionMarginX, y, eqWidth, 100);

        eqSwitchButton_->setBounds(10, 5, 40, 20);

        constexpr int knobSize = 42;
        const int y = 30 + (section->getHeight() - 30) / 2 - knobSize / 2 - 8;
        const int interval = (section->getWidth() - knobSize * kNumOfEqBands * 3) / (kNumOfEqBands * 3 + 1);

        int x = interval;
        const int expandWidth = 40;
        for (size_t bandIndex = 0; bandIndex < kNumOfEqBands; ++bandIndex)
        {
            eqFreqKnobs_[bandIndex]->setBounds(x, y, knobSize, knobSize);
            knobLabels_[bandIndex * 3 + 0]->setBounds(x - expandWidth / 2, y + knobSize - 8, knobSize + expandWidth, 30);
            x += knobSize + interval;

            eqGainKnobs_[bandIndex]->setBounds(x, y, knobSize, knobSize);
            knobLabels_[bandIndex * 3 + 1]->setBounds(x - expandWidth / 2, y + knobSize - 8, knobSize + expandWidth, 30);
            x += knobSize + interval;

            eqQKnobs_[bandIndex]->setBounds(x, y, knobSize, knobSize);
            knobLabels_[bandIndex * 3 + 2]->setBounds(x - expandWidth / 2, y + knobSize - 8, knobSize + expandWidth, 30);
            x += knobSize + interval;
        }

        constexpr int qIconWidth = 24;
        constexpr int qIconHeight = 10;
        constexpr int qIconXMargin = 8;
        qIconComponents_[0]->setBounds(eqQKnobs_[0]->getX() - qIconWidth - qIconXMargin, eqQKnobs_[0]->getBottom() - qIconHeight, qIconWidth, qIconHeight);
        qIconComponents_[1]->setBounds(eqQKnobs_[0]->getRight() + qIconXMargin, eqQKnobs_[0]->getBottom() - qIconHeight, qIconWidth, qIconHeight);
    }

    // Mixer
    {
        auto section = sectionComponents_[kSection_Mixer].get();
        section->setBounds(sectionComponents_[kSection_Eq]->getRight() + sectionMarginX, y, mixerWidth, 100);

        const int controlWidth = (section->getWidth() - speedOutputWidth - 10 * 5) / 3;
        const int y = 30 + (section->getHeight() - 30) / 2 - controlHeight / 2 + 14;

        outputModeComboBox_->setBounds(10, y, speedOutputWidth, controlHeight);
        musicVolumeSlider_->setBounds(outputModeComboBox_->getRight() + 10, y, controlWidth, controlHeight);
        volumeBalanceSlider_->setBounds(musicVolumeSlider_->getRight() + 10, y, controlWidth, controlHeight);
        metronomeVolumeSlider_->setBounds(volumeBalanceSlider_->getRight() + 10, y, controlWidth, controlHeight);
    }

    // File component (Browser / Playlist / History)
    {
        int y = controlComponent_->getBottom() + kGradationHeight - 10;
        int h = (getHeight() - 10) - y;
        fileComponent_->setBounds(10, y, songWidth, h);
        int x = fileComponent_->getRight() + 10;
        listComponent_->setBounds(x, y, (getWidth() - 10) - x, h);
        const int kTabMargin = 2;
        int tabWidth = (fileComponent_->getWidth() - 20 - kTabMargin * (kNumOfFileChooserTabs - 1)) / kNumOfFileChooserTabs;

        x = 10;
        browseToggleButton_->setBounds(x, 10, tabWidth, 30);
        x += (tabWidth + kTabMargin);
        playlistToggleButton_->setBounds(x, 10, tabWidth, 30);
        x += (tabWidth + kTabMargin);
        historyToggleButton_->setBounds(x, 10, tabWidth, 30);

        tabWidth = 200;
        int w = fileComponent_->getWidth() - 20;
        h = fileComponent_->getHeight() - 60;
        fileBrowserComponent_->setBounds(10, 50, w, h);
        playlistComponent_->setBounds(10, 50, w, h);
        historyTable_->setBounds(10, 50, w, h);

        x = 10;
        y = 10;
        practiceListToggleButton_->setBounds(x, y, tabWidth, 30);
        addToPracticeButton_->setBounds(x + tabWidth - 30, y + 6, 18, 18);
        x += (tabWidth + 2);
        markerListToggleButton_->setBounds(x, y, tabWidth, 30);
        addMarkerButton_->setBounds(x + tabWidth - 30, y + 6, 18, 18);
        x += (tabWidth + 2);
        memoToggleButton_->setBounds(x, y, tabWidth, 30);
        x += (tabWidth + 2);
        browserToggleButton_->setBounds(x, y, tabWidth, 30);

        x = 10;
        y = 50;
        h = fileComponent_->getHeight() - 60;
        practiceTable_->setBounds(x, y, listComponent_->getWidth() - x - 20, h - 40);
        markerTable_->setBounds(x, y, listComponent_->getWidth() - x - 20, h);
        memoTextEditor_->setBounds(x, y, listComponent_->getWidth() - x - 20, h);
        browserComponent_->setBounds(x, y, listComponent_->getWidth() - x - 20, h);

        practiceListUpButton_->setBounds(x, practiceTable_->getBottom() + 10, 30, 30);
        practiceListDownButton_->setBounds(x + 40, practiceTable_->getBottom() + 10, 30, 30);
    }

    for (size_t label_i = 0; label_i < kNumOfLabels; ++label_i)
    {
        auto b = labelInfo_[label_i].second->getBoundsInParent();
        labels_[label_i]->setBounds(b.getX(), b.getY() - 30, b.getWidth(), 30);
        labels_[label_i]->toBack();
    }

    if (tutorialComponent_ != nullptr)
        tutorialComponent_->setBounds(0, 0, getWidth(), getHeight());
}

void MainComponent::resized_Mobile()
{
#ifdef JUCE_IOS
    auto safeBounds = [this]
    {
        auto bounds = getLocalBounds();
        if (auto *display = Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds()))
            return display->safeAreaInsets.subtractedFrom(display->keyboardInsets.subtractedFrom(bounds));
        return bounds;
    }();

    safeAreaComponent_->toBack();
    safeAreaComponent_->setBounds(safeBounds);
    const int width = safeAreaComponent_->getWidth();
    constexpr int kAdWidth = 320;
    constexpr int kAdHeight = 50;

    menuButton_->setBounds(10, 4, 30, 18);

    constexpr int kXOffset = 0;
    constexpr int kTextIconWidth = 40;
    constexpr int kTextIconHeight = 36;
    constexpr int xMargin = 10;

    // Waveform
    {
        /*
         constexpr int kOffset = 20;
         waveformHolderComponent_->setSize(getWidth() * 2 - 30 * 2, 160 + 36);
         waveformComponent_->setBounds(0, 36, waveformHolderComponent_->getWidth(), 160);
         markerMemoComponent_->setBounds(kOffset, 0, waveformHolderComponent_->getWidth() - kOffset * 2, 30);*/
        constexpr int kWaveformOffset = 0;
        waveformHolderComponent_->setSize(width, 90 + 18);
        waveformComponent_->setBounds(0, 18, width, 90);
        markerMemoComponent_->setBounds(0, 0, width, 16);
        waveformViewport_->setBounds(kXOffset, 50 - 18, width, 50 + 90);

        separators_[kSeparator_Upper]->setBounds(kXOffset, 140, width - kXOffset, 2);
        separators_[kSeparator_Lower]->setBounds(kXOffset, 192, width - kXOffset, 2);
    }

    // Center controls
    {
        constexpr int controlY = 147;
        int x = kXOffset;

        constexpr int minButtonAWidth = 90;
        constexpr int minButtonBWidth = 140;
        constexpr int playButtonWidth = 50;
        constexpr int playButtonHeight = 50;
        constexpr int loopLineLength = 10;

        const int totalButtonWidth = width - (playButtonWidth + kTextIconWidth + xMargin + kTextIconWidth + loopLineLength + xMargin + kTextIconWidth + xMargin);
        const int buttonAWidth = totalButtonWidth * minButtonAWidth / (minButtonAWidth * 2 + minButtonBWidth * 2);
        const int buttonBWidth = totalButtonWidth * minButtonBWidth / (minButtonAWidth * 2 + minButtonBWidth * 2);

        playPauseButton_->setBounds(kXOffset, 142, playButtonWidth, playButtonHeight);
        x += playPauseButton_->getWidth();

        iconTextButtons_[kIconText_Pitch]->setBounds(x, controlY + 4, kTextIconWidth, 36);
        x += iconTextButtons_[kIconText_Pitch]->getWidth();
        pitchButton_->setBounds(x, controlY, buttonAWidth, 40);
        x += pitchButton_->getWidth();

        x += xMargin;
        iconTextButtons_[kIconText_Loop]->setBounds(x, controlY + 4, kTextIconWidth, 36);
        x += iconTextButtons_[kIconText_Loop]->getWidth();
        aButton_->setBounds(x, controlY, buttonBWidth, 40);
        x += aButton_->getWidth();

        separators_[kSeparator_Loop]->setBounds(x, controlY + 19, loopLineLength, 2);
        x += loopLineLength;
        bButton_->setBounds(x, controlY, buttonBWidth, 40);
        x += bButton_->getWidth();

        x += xMargin;
        iconTextButtons_[kIconText_Speed]->setBounds(x, controlY + 4, kTextIconWidth, 36);
        x += iconTextButtons_[kIconText_Speed]->getWidth();
        speedButton_->setBounds(x, controlY, buttonAWidth, 40);
        x += speedButton_->getWidth();
    }

    {
        int x = 0;
        int y = 200;
        int kNumRowIcons = 3;
        int xMargin = 2;
        int yMargin = ((safeAreaComponent_->getHeight() - kAdHeight - y) - (kTextIconHeight * kNumRowIcons)) / (kNumRowIcons - 1);

        for (int pageIndex = 0; pageIndex < kNumControlPages; ++pageIndex)
        {
            const int row = pageIndex % kNumRowIcons;
            const int column = pageIndex / kNumRowIcons;
            iconTextButtons_[kIconText_Control_Begin + pageIndex]->setBounds(x + (kTextIconWidth + xMargin) * column, y + (kTextIconHeight + yMargin) * row, kTextIconWidth, kTextIconHeight);
        }

        const int controlPageX = showAllControlIcons_ ? iconTextButtons_[kIconText_Control_Begin + kNumControlPages - 1]->getRight() : iconTextButtons_[kIconText_Control_Begin + kControlPage_Home]->getRight();

        for (int pageIndex = 0; pageIndex < kNumControlPages; ++pageIndex)
        {
            if (pages_[pageIndex] != nullptr)
                pages_[pageIndex]->setBounds(controlPageX, y, safeAreaComponent_->getWidth() - controlPageX, safeAreaComponent_->getHeight() - kAdHeight - y);
        }

        for (int buttonIndex = 0; buttonIndex < kNumControlButtons; ++buttonIndex)
        {
            constexpr int numButtonInRow = 2;
            constexpr int numButtonInColumn = kNumControlButtons / numButtonInRow;
            const int row = buttonIndex / numButtonInColumn;
            const int column = buttonIndex % numButtonInColumn;

            const int buttonMargin = 2;
            const int buttonWidth = (pages_[kControlPage_Home]->getWidth() - buttonMargin * (numButtonInColumn - 1)) / numButtonInColumn;
            const int buttonHeight = (pages_[kControlPage_Home]->getHeight() - buttonMargin * (numButtonInRow - 1)) / numButtonInRow;

            controlButtons_[buttonIndex]->setBounds(column * (buttonWidth + buttonMargin), row * (buttonHeight + buttonMargin), buttonWidth, buttonHeight);
        }
    }

#ifdef ENABLE_MOBILEAD
    adComponent_->setBounds((safeAreaComponent_->getWidth() - kAdWidth) / 2, safeAreaComponent_->getBottom() - kAdHeight, kAdWidth, kAdHeight);
#endif

    if (menuComponent_ != nullptr)
    {
        menuComponent_->setBounds(-safeBounds.getWidth(), 0, safeBounds.getWidth(), safeBounds.getHeight());
        menuComponentAnimator_->animateComponent(menuComponent_.get(), safeAreaComponent_->getLocalBounds(), 1.f, 250.f, true, 1, 0);
    }

    importButton_->setBounds(100, 0, 50, 20);
    debugButton_->setBounds(160, 0, 50, 20);

    menuButton_->toFront(false);
#endif
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioEngine_->setOutputSampleRate(sampleRate);
    metronome_->setOutputSampleRate(sampleRate);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
{
    const auto numSamples = bufferToFill.numSamples;
    if (timeIndicesMSec_.size() != numSamples)
    {
        timeIndicesMSec_.resize(numSamples);
    }
    bufferToFill.clearActiveBufferRegion();

    const size_t numOfChannels = bufferToFill.buffer->getNumChannels();

    if (numOfChannels == 1)
    {
        float *buffer[] = {bufferToFill.buffer->getWritePointer(0)};
        if (model_->getPlaybackStatus() == kPlaybackStatus_Playing && !prepareingNextSong_)
        {
            audioEngine_->render(buffer, numOfChannels, timeIndicesMSec_, bufferToFill.numSamples);
        }
        metronome_->render(buffer, numOfChannels, timeIndicesMSec_, bufferToFill.numSamples);

        for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
        {
            buffer[0][sampleIndex] *= mainVolume_;
        }
    }
    else if (numOfChannels == 2)
    {
        float *buffer[] = {bufferToFill.buffer->getWritePointer(0), bufferToFill.buffer->getWritePointer(1)};
        if (model_->getPlaybackStatus() == kPlaybackStatus_Playing && !prepareingNextSong_)
        {
            audioEngine_->render(buffer, numOfChannels, timeIndicesMSec_, bufferToFill.numSamples);
        }
        metronome_->render(buffer, numOfChannels, timeIndicesMSec_, bufferToFill.numSamples);
        for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
        {
            buffer[0][sampleIndex] *= mainVolume_;
            buffer[1][sampleIndex] *= mainVolume_;
        }
    }
    else
    {
        jassertfalse;
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

void MainComponent::paint(Graphics &g)
{
    g.fillAll(MelissaUISettings::getMainColour());
}

void MainComponent::resized()
{
    if (isDesktop)
    {
        resized_Desktop();
    }
    else
    {
        resized_Mobile();
    }

    if (menuOverlay_ != nullptr)
    {
        menuOverlay_->setBounds(0, 0, getWidth(), getHeight());
    }

#ifdef JUCE_IOS
    importButton_->setBounds(10, headerComponent_->getBottom() + 10, 100, 30);
#endif

    MelissaModalDialog::resize();
}

void MainComponent::changeListenerCallback(ChangeBroadcaster *source)
{
    if (source == &deviceManager)
    {
        auto currentAudioDevice = deviceManager.getCurrentAudioDevice();
        if (currentAudioDevice != nullptr)
            audioDeviceButton_->setAudioDeviceName(currentAudioDevice->getName());
    }
}

bool MainComponent::keyPressed(const KeyPress &key, Component *originatingComponent)
{
#ifdef JUCE_MAC
    if (key.getTextDescription() == "command + Q")
        JUCEApplication::quit();
#endif
    return MelissaShortcutManager::getInstance()->processKeyboardMessage(key.getTextDescription());
}

void MainComponent::showAudioMidiSettingsDialog()
{
    auto component = std::make_shared<AudioDeviceSelectorComponent>(deviceManager, 0, 0, 0, 2, true, false, true, false);
    component->setSize(getWidth() * 0.6f, getHeight() * 0.8f);
    MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("audio_midi_settings"));
}

void MainComponent::showShortcutDialog()
{
    auto component = std::make_shared<MelissaShortcutComponent>();
    component->setSize(getWidth() * 0.8f, getHeight() * 0.8f);
    MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("shortcut_settings"));
}

void MainComponent::showAboutDialog()
{
    auto component = std::make_shared<MelissaAboutComponent>();
    MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("about_melissa"));
}

void MainComponent::showBPMSettingDialog()
{
    auto component = std::make_shared<MelissaBPMSettingComponent>();
    MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(component), TRANS("bpm_setting"));
}

void MainComponent::showTutorial()
{
#if defined(ENABLE_TUTORIAL)
    tutorialComponent_ = std::make_unique<MelissaTutorialComponent>(this);
    tutorialComponent_->setPages({
        {dynamic_cast<Component *>(menuButton_.get()), TRANS("explanation_menu")},
        {dynamic_cast<Component *>(waveformComponent_.get()), TRANS("explanation_waveform")},
        {dynamic_cast<Component *>(musicVolumeSlider_.get()), TRANS("explanation_volume")},
        {dynamic_cast<Component *>(pitchButton_.get()), TRANS("explanation_pitch")},
        {dynamic_cast<Component *>(speedButton_.get()), TRANS("explanation_speed")},
        {dynamic_cast<Component *>(fileBrowserComponent_.get()), TRANS("explanation_browser")},
    });
    addAndMakeVisible(tutorialComponent_.get());
    resized();
#endif
}

void MainComponent::dialogWillOpen()
{
    if (browserToggleButton_->getToggleState()) browserComponent_->setVisible(false);
}

void MainComponent::dialogWillClose()
{
    if (browserToggleButton_->getToggleState()) browserComponent_->setVisible(true);
}

void MainComponent::showUpdateDialog(bool showIfThereIsNoUpdate)
{
    const auto updateStatus = MelissaUpdateChecker::getUpdateStatus();

    if (updateStatus == MelissaUpdateChecker::kUpdateStatus_IsLatest)
    {
        if (showIfThereIsNoUpdate)
        {
            const std::vector<String> options = {TRANS("ok")};
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("there_is_no_update"), options, [&](size_t index) {});
            MelissaModalDialog::show(dialog, TRANS("update"));
        }
    }
    else if (updateStatus == MelissaUpdateChecker::kUpdateStatus_Failed)
    {
        if (showIfThereIsNoUpdate)
        {
            const std::vector<String> options = {TRANS("ok")};
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("failed_to_get_update"), options, [&](size_t index) {});
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

void MainComponent::songChanged(const String &filePath, size_t bufferLength, int32_t sampleRate)
{
    memoTextEditor_->setText(dataSource_->getMemo());
    auto parentDir = File(filePath).getParentDirectory();
    parentDir.setAsCurrentWorkingDirectory();
    fileBrowserComponent_->setRoot(parentDir);

    shouldInitializeBpmDetector_ = true;
}

void MainComponent::fileLoadStatusChanged(FileLoadStatus status, const String &filePath)
{
    if (status == kFileLoadStatus_Success)
    {
        fileNameLabel_->setText(File(filePath).getFileNameWithoutExtension(), dontSendNotification);
    }
    else if (status == kFileLoadStatus_Failed)
    {
        fileNameLabel_->setText(File(dataSource_->getCurrentSongFilePath()).getFileNameWithoutExtension(), dontSendNotification);
        const std::vector<String> options = {TRANS("ok")};
        MelissaModalDialog::show(std::make_shared<MelissaOptionDialog>(TRANS("load_failed") + "\n" + filePath, options, [&](size_t) {}), "Melissa", false);
    }
    else if (status == kFileLoadStatus_Loading)
    {
        fileNameLabel_->setText("Loading...", dontSendNotification);
    }
}

void MainComponent::fileDoubleClicked(const File &file)
{
    dataSource_->loadFileAsync(file);
}

void MainComponent::browserRootChanged(const File &newRoot)
{
    newRoot.setAsCurrentWorkingDirectory();
}

StringArray MainComponent::getMenuBarNames()
{
    return {"File"};
}

PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex, const String &menuName)
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
        showFileChooser();
    else if (menuItemID == kMenuID_Website)
        URL("https://mosynthkey.github.io/Melissa/").launchInDefaultBrowser();
    else if (menuItemID == kMenuID_About)
        showAboutDialog();
    else if (menuItemID == kMenuID_Manual)
        URL("https://github.com/mosynthkey/Melissa/wiki").launchInDefaultBrowser();
    else if (menuItemID == kMenuID_VersionCheck)
        showUpdateDialog(true);
    else if (menuItemID == kMenuID_Preferences)
        showAudioMidiSettingsDialog();
    else if (menuItemID == kMenuID_Shortcut)
        showShortcutDialog();
    else if (menuItemID == kMenuID_UITheme_Dark || menuItemID == kMenuID_UITheme_Light)
    {
        const std::vector<String> options = {TRANS("ok")};
        auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("restart_to_apply"), options, [&, menuItemID](size_t index)
                                                            {
            String uiTheme;
            if (menuItemID == kMenuID_UITheme_Dark)
            {
                uiTheme = "System_Dark";
            }
            else
            {
                uiTheme = "System_Light";
            }
            dataSource_->setUITheme(uiTheme);
            File::getSpecialLocation(File::currentExecutableFile).startAsProcess("--relaunch");
            JUCEApplicationBase::quit(); });
        MelissaModalDialog::show(dialog, TRANS("ui_theme"));
    }
    else if (menuItemID == kMenuID_Tutorial)
        showTutorial();
    else if (menuItemID == kMenuID_TwitterShare)
    {
        if (isLangJapanese_)
        {
            URL("https://twitter.com/intent/tweet?&text=Melissa+-+%E6%A5%BD%E5%99%A8%E7%B7%B4%E7%BF%92%2F%E8%80%B3%E3%82%B3%E3%83%94%E7%94%A8%E3%81%AE%E3%83%9F%E3%83%A5%E3%83%BC%E3%82%B8%E3%83%83%E3%82%AF%E3%83%97%E3%83%AC%E3%82%A4%E3%83%A4%E3%83%BC+%28macOS+%2F+Windows+%E5%AF%BE%E5%BF%9C%29&url=https%3A%2F%2Fmosynthkey.github.io%2FMelissa%2F&hashtags=MelissaMusicPlayer").launchInDefaultBrowser();
        }
        else
        {
            URL("https://twitter.com/intent/tweet?text=Melissa%20-%20A%20music%20player%20for%20musical%20instrument%20practice%0Afor%20macOS%20and%20Windows%20https%3A%2F%2Fgithub.com%2Fmosynthkey%2FMelissa&hashtags=MelissaMusicPlayer").launchInDefaultBrowser();
        }
    }
    else if (menuItemID == kMenuID_RevealSettingsFile)
        settingsFile_.revealToUser();
}

void MainComponent::handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message)
{
    MelissaShortcutManager::getInstance()->processMIDIMessage(message);
}

void MainComponent::run()
{
    while (!shouldExit_)
    {
        if (dataSource_->isFileLoaded())
        {
            if (audioEngine_->getStatus() == MelissaAudioEngine::kStatus_RequestingForNextSong)
            {
                audioEngine_->setStatus(MelissaAudioEngine::kStatus_WaitingNextSongToLoad);
                prepareingNextSong_ = true;
                MessageManager::callAsync([&]()
                                          {
                    loadNextSong();
                    prepareingNextSong_ = false; });
            }

            if (audioEngine_->isBufferSet() && audioEngine_->needToProcess())
            {
                audioEngine_->process();
            }
            else if (!bpmAnalyzeFinished_)
            {
                if (shouldInitializeBpmDetector_)
                {
                    bpmDetector_->initialize(dataSource_->getSampleRate(), dataSource_->getBufferLength());
                    shouldInitializeBpmDetector_ = false;
                }
                bpmDetector_->process(&bpmAnalyzeFinished_, &analyzedBpm_, &analyzedBeatPosMSec_);
                if (bpmAnalyzeFinished_)
                    shouldUpdateBpm_ = true;
            }
            else
            {
                wait(100);
            }
        }
        else
        {
            wait(1000);
        }
    }
}

void MainComponent::exitSignalSent()
{
    shouldExit_ = true;
    dataSource_->saveMemo(memoTextEditor_->getText());
    dataSource_->saveSongState();
    dataSource_->global_.rootDir_ = File::getCurrentWorkingDirectory().getFullPathName();
    dataSource_->global_.width_ = getWidth();
    dataSource_->global_.height_ = getHeight();
    const auto stateXml = deviceManager.createStateXml();
    if (stateXml != nullptr)
        dataSource_->global_.device_ = stateXml->toString();
    dataSource_->global_.playMode_ = static_cast<int>(model_->getPlaybackMode());

    const int selectedFileBrowserTab = (fileBrowserComponent_->isVisible()) ? 0 : (playlistComponent_->isVisible() ? 1 : 2);
    const int selectedPlaylist = playlistComponent_->getSelected();
    dataSource_->saveUIState({selectedFileBrowserTab, selectedPlaylist});

    dataSource_->saveSettingsFile();
    dataSource_->disposeBuffer();
}

void MainComponent::timerCallback()
{
    if (model_ == nullptr)
        return;

    if (!requestedKeyboardFocusOnFirstLaunch_)
    {
        // Adhoc
        grabKeyboardFocus();
        requestedKeyboardFocusOnFirstLaunch_ = true;
    }

    timeLabel_->setText(MelissaUtility::getFormattedTimeMSec(model_->getPlayingPosMSec()), dontSendNotification);
    waveformComponent_->setPlayPosition(model_->getPlayingPosRatio());

    const auto remainingTimeSec = (model_->getLengthMSec() - model_->getPlayingPosMSec()) / 1000.f;
    if (model_->getPlaybackMode() == kPlaybackMode_LoopPlaylistSongs && model_->getLoopAPosRatio() == 0.f && model_->getLoopBPosRatio() == 1.f && remainingTimeSec < 10)
    {
        if (!nextFileNameShown_)
        {
            const auto nextSongFilePath = getNextSongFilePath();
            if (nextSongFilePath.isNotEmpty())
            {
                const auto songName = File(nextSongFilePath).getFileNameWithoutExtension();
                fileNameLabel_->setText("Next ... \"" + songName + "\"", dontSendNotification);
            }
            nextFileNameShown_ = true;
        }
    }
    else if (nextFileNameShown_)
    {
        const auto songName = File(dataSource_->getCurrentSongFilePath()).getFileNameWithoutExtension();
        fileNameLabel_->setText(songName, dontSendNotification);
        nextFileNameShown_ = false;
    }

    if (shouldUpdateBpm_)
    {
        model_->setBpm((analyzedBpm_ == 0) ? kBpmMeasureFailed : analyzedBpm_);
        model_->setBeatPositionMSec(analyzedBeatPosMSec_);
        shouldUpdateBpm_ = false;
    }
}

void MainComponent::updatePlayBackModeButton()
{
    if (model_->getPlaybackMode() == kPlaybackMode_LoopOneSong)
    {
        playbackModeButton_->setImages(iconImages_[kIcon_LoopOneSong].get(), iconHighlightedImages_[kIcon_LoopOneSong].get());
        playbackModeButton_->setTooltip(TRANS("tooltip_playback_mode_one"));
        nextButton_->setEnabled(false);
    }
    else
    {
        playbackModeButton_->setImages(iconImages_[kIcon_LoopPlaylist].get(), iconHighlightedImages_[kIcon_LoopPlaylist].get());
        playbackModeButton_->setTooltip(TRANS("tooltip_playback_mode_playlist"));
        nextButton_->setEnabled(true);
    }
}

void MainComponent::updateSpeedModeTab(SpeedModeTab tab)
{
#if defined(ENABLE_SPEED_TRAINING)
    speedModeBasicToggleButton_->setToggleState(tab == kSpeedModeTab_Basic, dontSendNotification);
    speedModeTrainingToggleButton_->setToggleState(tab == kSpeedModeTab_Training, dontSendNotification);
    speedModeNormalComponent_->setVisible(tab == kSpeedModeTab_Basic);
    speedModeTrainingComponent_->setVisible(tab == kSpeedModeTab_Training);
#else
    speedModeNormalComponent_->setVisible(true);
#endif
}

void MainComponent::updateFileChooserTab(FileChooserTab tab)
{
    browseToggleButton_->setToggleState(tab == kFileChooserTab_Browse, dontSendNotification);
    playlistToggleButton_->setToggleState(tab == kFileChooserTab_Playlist, dontSendNotification);
    historyToggleButton_->setToggleState(tab == kFileChooserTab_History, dontSendNotification);

    fileBrowserComponent_->setVisible(tab == kFileChooserTab_Browse);
    playlistComponent_->setVisible(tab == kFileChooserTab_Playlist);
    historyTable_->setVisible(tab == kFileChooserTab_History);
}

void MainComponent::updateListMemoTab(ListMemoTab tab)
{
    practiceTable_->setVisible(tab == kListMemoTab_Practice);
    practiceListUpButton_->setVisible(tab == kListMemoTab_Practice);
    practiceListDownButton_->setVisible(tab == kListMemoTab_Practice);
    markerTable_->setVisible(tab == kListMemoTab_Marker);
    memoTextEditor_->setVisible(tab == kListMemoTab_Memo);
    browserComponent_->setVisible(tab == kListMemoTab_Browser);
}

void MainComponent::updateControlPage(ControlPage page)
{
#ifdef JUCE_IOS
    for (int pageIndex = 0; pageIndex < kNumControlPages; ++pageIndex)
    {
        if (pages_[pageIndex] != nullptr)
            pages_[pageIndex]->setVisible(page == pageIndex);
    }
#endif
}

void MainComponent::prev()
{
    if (model_ == nullptr)
        return;

    const auto mode = model_->getPlaybackMode();
    if (mode == kPlaybackMode_LoopOneSong)
    {
        model_->setPlayingPosRatio(model_->getLoopAPosRatio());
    }
    else if (mode == kPlaybackMode_LoopPlaylistSongs)
    {
        if (model_->getLoopAPosRatio() == 0.f && model_->getLoopBPosRatio() == 1.f && model_->getPlayingPosMSec() < 1000)
        {
            loadPrevSong();
        }
        else
        {
            model_->setPlayingPosRatio(model_->getLoopAPosRatio());
        }
    }
}

void MainComponent::next()
{
    if (model_ == nullptr)
        return;

    const auto mode = model_->getPlaybackMode();
    if (mode == kPlaybackMode_LoopPlaylistSongs)
    {
        loadNextSong();
    }
}

void MainComponent::resetLoop()
{
    model_->setLoopPosRatio(0.f, 1.f);
}

void MainComponent::loadPrevSong()
{
    const auto nextSongFilePath = getPrevSongFilePath();
    if (nextSongFilePath.isNotEmpty())
    {
        dataSource_->loadFileAsync(nextSongFilePath);
    }
    else
    {
        model_->setPlaybackStatus(kPlaybackStatus_Pause);
        model_->setPlayingPosRatio(0.f);
    }
}

void MainComponent::loadNextSong()
{
    const auto nextSongFilePath = getNextSongFilePath();
    if (nextSongFilePath.isNotEmpty())
    {
        dataSource_->loadFileAsync(nextSongFilePath);
    }
    else
    {
        model_->setPlaybackStatus(kPlaybackStatus_Pause);
        model_->setPlayingPosRatio(0.f);
    }
}

String MainComponent::getPrevSongFilePath()
{
    const auto currentFilePath = dataSource_->getCurrentSongFilePath();
    String nextFilePathToLoad = "";

    MelissaDataSource::FilePathList filePathList;

    dataSource_->getPlaylist(playlistComponent_->getSelected(), filePathList);

    bool found = false;
    int fileIndex = 0;
    for (; fileIndex < filePathList.size(); ++fileIndex)
    {
        if (filePathList[fileIndex] == currentFilePath)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        --fileIndex;
        if (0 <= fileIndex)
        {
            nextFilePathToLoad = filePathList[fileIndex];
        }
        else
        {
            nextFilePathToLoad = filePathList[filePathList.size() - 1];
        }
    }

    return File(nextFilePathToLoad).existsAsFile() ? nextFilePathToLoad : "";
}

String MainComponent::getNextSongFilePath()
{
    const auto currentFilePath = dataSource_->getCurrentSongFilePath();
    String nextFilePathToLoad = "";

    MelissaDataSource::FilePathList filePathList;

    dataSource_->getPlaylist(playlistComponent_->getSelected(), filePathList);

    bool found = false;
    int fileIndex = 0;
    for (; fileIndex < filePathList.size(); ++fileIndex)
    {
        if (filePathList[fileIndex] == currentFilePath)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        ++fileIndex;
        if (fileIndex < filePathList.size())
        {
            nextFilePathToLoad = filePathList[fileIndex];
        }
        else
        {
            nextFilePathToLoad = filePathList[0];
        }
    }

    return File(nextFilePathToLoad).existsAsFile() ? nextFilePathToLoad : "";
}

void MainComponent::musicVolumeChanged(float volume)
{
    musicVolumeSlider_->setValue(volume);
    const float db = 20 * log10(volume);
    String dbStr = String::formatted("%+1.1f dB", db);
    musicVolumeSlider_->setTooltip(TRANS("volume") + " : " + dbStr);
}

void MainComponent::pitchChanged(float semitone)
{
    pitchButton_->setText(MelissaUtility::getFormattedPitch(semitone));
}

void MainComponent::speedChanged(int speed)
{
    speedButton_->setText(String(speed) + "%");
}

void MainComponent::playPartChanged(PlayPart playPart)
{
}

void MainComponent::preCountSwitchChanged(bool preCountSwitch)
{
#if defined(ENABLE_PRECOUNT)
    preCountOnOffButton_->setToggleState(preCountSwitch, dontSendNotification);
#endif
}

void MainComponent::controlMessageReceived(const String &controlMessage)
{
    const auto assignedShortcut = MelissaDataSource::getInstance()->getAssignedShortcut(controlMessage);
    if (assignedShortcut.isEmpty())
        return;

    popupMessage_->show(controlMessage + String(" : ") + MelissaCommand::getInstance()->getCommandDescription(assignedShortcut));
}

void MainComponent::stemProviderStatusChanged(StemProviderStatus status)
{
}

void MainComponent::stemProviderResultReported(StemProviderResult result)
{
}

#if defined(ENABLE_SPEED_TRAINING)
void MainComponent::speedModeChanged(SpeedMode mode)
{
    updateSpeedModeTab((mode == kSpeedMode_Basic) ? kSpeedModeTab_Basic : kSpeedModeTab_Training);
    resetSpeedTrainingButton_->setVisible(mode == kSpeedMode_Training);
}

void MainComponent::speedIncStartChanged(int speedIncStart)
{
    speedIncStartButton_->setText(String(speedIncStart) + "%");
}

void MainComponent::speedIncValueChanged(int speedIncValue)
{
    speedIncValueButton_->setText(String("+ ") + String(speedIncValue));
}

void MainComponent::speedIncPerChanged(int speedIncPer)
{
    speedIncPerButton_->setText(String(speedIncPer) + " times");
}

void MainComponent::speedIncGoalChanged(int speedIncGoal)
{
    speedIncGoalButton_->setText(String(speedIncGoal) + "%");
}
#endif

void MainComponent::loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio)
{
    aButton_->setText(MelissaUtility::getFormattedTimeMSec(aTimeMSec));
    bButton_->setText(MelissaUtility::getFormattedTimeMSec(bTimeMSec));
}

void MainComponent::metronomeSwitchChanged(bool on)
{
    metronomeOnOffButton_->setToggleState(on, dontSendNotification);
}

void MainComponent::bpmChanged(float bpm)
{
    if (bpm == kBpmMeasureFailed)
    {
        bpmButton_->setText("---");
        bpmAnalyzeFinished_ = true;
    }
    else if (bpm == kBpmShouldMeasure)
    {
        bpmButton_->setText("...");
        bpmAnalyzeFinished_ = false;
    }
    else
    {
        bpmButton_->setText(String(bpm));
        bpmAnalyzeFinished_ = true;
    }
}

void MainComponent::beatPositionChanged(float beatPositionMSec)
{
    accentPositionButton_->setText(MelissaUtility::getFormattedTimeMSec(model_->getBeatPositionMSec()));
}

void MainComponent::accentChanged(int accent)
{
    accentButton_->setText((accent == 0) ? "---" : String(accent));
}

void MainComponent::metronomeVolumeChanged(float volume)
{
    metronomeVolumeSlider_->setValue(volume);
}

void MainComponent::musicMetronomeBalanceChanged(float balance)
{
    volumeBalanceSlider_->setValue(balance);
}

void MainComponent::outputModeChanged(OutputMode outputMode)
{
    outputModeComboBox_->setSelectedId(outputMode + 1);
}

void MainComponent::eqSwitchChanged(bool on)
{
    eqSwitchButton_->setToggleState(on, dontSendNotification);
}

void MainComponent::eqFreqChanged(size_t band, float freq)
{
    const float quantizedValue = std::log10(freq / 20.f) / 3.f; // log10(1000.f);
    eqFreqKnobs_[band]->setValue(quantizedValue, dontSendNotification);
    knobLabels_[0]->setText(String::formatted("%d Hz", static_cast<int>(freq)), dontSendNotification);
}

void MainComponent::eqGainChanged(size_t band, float gain)
{
    eqGainKnobs_[band]->setValue(gain, dontSendNotification);
    knobLabels_[1]->setText(String::formatted("%+2.1f dB", gain), dontSendNotification);
}

void MainComponent::eqQChanged(size_t band, float q)
{
    eqQKnobs_[band]->setValue(q, dontSendNotification);
    knobLabels_[2]->setText(String::formatted("Q:%1.2f", q), dontSendNotification);
}

void MainComponent::mainVolumeChanged(float mainVolume)
{
    mainVolume_ = mainVolume;
    mainVolumeSlider_->setValue(mainVolume, dontSendNotification);
}

void MainComponent::exportStarted()
{
    MessageManager::callAsync([&]()
                              { exportProgressBar_->setVisible(true); });
}

void MainComponent::exportCompleted(bool result, juce::String message)
{
    MessageManager::callAsync([&, message]()
                              {
        popupMessage_->show(message);
        exportProgressBar_->setVisible(false); });
}

void MainComponent::markerClicked(size_t markerIndex, bool isShiftKeyDown)
{
    std::vector<MelissaDataSource::Song::Marker> markers;
    dataSource_->getMarkers(markers);

    if (isShiftKeyDown)
    {
        const auto timeSec = model_->getLengthMSec() / 1000.f;
        const auto markerPosSec = markers[markerIndex].position_ * timeSec;
        const auto startPosRatio = std::clamp<float>((markerPosSec - 3) / timeSec, 0, 1.f);
        const auto endPosSec = std::clamp<float>((markerPosSec + 3) / timeSec, 0, 1.f);
        model_->setLoopPosRatio(startPosRatio, endPosSec);
    }
    else
    {
        const auto markerPosRatio = markers[markerIndex].position_;
        if (markerPosRatio < model_->getLoopAPosRatio() || model_->getLoopBPosRatio() < markerPosRatio)
        {
            model_->setLoopPosRatio(0.f, 1.f);
        }
        model_->setPlayingPosRatio(markers[markerIndex].position_);
        markerTable_->selectRow(static_cast<int>(markerIndex));
    }
}

bool MainComponent::isInterestedInFileDrag(const StringArray &files)
{
    return true;
}

void MainComponent::filesDropped(const StringArray &files, int x, int y)
{
    if (files.size() == 1)
    {
        dataSource_->loadFileAsync(files[0]);
    }
    else
    {
        MelissaModalDialog::show(std::make_shared<MelissaInputDialog>(TRANS("detect_multifiles_drop"), "new playlist", [&, files](const String &playlistName)
                                                                      {
            if (!playlistName.isEmpty())
            {
                const auto index = dataSource_->createPlaylist(playlistName);
                for (auto file : files) dataSource_->addToPlaylist(index, file);
                playlistComponent_->select(static_cast<int>(index));
            } }),
                                 TRANS("new_playlist"));
    }
}
