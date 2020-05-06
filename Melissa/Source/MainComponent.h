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
#include "MelissaMIDIControlManager.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaPlaylistComponent.h"
#include "MelissaPracticeTableListBox.h"
#include "MelissaPreferencesComponent.h"
#include "MelissaScrollLabel.h"
#include "MelissaSectionComponent.h"
#include "MelissaToHeadButton.h"
#include "MelissaTutorialComponent.h"
#include "MelissaUpdateChecker.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"

enum FileChooserTab
{
    kFileChooserTab_Browse,
    kFileChooserTab_Playlist,
    kFileChooserTab_History,
    kNumOfFileChooserTabs
};

enum PracticeMemoTab
{
    kPracticeMemoTab_Practice,
    kPracticeMemoTab_Memo
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
    
    void updateFileChooserTab(FileChooserTab tab);
    void updatePracticeMemo(PracticeMemoTab tab);
    
    void toHead();
    void resetLoop();
    void addToPracticeList(String name);
    void saveMemo();
    
    var getSongSetting(String fileName);
    void showPreferencesDialog();
    void showAboutDialog();
    
    void showTutorial();
    void showUpdateDialog(bool showIfThereIsNoUpdate = false);

private:
    std::unique_ptr<MelissaAudioEngine> audioEngine_;
    MelissaModel* model_;
    MelissaDataSource* dataSource_;
    std::unique_ptr<MelissaBPMDetector> bpmDetector_;
    
    std::shared_ptr<AudioSampleBuffer> audioSampleBuf_;
    
    std::unique_ptr<MelissaMenuButton> menuButton_;
    
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
    std::unique_ptr<TextButton> analyzeButton_;
    
    std::unique_ptr<Slider> musicVolumeSlider_;
    std::unique_ptr<Slider> crossFaderSlider_;
    std::unique_ptr<Slider> metronomeVolumeSlider_;
    
    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    std::unique_ptr<TextButton> resetButton_;
    
    std::unique_ptr<ToggleButton> speedModeNormalToggleButton_;
    std::unique_ptr<ToggleButton> speedModeTrainingToggleButton_;
    
    std::unique_ptr<MelissaIncDecButton> speedButton_;

#if defined(ENABLE_SPEEDTRAINER)
    std::unique_ptr<MelissaIncDecButton> speedIncPerButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncValueButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncMaxButton_;
#endif
    
    std::unique_ptr<MelissaIncDecButton> pitchButton_;
    
    std::unique_ptr<ToggleButton> browseToggleButton_;
    std::unique_ptr<ToggleButton> playlistToggleButton_;
    std::unique_ptr<ToggleButton> historyToggleButton_;
    std::unique_ptr<WildcardFileFilter> wildCardFilter_;
    std::unique_ptr<FileBrowserComponent> fileBrowserComponent_;
    std::unique_ptr<MelissaFileListBox> historyTable_;
    
    std::unique_ptr<ToggleButton> practiceListToggleButton_;
    std::unique_ptr<ToggleButton> memoToggleButton_;
    std::unique_ptr<TextEditor> memoTextEditor_;
    std::unique_ptr<MelissaAddButton> addToListButton_;
    std::unique_ptr<MelissaPracticeTableListBox> practiceTable_;
    
    std::unique_ptr<ComboBox> oututModeComboBox_;
    
    std::unique_ptr<MelissaPreferencesComponent> preferencesComponent_;
    std::unique_ptr<MelissaPlaylistComponent> playlistComponent_;
    std::unique_ptr<MelissaModalDialog> modalDialog_;
    
    std::unique_ptr<FileChooser> fileChooser_;
    
    enum
    {
        kSection_Song,
        kSection_Loop,
        kSection_Output,
        kSection_Metronome,
        kSection_Speed,
        kNumOfSections
    };
    std::array<std::unique_ptr<MelissaSectionComponent>, kNumOfSections> sectionComponents_;
    
    std::unique_ptr<MelissaTutorialComponent> tutorialComponent_;
    
    enum
    {
        kLabel_MetronomeBpm,
        kLabel_MetronomeOffset,
        kLabel_MusicVolume,
        kLabel_MetronomeVolume,
        kLabel_Pitch,
        kLabel_OutputMode,
        
        kLabel_ATime,
        kLabel_BTime,
        
        kLabel_Speed,
#if defined(ENABLE_SPEEDTRAINER)
        kLabel_SpeedPlus,
        kLabel_SpeedPer,
        kLabel_SpeedMax,
#endif
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
    
    String fileName_, fileFullPath_;
    
    File settingsDir_, settingsFile_;
    
    bool shouldExit_;
    
    MelissaMIDIControlManager midiControlManager_;
    
    std::unique_ptr<TooltipWindow> tooltipWindow_;
    
    void arrangeEvenly(const Rectangle<int> bounds, const std::vector<std::vector<Component*>>& components_, float widthRatio = 1.f);
    
    // MelissaModelListener
    void volumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedChanged(int speed) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void bpmChanged(float bpm) override;
    void beatPositionChanged(float beatPositionMSec) override;
    void outputModeChanged(OutputMode outputMode) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
