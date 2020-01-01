#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Melissa.h"
#include "MelissaControlComponent.h"
#include "MelissaIncDecButton.h"
#include "MelissaLookAndFeel.h"
#include "MelissaPlayPauseButton.h"
#include "MelissaPreferencesComponent.h"
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

class MelissaHost
{
public:
    virtual ~MelissaHost() {};
    virtual void setMelissaParameters(float aRatio, float bRatio, float speed, int32_t pitch) = 0;
    virtual void getMelissaParameters(float* aRatio, float* bRatio, float* speed, int32_t* pitch, int32_t* count) = 0;
    virtual void updatePracticeList(const Array<var>& list) = 0;    
    virtual void loadFile(const String& filePath) = 0;
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
        kColumn_Pitch,
        kColumn_Count,
        kNumOfColumn
    };
    
    MelissaPracticeTableListBox(MelissaHost* host, const String& componentName = String()) :
    TableListBox(componentName, this), host_(host)
    {
        std::string headerTitles[kNumOfColumn] = { "Name", "A", "B", "Speed", "Pitch", "Count" };
        for (int i = 0; i < kNumOfColumn; ++i)
        {
            getHeader().addColumn(headerTitles[i], i + 1, 50);
        }
        
        autoSizeAllColumns();
        
        popupMenu_ = std::make_shared<PopupMenu>();
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        g.fillAll(Colour::fromFloatRGBA(1.f, 1.f, 1.f, rowIsSelected ? 0.2f : 0.f));
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
                case kColumn_Pitch + 1:
                    text = MelissaUtility::getFormattedPitch(prac.getProperty("pitch", 0));
                    break;
                case kColumn_Count + 1:
                    text = prac.getProperty("count", "").toString();
                    break;
            }
        }
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.drawText(text, 0, 0, width, height, Justification::left);
    }
    
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override
    {
        return nullptr;
    }
    
    int getColumnAutoSizeWidth(int columnId) override
    {
        return 140;
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
            popupMenu_->addItem(kMenuId_Erase, "Erase", true);
            popupMenu_->addItem(kMenuId_Overwrite, "Overwrite", true);
            auto result = popupMenu_->show();
            if (result == kMenuId_Erase)
            {
                list_.remove(rowNumber);
                shouldRefresh = true;
            }
            else if (result == kMenuId_Overwrite)
            {
                float a, b, speed;
                int32_t pitch, count;
                host_->getMelissaParameters(&a, &b, &speed, &pitch, &count);
                list_[rowNumber].getDynamicObject()->setProperty("a", a);
                list_[rowNumber].getDynamicObject()->setProperty("b", b);
                list_[rowNumber].getDynamicObject()->setProperty("speed", speed);
                list_[rowNumber].getDynamicObject()->setProperty("pitch", pitch);
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
        float pitch = prac.getProperty("pitch", 1.f);
        
        host_->setMelissaParameters(a, b, speed, pitch);
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
};

class MelissaRecentListBox : public ListBox,
                             public ListBoxModel
{
public:
    MelissaRecentListBox(MelissaHost* host, const String& componentName = String()) :
    ListBox(componentName, this), host_(host)
    {
    }
    
    void paint(Graphics& g) override
    {
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRect(0, 0, getWidth(), getHeight());
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void listBoxItemDoubleClicked(int row, const MouseEvent& e) override
    {
        host_->loadFile(list_[row].toString());
    }
    
    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
    {
        const String fullPath = (rowNumber < list_.size()) ?  list_[rowNumber].toString() : "";
        const String fileName = File(fullPath).getFileName();
        
        g.fillAll(Colour::fromFloatRGBA(1.f, 1.f, 1.f, rowIsSelected ? 0.2f : 0.f));
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.drawText(fileName, 10, 0, width - 20, height, Justification::left);
    }
    
    void setList(const Array<var>& list)
    {
        list_ = list;
        updateContent();
    }
    
private:
    Array<var> list_;
    MelissaHost* host_;
};

class MelissaSectionTitleComponent : public Component
{
public:
    MelissaSectionTitleComponent(const std::string& title, int lineWidth) :
    lineWidth_(lineWidth)
    {
        label_ = std::make_unique<Label>();
        label_->setColour(Label::textColourId, Colours::white.withAlpha(0.6f));
        label_->setText(title, dontSendNotification);
        label_->setJustificationType(Justification::centred);
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
        g.fillRect(0, (getHeight() - lineHeight) / 2, lineWidth_, lineHeight);
        g.fillRect(getWidth() - lineWidth_, (getHeight() - lineHeight) / 2, lineWidth_, lineHeight);
    }

private:
    std::unique_ptr<Label> label_;
    int labelWidth_;
    int lineWidth_;
};

class MainComponent   : public AudioAppComponent,
                        public FileBrowserListener,
                        public KeyListener,
                        public MelissaHost,
                        public MelissaWaveformControlListener,
                        public MenuBarModel,
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
    
    // FileBrowserListener
    void selectionChanged() override {};
    void fileClicked(const File& file, const MouseEvent& e) override {}
    void fileDoubleClicked(const File& file) override;
    void browserRootChanged(const File& newRoot) override {};
    
    // KeyListener
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override;
    
    // Melissa
    void setMelissaParameters(float aRatio, float bRatio, float speed, int32_t pitch) override;
    void getMelissaParameters(float* aRatio, float* bRatio, float* speed, int32_t* pitch, int32_t* count) override;
    void updatePracticeList(const Array<var>& list) override;
    void loadFile(const String& filePath) override;
    
    // MelissaWaveformControlListener
    void setPlayPosition(MelissaWaveformControlComponent* sender, float ratio) override;
    void setAPosition(MelissaWaveformControlComponent* sender, float ratio) override;
    void setBPosition(MelissaWaveformControlComponent* sender, float ratio) override;
    
    // MenuBarModel
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;

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
    
    void createSettingsFile();
    void saveSettings();
    
    var getSongSetting(String fileName);

private:
    std::unique_ptr<Melissa> melissa_;
    std::unique_ptr<AudioSampleBuffer> audioSampleBuf_;
    
    std::unique_ptr<PopupMenu> extraAppleMenuItems_;
    std::unique_ptr<MenuBarComponent> menuBar_;
    
    std::unique_ptr<MelissaWaveformControlComponent> waveformComponent_;
    std::unique_ptr<MelissaControlComponent> controlComponent_;
    
    std::unique_ptr<MelissaPlayPauseButton> playPauseButton_;
    std::unique_ptr<MelissaToHeadButton> toHeadButton_;
    
    std::unique_ptr<Label> timeLabel_;
    std::unique_ptr<Label> fileNameLabel_;
    
    std::unique_ptr<ToggleButton> metronomeOnOffButton_;
    std::unique_ptr<MelissaIncDecButton> bpmButton_;
    std::unique_ptr<MelissaIncDecButton> metronomeOffsetButton_;
    std::unique_ptr<TextButton> analyzeButton_;
    
    std::unique_ptr<Slider> volumeSlider_;
    
    std::unique_ptr<TextButton> aSetButton_;
    std::unique_ptr<MelissaIncDecButton> aButton_;
    std::unique_ptr<TextButton> bSetButton_;
    std::unique_ptr<MelissaIncDecButton> bButton_;
    std::unique_ptr<TextButton> resetButton_;
    
    std::unique_ptr<Label> speedLabel_;
    std::unique_ptr<MelissaIncDecButton> speedButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncPerButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncValueButton_;
    std::unique_ptr<MelissaIncDecButton> speedIncMaxButton_;
    
    std::unique_ptr<Label> pitchLabel_;
    std::unique_ptr<MelissaIncDecButton> pitchButton_;
    
    std::unique_ptr<ToggleButton> browseToggleButton_;
    std::unique_ptr<ToggleButton> setListToggleButton_;
    std::unique_ptr<ToggleButton> recentToggleButton_;
    std::unique_ptr<WildcardFileFilter> wildCardFilter_;
    std::unique_ptr<FileBrowserComponent> fileBrowserComponent_;
    std::unique_ptr<MelissaRecentListBox> recentTable_;
    
    std::unique_ptr<ToggleButton> practiceListToggleButton_;
    std::unique_ptr<ToggleButton> memoToggleButton_;
    std::unique_ptr<TextEditor> pracListNameTextEditor_;
    std::unique_ptr<TextEditor> memoTextEditor_;
    std::unique_ptr<TextButton> addToListButton_;
    std::unique_ptr<MelissaPracticeTableListBox> practiceTable_;
    
    std::unique_ptr<MelissaPreferencesComponent> preferencesComponent_;
    
    void showPreferencesDialog();
    
    enum
    {
        kSectionTitle_Loop,
        kSectionTitle_Speed,
        kNumOfSectionTitles
    };
    std::vector<std::unique_ptr<MelissaSectionTitleComponent>> sectionTitles_;
    
    MelissaLookAndFeel lookAndFeel_;
    MelissaLookAndFeel_Tab lookAndFeelTab_;
    MelissaLookAndFeel_Memo lookAndFeelMemo_;
    
    String fileName_, fileFullPath_;
    
    File settingsDir_, settingsFile_;
    var setting_;
    Array<var>* recent_;
    
    bool shouldExit_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
