#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Melissa.h"
#include "MelissaBottomControlComponent.h"
#include "MelissaFileListBox.h"
#include "MelissaHost.h"
#include "MelissaIncDecButton.h"
#include "MelissaLookAndFeel.h"
#include "MelissaMIDIControlManager.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaPlayPauseButton.h"
#include "MelissaPreferencesComponent.h"
#include "MelissaScrollLabel.h"
#include "MelissaSetListComponent.h"
#include "MelissaToHeadButton.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"

enum FileChooserTab
{
    kFileChooserTab_Browse,
    kFileChooserTab_SetList,
    kFileChooserTab_Recent,
    kNumOfFileChooserTabs
};

enum PracticeMemoTab
{
    kPracticeMemoTab_Practice,
    kPracticeMemoTab_Memo
};

class MelissaPracticeTableListBox : public TableListBox,
                                    public TableListBoxModel
{
public:
    enum Column
    {
        kColumn_Name,
        kColumn_A,
        kColumn_B,
        kColumn_Speed,
        kNumOfColumn
    };
    
    MelissaPracticeTableListBox(MelissaHost* host, const String& componentName = String()) :
    TableListBox(componentName, this), host_(host)
    {
        String headerTitles[kNumOfColumn] = { "Name", "A", "B", "Speed" };
        for (int i = 0; i < kNumOfColumn; ++i)
        {
            getHeader().addColumn(headerTitles[i], i + 1, 50);
        }
        popupMenu_ = std::make_shared<PopupMenu>();
    }
    
    void resized() override
    {
        TableListBox::resized();
        autoSizeAllColumns();
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        const auto colour = Colour(MelissaColourScheme::MainColour()).withAlpha(rowIsSelected ? 0.06f : 0.f);
        g.fillAll(colour);
    }
    
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        String text = "";
        if (rowNumber < list_.size())
        {
            auto prac = list_[rowNumber];
            switch (columnId)
            {
                case kColumn_Name + 1:
                    text = prac.getProperty("name", "").toString();
                    break;
                case kColumn_A + 1:
                {
                    float ratio = prac.getProperty("a", 0);
                    text = MelissaUtility::getFormattedTimeMSec(ratio * totalLengthMSec_);
                    break;
                }
                case kColumn_B + 1:
                {
                    float ratio = prac.getProperty("b", 1);
                    text = MelissaUtility::getFormattedTimeMSec(ratio * totalLengthMSec_);
                    break;
                }
                case kColumn_Speed + 1:
                    text = String(static_cast<int32_t>(prac.getProperty("speed", 0))) + " %";
                    break;
            }
        }
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(22);
        g.drawText(text, 0, 0, width, height, Justification::left);
    }
    
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override
    {
        return nullptr;
    }
    
    int getColumnAutoSizeWidth(int columnId) override
    {
        constexpr int defaultWidth = 200;
        switch (columnId)
        {
            case kColumn_Name + 1:
                return getWidth() - defaultWidth * (kNumOfColumn - 1);
            default:
                return defaultWidth;
        }
    }
    
    void cellClicked(int rowNumber, int columnId, const MouseEvent& e) override
    {
        bool shouldRefresh = false;
        if (e.mods.isRightButtonDown())
        {
            enum
            {
                kMenuId_Erase = 1,
                kMenuId_Overwrite,
            };
            popupMenu_->clear();
            popupMenu_->setLookAndFeel(&lookAndFeel_);
            popupMenu_->addItem(kMenuId_Erase, TRANS("erase"), true);
            popupMenu_->addItem(kMenuId_Overwrite, TRANS("overwrite"), true);
            auto result = popupMenu_->show();
            if (result == kMenuId_Erase)
            {
                list_.remove(rowNumber);
                shouldRefresh = true;
            }
            else if (result == kMenuId_Overwrite)
            {
                float a, b;
                int speed;
                auto model = MelissaModel::getInstance();
                a = model->getLoopAPosRatio();
                b = model->getLoopBPosRatio();
                speed = model->getSpeed();
                list_[rowNumber].getDynamicObject()->setProperty("a", a);
                list_[rowNumber].getDynamicObject()->setProperty("b", b);
                list_[rowNumber].getDynamicObject()->setProperty("speed", speed);
                shouldRefresh = true;
            }
        }
        
        if (shouldRefresh)
        {
            host_->updatePracticeList(list_);
            updateContent();
        }
    }
    
    void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e) override
    {
        auto prac = list_[rowNumber];
        float a = prac.getProperty("a", 0);
        float b = prac.getProperty("b", 1);
        float speed = prac.getProperty("speed", 1.f);
        auto model = MelissaModel::getInstance();
        model->setLoopPosRatio(a, b);
        model->setSpeed(speed);
    }
    
    void setList(const Array<var>& list, float totalLengthMSec)
    {
        list_ = list;
        totalLengthMSec_ = totalLengthMSec;
        
        updateContent();
    }
    
private:
    Array<var> list_;
    float totalLengthMSec_;
    MelissaHost* host_;
    std::shared_ptr<PopupMenu> popupMenu_;
    MelissaLookAndFeel lookAndFeel_;
};

class MelissaTieComponent : public Component
{
public:
    MelissaTieComponent(Component* a, Component* b) : a_(a), b_(b) { }
    void paint(Graphics& g) override { g.fillAll(Colours::white.withAlpha(0.4f)); }
    void updatePosition()
    {
        constexpr int lightHeight = 2;
        int x = a_->getRight() - 1;
        int y = a_->getY() + a_->getHeight() / 2;
        setBounds(x, y - lightHeight / 2, b_->getX() - x, lightHeight);
    }
    
private:
    Component *a_, *b_;
};

class MelissaSectionTitleComponent : public Component
{
public:
    MelissaSectionTitleComponent(const String& title, float lineRatio) :
    lineRatio_(lineRatio)
    {
        label_ = std::make_unique<Label>();
        label_->setColour(Label::textColourId, Colours::white.withAlpha(0.6f));
        label_->setText(title, dontSendNotification);
        label_->setJustificationType(Justification::centred);
        label_->setFont(Font(22));
        addAndMakeVisible(label_.get());
        
        labelWidth_ = label_->getFont().getStringWidth(title);
    }
    
    void resized() override
    {
        label_->setBounds(getLocalBounds());
    }
    
    void paint(Graphics& g) override
    {
        labelWidth_ = label_->getFont().getStringWidth(label_->getText());
        constexpr int lineHeight = 1;
        g.setColour(Colours::white.withAlpha(0.4f));
        
        const int lineWidth = getWidth() * lineRatio_;
        g.fillRect(0, (getHeight() - lineHeight) / 2, lineWidth, lineHeight);
        g.fillRect(getWidth() - lineWidth, (getHeight() - lineHeight) / 2, lineWidth, lineHeight);
    }

private:
    std::unique_ptr<Label> label_;
    int labelWidth_;
    float lineRatio_;
};

class MelissaMenuButton : public Button
{
public:
    MelissaMenuButton() : Button("")
    {
        
    }
    
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        constexpr int lineHeight = 2;
        
        const bool highlighed = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
        g.setColour(Colour(MelissaColourScheme::MainColour()).withAlpha(highlighed ? 1.f : 0.6f));
        
        const int w = getWidth();
        const int h = getHeight();
        g.fillRoundedRectangle(0, 0, w, lineHeight, lineHeight / 2);
        g.fillRoundedRectangle(0, (h - lineHeight) / 2, w, lineHeight, lineHeight / 2);
        g.fillRoundedRectangle(0, h - lineHeight, w, lineHeight, lineHeight / 2);
    }
};

class MainComponent   : public AudioAppComponent,
                        public FileDragAndDropTarget,
                        public FileBrowserListener,
                        public KeyListener,
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
    void browserRootChanged(const File& newRoot) override {};
    
    // KeyListener
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override;
    
    // Melissa
    void updatePracticeList(const Array<var>& list) override;
    void createSetlist(const String& name) override;
    bool loadFile(const String& filePath) override;
    void showModalDialog(std::shared_ptr<Component> component, const String& title) override;
    void showPreferencesDialog() override;
    void closeModalDialog() override;
    
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
    
    enum Status
    {
        kStatus_Playing,
        kStatus_Pause,
        kStatus_Stop
    } status_;
    bool openFile(const File& file);
    void play();
    void pause();
    void stop();
    void toHead();
    void resetLoop();
    void addToPracticeList(String name);
    void addToRecent(String filePath);
    void saveMemo();
    
    void updateAll();
    void updateAButtonLabel();
    void updateBButtonLabel();
    void updateSpeedButtonLabel();
    void updatePitchButtonLabel();
    void updateBpm();
    void updateMetronomeOffset();
    void updateVolume();
    
    void createSettingsFile();
    void saveSettings();
    
    var getSongSetting(String fileName);
    void showAboutDialog();

private:
    std::unique_ptr<Melissa> melissa_;
    MelissaModel* model_;
    
    std::shared_ptr<AudioSampleBuffer> audioSampleBuf_;
    
    std::unique_ptr<MelissaMenuButton> menuButton_;
    
    std::unique_ptr<PopupMenu> extraAppleMenuItems_;
    std::unique_ptr<MenuBarComponent> menuBar_;
    
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<Label> controlComponent_;
#if defined(SHOW_BOTTOM)
    std::unique_ptr<MelissaBottomControlComponent> bottomComponent_;
#endif
    
    std::unique_ptr<MelissaPlayPauseButton> playPauseButton_;
    std::unique_ptr<MelissaToHeadButton> toHeadButton_;
    
    std::unique_ptr<Label> timeLabel_;
    std::unique_ptr<MelissaScrollLabel> fileNameLabel_;
    
#if defined(ENABLE_METRONOME)
    std::unique_ptr<ToggleButton> metronomeOnOffButton_;
    std::unique_ptr<MelissaIncDecButton> bpmButton_;
    std::unique_ptr<MelissaIncDecButton> metronomeOffsetButton_;
    std::unique_ptr<TextButton> analyzeButton_;
#endif
    
    std::unique_ptr<Slider> volumeSlider_;
    
    std::unique_ptr<TextButton> aSetButton_;
    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<TextButton> bSetButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    std::unique_ptr<TextButton> resetButton_;
    std::unique_ptr<MelissaTieComponent> tie_[2];
    
    std::unique_ptr<MelissaIncDecButton> speedButton_;

#if defined(ENABLE_SPEEDTRAINER)
    std::unique_ptr<MelissaIncDecButton> speedIncPerButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncValueButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncMaxButton_;
#endif
    
    std::unique_ptr<MelissaIncDecButton> pitchButton_;
    
    std::unique_ptr<ToggleButton> browseToggleButton_;
    std::unique_ptr<ToggleButton> setListToggleButton_;
    std::unique_ptr<ToggleButton> recentToggleButton_;
    std::unique_ptr<WildcardFileFilter> wildCardFilter_;
    std::unique_ptr<FileBrowserComponent> fileBrowserComponent_;
    std::unique_ptr<MelissaFileListBox> recentTable_;
    
    std::unique_ptr<ToggleButton> practiceListToggleButton_;
    std::unique_ptr<ToggleButton> memoToggleButton_;
    std::unique_ptr<TextEditor> memoTextEditor_;
    std::unique_ptr<TextButton> addToListButton_;
    std::unique_ptr<MelissaPracticeTableListBox> practiceTable_;
    
    std::unique_ptr<MelissaPreferencesComponent> preferencesComponent_;
    std::unique_ptr<MelissaSetListComponent> setListComponent_;
    std::unique_ptr<MelissaModalDialog> modalDialog_;
    
    std::unique_ptr<FileChooser> fileChooser_;
    
    enum
    {
#if defined(ENABLE_METRONOME)
        kLabel_MetronomeSw,
        kLabel_MetronomeBpm,
        kLabel_MetronomeOffset,
#endif
        kLabel_Volume,
        kLabel_Pitch,
        
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
    std::unique_ptr<Label> labels_[kNumOfLabels];
    
    enum
    {
        kSectionTitle_Settings,
        kSectionTitle_Loop,
#if defined(ENABLE_SPEEDTRAINER)
        kSectionTitle_Speed,
#endif
#if defined(ENABLE_METRONOME)
        kSectionTitle_Metronome,
#endif
        kNumOfSectionTitles
    };
    std::unique_ptr<MelissaSectionTitleComponent> sectionTitles_[kNumOfSectionTitles];
    
    MelissaLookAndFeel lookAndFeel_;
    MelissaLookAndFeel_Tab lookAndFeelTab_;
    MelissaLookAndFeel_Memo lookAndFeelMemo_;
    
    String fileName_, fileFullPath_;
    
    File settingsDir_, settingsFile_;
    var setting_;
    Array<var>* recent_;
    Array<var>* setList_;
    
    bool shouldExit_;
    
    MelissaMIDIControlManager midiControlManager_;
    
    void arrangeEvenly(const Rectangle<int> bounds, const std::vector<std::vector<Component*>>& components_, float widthRatio = 1.f);
    bool isSettingValid() const;
    
    // MelissaModelListener
    void volumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedChanged(int speed) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
