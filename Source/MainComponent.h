//
//  MainComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <array>
#include <numeric>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaAudioEngine.h"
#include "MelissaBottomControlComponent.h"
#include "MelissaBPMDetector.h"
#include "MelissaButtons.h"
#include "MelissaDataSource.h"
#include "MelissaFileListBox.h"
#include "MelissaHost.h"
#include "MelissaIncDecButton.h"
#include "MelissaLookAndFeel.h"
#include "MelissaScrollLabel.h"
#include "MelissaShortcutManager.h"
#include "MelissaStemProvider.h"
#include "MelissaPopupMessageComponent.h"
#include "MelissaStemControlComponent.h"
#include "MelissaStemDetailComponent.h"

#if defined(ENABLE_SPEED_TRAINING)
#include "MelissaSpeedTrainingProgressComponent.h"
#endif

#include "MelissaMarkerListener.h"
#include "MelissaMarkerMemoComponent.h"
#include "MelissaMetronome.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaPlaylistComponent.h"
#include "MelissaPracticeTableListBox.h"
#include "MelissaMarkerListBox.h"
#include "MelissaSectionComponent.h"
#include "MelissaTutorialComponent.h"
#include "MelissaUpdateChecker.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"

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
    kListMemoTab_Memo
};

class MainComponent   : public AudioAppComponent,
                        public ChangeListener,
                        public FileDragAndDropTarget,
                        public FileBrowserListener,
                        public KeyListener,
                        public MelissaDataSourceListener,
                        public MelissaHost,
                        public MelissaMarkerListener,
                        public MelissaModelListener,
                        public MelissaShortcutListener,
                        public MelissaStemProviderListener,
                        public MenuBarModel,
                        public MidiInputCallback,
                        public Timer,
                        public Thread,
                        public Thread::Listener
{
public:
    MainComponent();
    ~MainComponent();
    
    void createUI();
    void showFileChooser();
    
    // AudioAppComponent
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void paint(Graphics& g) override;
    void resized() override;
    
    // ChangeListener
    void changeListenerCallback(ChangeBroadcaster* source) override;
    
    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const StringArray &files) override;
    void filesDropped(const StringArray& files, int x, int y) override;
    
    // FileBrowserListener
    void selectionChanged() override {};
    void fileClicked(const File& file, const MouseEvent& e) override {}
    void fileDoubleClicked(const File& file) override;
    void browserRootChanged(const File& newRoot) override;
    
    // KeyListener
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override;
    
    // Melissa
    void closeTutorial() override;
    
    // MelissaDataSourceListener
    void songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate) override;
    void fileLoadStatusChanged(FileLoadStatus status, const String& filePath) override;
    
    // MenuBarModel
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    
    // MidiInputCallback
    void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message) override;

    // Thread
    void run() override;
    
    // Thread
    void exitSignalSent() override;
    
    // Timer
    void timerCallback() override;
    
    void updatePlayBackModeButton();
    void updateStemToggleButton();
    void updateSpeedModeTab(SpeedModeTab tab);
    void updateFileChooserTab(FileChooserTab tab);
    void updateListMemoTab(ListMemoTab tab);
    
    void prev();
    void next();
    void resetLoop();
    void saveMemo();
    
    var getSongSetting(String fileName);
    void showAudioMidiSettingsDialog();
    void showShortcutDialog();
    void showAboutDialog();
    void showBPMSettingDialog();
    
    void showTutorial();
    void showUpdateDialog(bool showIfThereIsNoUpdate = false);

private:
    std::unique_ptr<MelissaAudioEngine> audioEngine_;
    std::unique_ptr<MelissaMetronome> metronome_;
    MelissaModel* model_;
    MelissaDataSource* dataSource_;
    std::unique_ptr<MelissaBPMDetector> bpmDetector_;
    float analyzedBpm_;
    bool bpmAnalyzeFinished_;
    bool shouldInitializeBpmDetector_;
    bool shouldUpdateBpm_;
    MelissaDataSource::Previous::UIState uiState_;
    
    std::shared_ptr<AudioSampleBuffer> audioSampleBuf_;
    float mainVolume_;
    
    class HeaderComponent;
    std::unique_ptr<HeaderComponent> headerComponent_;
    
    std::unique_ptr<MelissaPlayPauseButton> playPauseButton_;
    std::unique_ptr<DrawableButton> prevButton_;
    std::unique_ptr<DrawableButton> nextButton_;
    std::unique_ptr<DrawableButton> playbackModeButton_;
    std::unique_ptr<Label> timeLabel_;
    std::unique_ptr<Label> fileNameLabel_;
    
    std::unique_ptr<MelissaMenuButton> menuButton_;
    
    std::unique_ptr<MelissaAudioDeviceButton> audioDeviceButton_;
    std::unique_ptr<Slider> mainVolumeSlider_;
    
    std::unique_ptr<DrawableButton> stemControlToggleButton_;
    bool isStemDetailShown_;
    std::unique_ptr<MelissaStemControlComponent> stemControlComponent_;
    std::unique_ptr<MelissaStemDetailComponent> stemDetailComponent_;
    
    std::unique_ptr<MelissaPopupMessageComponent> popupMessage_;
    
    std::unique_ptr<PopupMenu> extraAppleMenuItems_;
    std::unique_ptr<MenuBarComponent> menuBar_;
    
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<MelissaMarkerMemoComponent> markerMemoComponent_;
    std::unique_ptr<Label> controlComponent_;
    class RoundedComponent;
    std::unique_ptr<RoundedComponent> fileComponent_;
    std::unique_ptr<RoundedComponent> listComponent_;
    


    std::unique_ptr<ToggleButton> metronomeOnOffButton_;
    std::unique_ptr<MelissaIncDecButton> bpmButton_;
    std::unique_ptr<MelissaIncDecButton> accentPositionButton_;
    std::unique_ptr<MelissaIncDecButton> accentButton_;
    
    std::unique_ptr<Slider> musicVolumeSlider_;
    std::unique_ptr<Slider> volumeBalanceSlider_;
    std::unique_ptr<Slider> metronomeVolumeSlider_;
    
    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    std::unique_ptr<DrawableButton> aResetButton_;
    std::unique_ptr<DrawableButton> bResetButton_;
    std::unique_ptr<TextButton> resetButton_;
    
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
        kNumOfIcons
    };
    
    enum ColorInfo
    {
        kColorInfo_None,
        kColorInfo_WhiteToAccent,
        kColorInfo_WhiteToMain,
    };
    
    std::unique_ptr<Drawable> iconImages_[kNumOfIcons];
    std::unique_ptr<Drawable> iconHighlightedImages_[kNumOfIcons];
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
    std::unique_ptr<Viewport> speedPresetViewport_;
    std::unique_ptr<Component> speedPresetComponent_;
    std::unique_ptr<TextButton> speedPresetButtons_[kNumOfSpeedPresets];

#if defined(ENABLE_SPEED_TRAINING)
    class SlashComponent;
    std::unique_ptr<MelissaSpeedTrainingProgressComponent> speedProgressComponent_;
    std::unique_ptr<MelissaIncDecButton> speedIncStartButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncPerButton_;
    std::unique_ptr<SlashComponent>      slashComponent_;
    std::unique_ptr<MelissaIncDecButton> speedIncValueButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncGoalButton_;
#endif
    
    std::unique_ptr<MelissaIncDecButton> pitchButton_;
    
    enum EqBand
    {
        EqBandMid1,
        kNumOfEqBands
    };
    std::unique_ptr<ToggleButton> eqSwitchButton_;
    std::unique_ptr<Slider> eqFreqKnobs_[kNumOfEqBands];
    std::unique_ptr<Slider> eqQKnobs_[kNumOfEqBands];
    std::unique_ptr<Slider> eqGainKnobs_[kNumOfEqBands];
    class QIconComponent;
    std::unique_ptr<QIconComponent> qIconComponents_[2];
    std::unique_ptr<Label> knobLabels_[kNumOfEqBands * 3];
    
    std::unique_ptr<ToggleButton> browseToggleButton_;
    std::unique_ptr<ToggleButton> playlistToggleButton_;
    std::unique_ptr<ToggleButton> historyToggleButton_;
    std::unique_ptr<WildcardFileFilter> wildCardFilter_;
    std::unique_ptr<FileBrowserComponent> fileBrowserComponent_;
    std::unique_ptr<MelissaFileListBox> historyTable_;
    
    std::unique_ptr<ToggleButton> practiceListToggleButton_;
    std::unique_ptr<ToggleButton> markerListToggleButton_;
    std::unique_ptr<ToggleButton> memoToggleButton_;
    std::unique_ptr<TextEditor> memoTextEditor_;
    std::unique_ptr<DrawableButton> addToPracticeButton_;
    std::unique_ptr<MelissaPracticeTableListBox> practiceTable_;
    std::unique_ptr<DrawableButton> practiceListUpButton_;
    std::unique_ptr<DrawableButton> practiceListDownButton_;
    std::unique_ptr<DrawableButton> addMarkerButton_;
    std::unique_ptr<MelissaMarkerListBox> markerTable_;
    
    std::unique_ptr<ComboBox> outputModeComboBox_;
    
    std::unique_ptr<MelissaPlaylistComponent> playlistComponent_;
    std::unique_ptr<MelissaModalDialog> modalDialog_;
    
    std::unique_ptr<FileChooser> fileChooser_;
    
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
        kLabel_Part,
        
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
    
    std::array<std::pair<String, Component*>, kNumOfLabels> labelInfo_;
    std::unique_ptr<Label> labels_[kNumOfLabels];
    
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
    std::vector<Component*> lafList_;
    
    String fileName_, fileFullPath_;
    File settingsDir_, settingsFile_;
    bool nextFileNameShown_;    
    bool shouldExit_;
    bool isLangJapanese_;
    bool requestedKeyboardFocusOnFirstLaunch_;
    
    std::unique_ptr<TooltipWindow> tooltipWindow_;
    
    std::vector<float> timeIndicesMSec_;
    
    bool prepareingNextSong_;
    void loadPrevSong();
    void loadNextSong();
    String getPrevSongFilePath();
    String getNextSongFilePath();
    
    // MelissaModelListener
    void musicVolumeChanged(float volume) override;
    void pitchChanged(float semitone) override;
    void speedChanged(int speed) override;
    void playPartChanged(PlayPart playPart) override;
    
    // MelissaShortcutListener
    void controlMessageReceived(const String& controlMessage) override;
    
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
    
    // MelissaMarkerListener
    void markerClicked(size_t markerIndex, bool isShiftKeyDown) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
