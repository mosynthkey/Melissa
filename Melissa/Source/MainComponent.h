#pragma once

#include <numeric>
#include "../JuceLibraryCode/JuceHeader.h"
#include "Melissa.h"
#include "MelissaBottomControlComponent.h"
#include "MelissaButtons.h"
#include "MelissaDataSource.h"
#include "MelissaFileListBox.h"
#include "MelissaHost.h"
#include "MelissaIncDecButton.h"
#include "MelissaLookAndFeel.h"
#include "MelissaMIDIControlManager.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaOkCancelDialog.h"
#include "MelissaPlaylistComponent.h"
#include "MelissaPreferencesComponent.h"
#include "MelissaScrollLabel.h"
#include "MelissaToHeadButton.h"
#include "MelissaTutorialComponent.h"
#include "MelissaUpdateChecker.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"


#define SHOW_BOTTOM

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

class MelissaPracticeTableListBox : public TableListBox,
                                    public TableListBoxModel
{
public:
    enum Column
    {
        kColumn_Name,
        kColumn_LoopRange,
        kColumn_Speed,
        kNumOfColumn
    };
    
    MelissaPracticeTableListBox(MelissaHost* host, const String& componentName = String()) :
    TableListBox(componentName, this), host_(host)
    {
        String headerTitles[kNumOfColumn] = { "Name", "Loop range", "Speed" };
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
        const auto colour = Colour(MelissaUISettings::MainColour()).withAlpha(rowIsSelected ? 0.06f : 0.f);
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
                {
                    text = prac.getProperty("name", "").toString();
                    break;
                }
                case kColumn_Speed + 1:
                {
                    text = String(static_cast<int32_t>(prac.getProperty("speed", 0))) + " %";
                    break;
                }
                default:
                {
                    return;
                }
            }
        }
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(MelissaUISettings::FontSizeMain());
        constexpr int xMargin = 10;
        g.drawText(text, xMargin, 0, width - xMargin * 2, height, Justification::left);
    }
    
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override
    {
        class LoopRangeComponent : public Component
        {
        public:
            LoopRangeComponent(float aRatio, float bRatio) : aRatio_(aRatio), bRatio_(bRatio)
            {
                setInterceptsMouseClicks(false, false);
            };
            void paint(Graphics& g) override
            {
                constexpr float lineWidth = 8.f;
                constexpr float xMargin = 10.f;
                const auto w = getWidth();
                const auto h = getHeight();
                const float aX = (w - lineWidth - xMargin * 2) * aRatio_ + xMargin;
                const float bX = (w - lineWidth - xMargin * 2) * bRatio_ + xMargin + lineWidth;
                
                g.setColour(Colour(MelissaUISettings::MainColour()).withAlpha(0.1f));
                g.fillRoundedRectangle(xMargin, (h - lineWidth) / 2.f, w - xMargin * 2, lineWidth, lineWidth / 2);
                g.setColour(Colour(MelissaUISettings::MainColour()).withAlpha(0.4f));
                g.fillRoundedRectangle(aX,      (h - lineWidth) / 2.f, bX - aX,         lineWidth, lineWidth / 2);
            }
            
        private:
            float aRatio_, bRatio_;
        };
        
        if (rowNumber < list_.size() && columnId == kColumn_LoopRange + 1)
        {
            auto prac = list_[rowNumber];
            if (existingComponentToUpdate == nullptr)
            {
                const float aRatio = prac.getProperty("a", 0);
                const float bRatio = prac.getProperty("b", 0);
                return dynamic_cast<Component*>(new LoopRangeComponent(aRatio, bRatio));
            }
            else
            {
                return existingComponentToUpdate;
            }
        }
        
        return nullptr;
    }
    
    int getColumnAutoSizeWidth(int columnId) override
    {
        const std::vector<int> widthRatio = { 3, 5, 2 };
        const float sum = std::accumulate(widthRatio.begin(), widthRatio.end(), 0);
        return widthRatio[columnId - 1] / sum * getWidth();
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
            repaint();
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
        label_->setFont(Font(MelissaUISettings::FontSizeMain()));
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
    void createPlaylist(const String& name) override;
    void closeTutorial() override;
    
    // MelissaDataSourceListener
    void songChanged(const String& filePath, const float* buffer[], size_t bufferLength, int32_t sampleRate) override;
    
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
    void play();
    void pause();
    void stop();
    void toHead();
    void resetLoop();
    void addToPracticeList(String name);
    void addToHistory(String filePath);
    void saveMemo();
    
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
    void showPreferencesDialog();
    void showAboutDialog();
    
    void showTutorial();
    void showUpdateDialog(bool showIfThereIsNoUpdate = false);

private:
    std::unique_ptr<Melissa> melissa_;
    MelissaModel* model_;
    MelissaDataSource* dataSource_;
    
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
    
    std::unique_ptr<MelissaPreferencesComponent> preferencesComponent_;
    std::unique_ptr<MelissaPlaylistComponent> playlistComponent_;
    std::unique_ptr<MelissaModalDialog> modalDialog_;
    
    std::unique_ptr<FileChooser> fileChooser_;
    
    std::unique_ptr<MelissaTutorialComponent> tutorialComponent_;
    
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
    
    bool shouldExit_;
    
    MelissaMIDIControlManager midiControlManager_;
    
    std::unique_ptr<TooltipWindow> tooltipWindow_;
    
    void arrangeEvenly(const Rectangle<int> bounds, const std::vector<std::vector<Component*>>& components_, float widthRatio = 1.f);
    
    // MelissaModelListener
    void volumeChanged(float volume) override;
    void pitchChanged(int semitone) override;
    void speedChanged(int speed) override;
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override;
    void playingPosChanged(float time, float ratio) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
