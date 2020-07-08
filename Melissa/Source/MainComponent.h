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
#include "MelissaSpeedTrainingProgressComponent.h"
#include "MelissaMessageComponent.h"
#include "MelissaMetronome.h"
#include "MelissaMIDIControlManager.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaPlaylistComponent.h"
#include "MelissaPracticeTableListBox.h"
#include "MelissaPreferencesComponent.h"
#include "MelissaMarkerListBox.h"
#include "MelissaSectionComponent.h"
#include "MelissaToHeadButton.h"
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
                        public FileDragAndDropTarget,
                        public FileBrowserListener,
                        public KeyListener,
                        public MelissaDataSourceListener,
                        public MelissaHost,
                        public MelissaModelListener,
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
    
    void updateSpeedModeTab(SpeedModeTab tab);
    void updateFileChooserTab(FileChooserTab tab);
    void updateListMemoTab(ListMemoTab tab);
    
    void toHead();
    void resetLoop();
    void addToPracticeList(String name);
    void saveMemo();
    
    var getSongSetting(String fileName);
    void showPreferencesDialog();
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
    
    std::unique_ptr<MelissaMenuButton> menuButton_;
    
    MelissaMessageComponent* messageComponent_;
    
    std::unique_ptr<PopupMenu> extraAppleMenuItems_;
    std::unique_ptr<MenuBarComponent> menuBar_;
    
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<Label> controlComponent_;
    std::unique_ptr<MelissaBottomControlComponent> bottomComponent_;
    
    std::unique_ptr<MelissaPlayPauseButton> playPauseButton_;
    std::unique_ptr<MelissaToHeadButton> toHeadButton_;
    
    std::unique_ptr<Label> timeLabel_;
    std::unique_ptr<MelissaScrollLabel> fileNameLabel_;

    std::unique_ptr<ToggleButton> metronomeOnOffButton_;
    std::unique_ptr<MelissaIncDecButton> bpmButton_;
    std::unique_ptr<MelissaIncDecButton> beatPositionButton_;
    std::unique_ptr<MelissaIncDecButton> accentButton_;
    
    std::unique_ptr<Slider> musicVolumeSlider_;
    std::unique_ptr<Slider> volumeBalanceSlider_;
    std::unique_ptr<Slider> metronomeVolumeSlider_;
    
    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    std::unique_ptr<DrawableButton> aResetButton_;
    std::unique_ptr<DrawableButton> bResetButton_;
    std::unique_ptr<TextButton> resetButton_;
    
    std::unique_ptr<Drawable> leftArrowImage_;
    std::unique_ptr<Drawable> rightArrowImage_;
    std::unique_ptr<Drawable> leftArrowHighlightedImage_;
    std::unique_ptr<Drawable> rightArrowHighlightedImage_;
    
    std::unique_ptr<ToggleButton> speedModeBasicToggleButton_;
    std::unique_ptr<ToggleButton> speedModeTrainingToggleButton_;
    
    std::unique_ptr<Component> speedModeNormalComponent_;
    std::unique_ptr<Component> speedModeTrainingComponent_;
    
    enum { kNumOfSpeedPresets = 11 };
    std::unique_ptr<MelissaIncDecButton> speedButton_;
    std::unique_ptr<Viewport> speedPresetViewport_;
    std::unique_ptr<Component> speedPresetComponent_;
    std::unique_ptr<TextButton> speedPresetButtons_[kNumOfSpeedPresets];

    class SlashComponent;
    std::unique_ptr<MelissaSpeedTrainingProgressComponent> speedProgressComponent_;
    std::unique_ptr<MelissaIncDecButton> speedIncStartButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncPerButton_;
    std::unique_ptr<SlashComponent>      slashComponent_;
    std::unique_ptr<MelissaIncDecButton> speedIncValueButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncGoalButton_;
    
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
    std::unique_ptr<MelissaAddButton> addToListButton_;
    std::unique_ptr<MelissaMarkerListBox> markerTable_;
    std::unique_ptr<MelissaPracticeTableListBox> practiceTable_;
    
    std::unique_ptr<ComboBox> outputModeComboBox_;
    
    std::unique_ptr<MelissaPreferencesComponent> preferencesComponent_;
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
        kSection_Output,
        kNumOfSections
    };
    std::array<std::unique_ptr<MelissaSectionComponent>, kNumOfSections> sectionComponents_;
    
    std::unique_ptr<MelissaTutorialComponent> tutorialComponent_;
    
    enum
    {
        kLabel_MetronomeBpm,
        kLabel_MetronomeOffset,
        kLabel_MetronomeAccent,
        kLabel_MusicVolume,
        kLabel_MetronomeVolume,
        kLabel_Pitch,
        kLabel_OutputMode,
        
        kLabel_ATime,
        kLabel_BTime,
        
        kLabel_Speed,
        kLabel_SpeedPresets,
        
        kLabel_SpeedStart,
        kLabel_SpeedGoal,
        
        kNumOfLabels
    };
    
    std::array<std::pair<String, Component*>, kNumOfLabels> labelInfo_;
    std::unique_ptr<Label> labels_[kNumOfLabels];
    
    MelissaLookAndFeel laf_;
    MelissaLookAndFeel_Tab tabLaf_;
    MelissaLookAndFeel_Memo memoLaf_;
    MelissaLookAndFeel_SlideToggleButton slideToggleLaf_;
    MelissaLookAndFeel_CircleToggleButton circleToggleLaf_;
    MelissaLookAndFeel_SelectorToggleButton selectorLaf_;
    MelissaLookAndFeel_CrossFader crossFaderLaf_;
    MelissaLookAndFeel_SimpleTextButton simpleTextButtonLaf_;
    std::vector<Component*> lafList_;
    
    String fileName_, fileFullPath_;
    
    File settingsDir_, settingsFile_;
    
    bool shouldExit_;
    
    MelissaMIDIControlManager midiControlManager_;
    
    std::unique_ptr<TooltipWindow> tooltipWindow_;
    
    std::vector<float> timeIndicesMSec_;
    
    void arrangeEvenly(const Rectangle<int> bounds, const std::vector<std::vector<Component*>>& components_, float widthRatio = 1.f);
    
    // MelissaModelListener
    void musicVolumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedModeChanged(SpeedMode mode) override;
    void speedChanged(int speed) override;
    void speedIncStartChanged(int speedIncStart) override;
    void speedIncValueChanged(int speedIncValue) override;
    void speedIncPerChanged(int speedIncPer) override;
    void speedIncGoalChanged(int speedIncGoal) override;
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
