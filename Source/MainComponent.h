//
//  MainComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaAudioEngine.h"
#include "MelissaBPMDetector.h"
#include "MelissaBrowserComponent.h"
#include "MelissaButtons.h"
#include "MelissaDataSource.h"
#include "MelissaFileListBox.h"
#include "MelissaHost.h"
#include "MelissaIncDecButton.h"
#include "MelissaLookAndFeel.h"
#include "MelissaMarkerListBox.h"
#include "MelissaMarkerListener.h"
#include "MelissaMarkerMemoComponent.h"
#include "MelissaMetronome.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaPlaylistComponent.h"
#include "MelissaPopupMessageComponent.h"
#include "MelissaPracticeTableListBox.h"
#include "MelissaPreCountSettingComponent.h"
#include "MelissaProgressBarComponent.h"
#include "MelissaScrollLabel.h"
#include "MelissaSectionComponent.h"
#include "MelissaSeparatorComponent.h"
#include "MelissaShortcutManager.h"
#include "MelissaStemControlComponent.h"
#include "MelissaStemProvider.h"
#include "MelissaStemSeparationSelectComponent.h"
#include "MelissaTutorialComponent.h"
#include "MelissaUpdateChecker.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"
#include <array>
#include <numeric>

#if defined(ENABLE_SPEED_TRAINING)
#include "MelissaSpeedTrainingProgressComponent.h"
#endif

#ifdef JUCE_IOS
#include "MelissaMobileSupport.h"
#include "MelissaMobileMenuComponent.h"
#include "MelissaMobileFileListBox.h"
#endif

enum SpeedModeTab
{
    kSpeedModeTab_Basic,
    kSpeedModeTab_Training,
    kNumOfSpeedModeTabs
};

enum FileChooserTab
{
    kFileChooserTab_Browse,
    kFileChooserTab_Playlist,
    kFileChooserTab_History,
    kNumOfFileChooserTabs
};

enum ListMemoTab
{
    kListMemoTab_Practice,
    kListMemoTab_Marker,
    kListMemoTab_Memo,
    kListMemoTab_Browser,
    kNumOfListMemoTabs
};

enum ControlPage
{
    kControlPage_Home,
    kControlPage_Current,
    kControlPage_Expand_Shrink,
    kControlPage_List,
    kControlPage_Marker,
    kControlPage_Mixer,
    kControlPage_EQ,
    kControlPage_Metronome,
    kControlPage_Note,
    kNumControlPages
};

class MenuOverlayComponent : public juce::Component, public juce::Timer
{
public:
    MenuOverlayComponent();
    ~MenuOverlayComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent &e) override;
    void timerCallback() override;

    void showMenu(bool show);
    bool isMenuVisible() const { return isVisible() && menuVisible_; }

    std::function<void()> onMenuClosed;
    std::function<void(int)> onMenuItemSelected;

private:
    juce::Rectangle<int> menuArea_;
    bool menuVisible_;
    float menuPosX_;

    class MenuComponent;
    std::unique_ptr<MenuComponent> menuComponent_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuOverlayComponent)
};

class MainComponent : public juce::AudioAppComponent,
                      public juce::ChangeListener,
                      public juce::FileDragAndDropTarget,
                      public juce::FileBrowserListener,
                      public juce::KeyListener,
                      public MelissaDataSourceListener,
                      public MelissaHost,
                      public MelissaMarkerListener,
                      public MelissaModelListener,
                      public MelissaShortcutListener,
                      public MelissaStemProviderListener,
                      public juce::MenuBarModel,
                      public juce::MidiInputCallback,
                      public juce::Timer,
                      public juce::Thread,
                      public juce::Thread::Listener
{
public:
    MainComponent(const juce::String &commandLine = "");
    ~MainComponent();

    void createUI();
    void createMenu();
    void showFileChooser();
    void resized_Desktop();
    void resized_Mobile();

    // AudioAppComponent
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
    void releaseResources() override;
    void paint(juce::Graphics &g) override;
    void resized() override;

    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray &files) override;
    void filesDropped(const juce::StringArray &files, int x, int y) override;

    // FileBrowserListener
    void selectionChanged() override {};
    void fileClicked(const juce::File &file, const juce::MouseEvent &e) override {}
    void fileDoubleClicked(const juce::File &file) override;
    void browserRootChanged(const juce::File &newRoot) override;

    // KeyListener
    bool keyPressed(const juce::KeyPress &key, Component *originatingComponent) override;

    // Melissa
    void closeTutorial() override;

    // MelissaDataSourceListener
    void songChanged(const juce::String &filePath, size_t bufferLength, int32_t sampleRate) override;
    void fileLoadStatusChanged(FileLoadStatus status, const juce::String &filePath) override;

    // MenuBarModel
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String &menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    // MidiInputCallback
    void handleIncomingMidiMessage(juce::MidiInput *source, const juce::MidiMessage &message) override;

    // Thread
    void run() override;

    // Thread
    void exitSignalSent() override;

    // juce::Timer
    void timerCallback() override;

    void updatePlayBackModeButton();
    void updateSpeedModeTab(SpeedModeTab tab);
    void updateFileChooserTab(FileChooserTab tab);
    void updateListMemoTab(ListMemoTab tab);
    void updateControlPage(ControlPage page);

    void prev();
    void next();
    void resetLoop();
    void saveMemo();

    juce::var getSongSetting(juce::String fileName);
    void showAudioMidiSettingsDialog();
    void showShortcutDialog();
    void showAboutDialog();
    void showBPMSettingDialog();

    void showTutorial();
    void showUpdateDialog(bool showIfThereIsNoUpdate = false);

    void dialogWillOpen();
    void dialogWillClose();

private:
    std::unique_ptr<MelissaAudioEngine> audioEngine_;
    std::unique_ptr<MelissaMetronome> metronome_;
    MelissaModel *model_;
    MelissaDataSource *dataSource_;
    std::unique_ptr<MelissaBPMDetector> bpmDetector_;
    float analyzedBpm_;
    float analyzedBeatPosMSec_;
    bool bpmAnalyzeFinished_;
    bool shouldInitializeBpmDetector_;
    bool shouldUpdateBpm_;
    MelissaDataSource::Previous::UIState uiState_;

    std::shared_ptr<juce::AudioSampleBuffer> audioSampleBuf_;
    float mainVolume_;

    class HeaderComponent;
    std::unique_ptr<HeaderComponent> headerComponent_;

    std::unique_ptr<MelissaPlayPauseButton> playPauseButton_;
    std::unique_ptr<juce::DrawableButton> prevButton_;
    std::unique_ptr<juce::DrawableButton> nextButton_;
    std::unique_ptr<juce::DrawableButton> playbackModeButton_;
    std::unique_ptr<juce::Label> timeLabel_;
    std::unique_ptr<juce::Label> fileNameLabel_;

    std::unique_ptr<MelissaMenuButton> menuButton_;

    std::unique_ptr<juce::DrawableButton> trimButton_;
    std::unique_ptr<juce::DrawableButton> exportButton_;
    std::unique_ptr<MelissaProgressBarComponent> exportProgressBar_;

    std::unique_ptr<MelissaAudioDeviceButton> audioDeviceButton_;
    std::unique_ptr<juce::Slider> mainVolumeSlider_;

    std::unique_ptr<MelissaStemControlComponent> stemControlComponent_;
    std::unique_ptr<juce::TextButton> songDetailButton_;

    std::unique_ptr<MelissaPopupMessageComponent> popupMessage_;

    std::unique_ptr<juce::PopupMenu> extraAppleMenuItems_;
    std::unique_ptr<juce::MenuBarComponent> menuBar_;

    std::unique_ptr<juce::Viewport> waveformViewport_;
    std::unique_ptr<juce::Component> waveformHolderComponent_;
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<MelissaMarkerMemoComponent> markerMemoComponent_;
    std::unique_ptr<juce::Label> controlComponent_;
    class RoundedComponent;
    std::unique_ptr<RoundedComponent> fileComponent_;
    std::unique_ptr<RoundedComponent> listComponent_;

    std::unique_ptr<juce::ToggleButton> metronomeOnOffButton_;
    std::unique_ptr<MelissaIncDecButton> bpmButton_;
    std::unique_ptr<MelissaIncDecButton> accentPositionButton_;
    std::unique_ptr<MelissaIncDecButton> accentButton_;

    std::unique_ptr<juce::Slider> musicVolumeSlider_;
    std::unique_ptr<juce::Slider> volumeBalanceSlider_;
    std::unique_ptr<juce::Slider> metronomeVolumeSlider_;

    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    std::unique_ptr<juce::DrawableButton> aResetButton_;
    std::unique_ptr<juce::DrawableButton> bResetButton_;
    std::unique_ptr<juce::TextButton> resetButton_;
    std::unique_ptr<juce::ToggleButton> preCountOnOffButton_;
    std::unique_ptr<juce::TextButton> preCountSettingButton_;

    std::unique_ptr<MenuOverlayComponent> menuOverlay_;

    enum
    {
        kIcon_Prev,
        kIcon_Next,
        kIcon_LoopOneSong,
        kIcon_LoopPlaylist,
        kIcon_ArrowLeft,
        kIcon_ArrowRight,
        kIcon_Add,
        kIcon_Up,
        kIcon_Down,
        kIcon_Detail,
        kIcon_Select,
        kIcon_Export,
        kIcon_Trim,
        kNumOfIcons
    };

    enum ColorInfo
    {
        kColorInfo_None,
        kColorInfo_WhiteToAccent,
        kColorInfo_WhiteToMain,
    };

    std::unique_ptr<juce::Drawable> iconImages_[kNumOfIcons];
    std::unique_ptr<juce::Drawable> iconHighlightedImages_[kNumOfIcons];
    ColorInfo iconColorInfo_[kNumOfIcons];

#if defined(ENABLE_SPEED_TRAINING)
    std::unique_ptr<ToggleButton> speedModeBasicToggleButton_;
    std::unique_ptr<ToggleButton> speedModeTrainingToggleButton_;
#endif

    std::unique_ptr<Component> speedModeNormalComponent_;
#if defined(ENABLE_SPEED_TRAINING)
    std::unique_ptr<Component> speedModeTrainingComponent_;
#endif

    std::unique_ptr<MelissaIncDecButton> speedButton_;
    std::unique_ptr<juce::Viewport> speedPresetViewport_;
    std::unique_ptr<Component> speedPresetComponent_;
    std::unique_ptr<juce::TextButton> speedPresetButtons_[kNumOfSpeedPresets];

#if defined(ENABLE_SPEED_TRAINING)
    class SlashComponent;
    std::unique_ptr<MelissaSpeedTrainingProgressComponent> speedProgressComponent_;
    std::unique_ptr<MelissaIncDecButton> speedIncStartButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncPerButton_;
    std::unique_ptr<SlashComponent> slashComponent_;
    std::unique_ptr<MelissaIncDecButton> speedIncValueButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncGoalButton_;
    std::unique_ptr<juce::TextButton> resetSpeedTrainingButton_;
#endif

    std::unique_ptr<MelissaIncDecButton> pitchButton_;

    enum EqBand
    {
        EqBandMid1,
        kNumOfEqBands
    };
    std::unique_ptr<juce::ToggleButton> eqSwitchButton_;
    std::unique_ptr<juce::Slider> eqFreqKnobs_[kNumOfEqBands];
    std::unique_ptr<juce::Slider> eqQKnobs_[kNumOfEqBands];
    std::unique_ptr<juce::Slider> eqGainKnobs_[kNumOfEqBands];
    class QIconComponent;
    std::unique_ptr<QIconComponent> qIconComponents_[2];
    std::unique_ptr<juce::Label> knobLabels_[kNumOfEqBands * 3];

    std::unique_ptr<juce::ToggleButton> browseToggleButton_;
    std::unique_ptr<juce::ToggleButton> playlistToggleButton_;
    std::unique_ptr<juce::ToggleButton> historyToggleButton_;
    std::unique_ptr<juce::WildcardFileFilter> wildCardFilter_;
    std::unique_ptr<juce::FileBrowserComponent> fileBrowserComponent_;
    std::unique_ptr<MelissaFileListBox> historyTable_;

    std::unique_ptr<juce::ToggleButton> practiceListToggleButton_;
    std::unique_ptr<juce::ToggleButton> markerListToggleButton_;
    std::unique_ptr<juce::ToggleButton> memoToggleButton_;
    std::unique_ptr<juce::ToggleButton> browserToggleButton_;
    std::unique_ptr<juce::TextEditor> memoTextEditor_;
    std::unique_ptr<juce::DrawableButton> addToPracticeButton_;
    std::unique_ptr<MelissaPracticeTableListBox> practiceTable_;
    std::unique_ptr<juce::DrawableButton> practiceListUpButton_;
    std::unique_ptr<juce::DrawableButton> practiceListDownButton_;
    std::unique_ptr<juce::DrawableButton> addMarkerButton_;
    std::unique_ptr<MelissaMarkerListBox> markerTable_;

    std::unique_ptr<juce::ComboBox> outputModeComboBox_;

    std::unique_ptr<MelissaPlaylistComponent> playlistComponent_;
    std::unique_ptr<MelissaModalDialog> modalDialog_;

    std::unique_ptr<juce::FileChooser> fileChooser_;

    enum
    {
        kSection_Song,
        kSection_Loop,
        kSection_Speed,
        kSection_Metronome,
        kSection_Eq,
        kSection_Mixer,
        kNumOfSections
    };
    std::array<std::unique_ptr<MelissaSectionComponent>, kNumOfSections> sectionComponents_;

    std::unique_ptr<MelissaTutorialComponent> tutorialComponent_;

    enum
    {
        kLabel_MetronomeBpm,
        kLabel_AccentPosition,
        kLabel_MetronomeAccent,
        kLabel_MusicVolume,
        kLabel_MetronomeVolume,
        kLabel_Pitch,
        kLabel_OutputMode,

        kLabel_ATime,
        kLabel_BTime,

        kLabel_Speed,
        kLabel_SpeedPresets,

#if defined(ENABLE_SPEED_TRAINING)
        kLabel_SpeedStart,
        kLabel_SpeedGoal,
#endif

        kNumOfLabels
    };

    std::array<std::pair<juce::String, Component *>, kNumOfLabels> labelInfo_;
    std::unique_ptr<juce::Label> labels_[kNumOfLabels];

#ifdef JUCE_IOS
    std::unique_ptr<juce::TextButton> importButton_;
    std::unique_ptr<juce::TextButton> debugButton_;
#ifdef ENABLE_MOBILEAD
    std::unique_ptr<MelissaAdComponent> adComponent_;
#endif
    std::unique_ptr<MelissaMobileMenuComponent> menuComponent_;
    std::unique_ptr<juce::ComponentAnimator> menuComponentAnimator_;

    enum
    {
        kSeparator_Upper,
        kSeparator_Lower,
        kSeparator_Loop,
        kNumSeparators
    };
    std::unique_ptr<MelissaSeparatorComponent> separators_[kNumSeparators];

    enum
    {
        kIconText_Pitch,
        kIconText_Loop,
        kIconText_Speed,

        kIconText_Control_Begin,
        kIconText_Control_End = kIconText_Control_Begin + kNumControlPages - 1,

        kNumIconTexts
    };
    std::unique_ptr<MelissaIconTextButton> iconTextButtons_[kNumIconTexts];
    std::unique_ptr<juce::Label> pages_[kNumControlPages];
    bool showAllControlIcons_;
    ControlPage currentControlPage_;

    enum
    {
        kNumControlButtons = 16,
    };
    std::unique_ptr<MelissaControlButton> controlButtons_[kNumControlButtons];
#endif
    std::unique_ptr<juce::Label> safeAreaComponent_;

    MelissaLookAndFeel laf_;
    MelissaLookAndFeel_FileBrowser browserLaf_;
    MelissaLookAndFeel_Tab tabLaf_;
    MelissaLookAndFeel_Memo memoLaf_;
    MelissaLookAndFeel_SlideToggleButton slideToggleLaf_;
    MelissaLookAndFeel_CircleToggleButton circleToggleLaf_;
    MelissaLookAndFeel_SelectorToggleButton selectorLaf_;
    MelissaLookAndFeel_CrossFader crossFaderLaf_;
    MelissaLookAndFeel_SimpleTextButton simpleTextButtonLaf_;
    MelissaLookAndFeel_SimpleTextEditor simpleTextEditorLaf_;
    std::vector<Component *> lafList_;

    juce::String fileName_, fileFullPath_;
    juce::File settingsDir_, settingsFile_;
    bool nextFileNameShown_;
    bool shouldExit_;
    bool isLangJapanese_;
    bool requestedKeyboardFocusOnFirstLaunch_;

    std::unique_ptr<juce::TooltipWindow> tooltipWindow_;

    std::vector<float> timeIndicesMSec_;

    bool prepareingNextSong_;
    void loadPrevSong();
    void loadNextSong();
    juce::String getPrevSongFilePath();
    juce::String getNextSongFilePath();

    // MelissaModelListener
    void musicVolumeChanged(float volume) override;
    void pitchChanged(float semitone) override;
    void speedChanged(int speed) override;
    void playPartChanged(PlayPart playPart) override;
    void preCountSwitchChanged(bool preCountSwitch) override;

    // MelissaShortcutListener
    void controlMessageReceived(const juce::String &controlMessage) override;

    // MelissaStemProviderListener
    void stemProviderStatusChanged(StemProviderStatus status) override;
    void stemProviderResultReported(StemProviderResult result) override;

#if defined(ENABLE_SPEED_TRAINING)
    void speedModeChanged(SpeedMode mode) override;
    void speedIncStartChanged(int speedIncStart) override;
    void speedIncValueChanged(int speedIncValue) override;
    void speedIncPerChanged(int speedIncPer) override;
    void speedIncGoalChanged(int speedIncGoal) override;
#endif

    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void metronomeSwitchChanged(bool on) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void accentChanged(int accent) override;
    void metronomeVolumeChanged(float volume) override;
    void musicMetronomeBalanceChanged(float balance) override;
    void outputModeChanged(OutputMode outputMode) override;
    void eqSwitchChanged(bool on) override;
    void eqFreqChanged(size_t band, float freq) override;
    void eqGainChanged(size_t band, float gain) override;
    void eqQChanged(size_t band, float q) override;
    void mainVolumeChanged(float mainVolume) override;
    void exportStarted() override;
    void exportCompleted(bool result, juce::String message) override;

    // MelissaMarkerListener
    void markerClicked(size_t markerIndex, bool isShiftKeyDown) override;

    std::unique_ptr<MelissaPracticeTableListBox> practiceList_;
    std::unique_ptr<MelissaMarkerListBox> markerList_;
    std::unique_ptr<MelissaMarkerMemoComponent> markerMemo_;
    std::unique_ptr<MelissaBrowserComponent> browserComponent_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
