//
//  MelissaDataSource.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaAudioEngine.h"
#include "MelissaDefinitions.h"
#include "MelissaModel.h"

#define SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST

enum FileLoadStatus
{
    kFileLoadStatus_Success,
    kFileLoadStatus_Failed,
    kFileLoadStatus_Loading,
};

class MelissaDataSourceListener
{
public:
    virtual ~MelissaDataSourceListener() { }
    
    virtual void songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate) { }
    virtual void historyUpdated() { }
    virtual void playlistUpdated(size_t index) { }
    virtual void practiceListUpdated() { }
    virtual void markerUpdated() { }
    virtual void fileLoadStatusChanged(FileLoadStatus status, const String& filePath) { }
    virtual void shortcutUpdated() { }
    virtual void colourChanged(const Colour& mainColour, const Colour& subColour, const Colour& accentColour, const Colour& textColour, const Colour& waveformColour) { }
    virtual void fontChanged(const Font& mainFont, const Font& subFont, const Font& miniFont) { }
};

class MelissaDataSource : public AsyncUpdater
{
public:
    typedef Array<String> FilePathList;
    
    struct Global
    {
        String version_;
        String rootDir_;
        int width_;
        int height_;
        String device_;
        int playMode_;
        std::map<String, String> shortcut_;
        String uiTheme_;
        String fontName_;
        enum FontSize
        {
            kFontSize_Main,
            kFontSize_Sub,
            kFontSize_Small,
            kNumFontSizes
        };
        
        Global() : version_(ProjectInfo::versionString), width_(1400), height_(860), uiTheme_("System_Dark")
        {
            rootDir_ = File::getSpecialLocation(File::userMusicDirectory).getFullPathName();
        }
    } global_;
    
    struct Previous
    {
        String filePath_;
        float pitch_;
        
        float aRatio_;
        float bRatio_;
        
        OutputMode outputMode_;
        float musicVolume_;
        float metronomeVolume_;
        float volumeBalance_;
        
        //bool metronomeSw_;
        int bpm_;
        int accent_;
        float beatPositionMSec_;

        SpeedMode speedMode_;
        int speed_;
        int speedIncStart_;
        int speedIncValue_;
        int speedIncPer_;
        int speedIncGoal_;
        
        bool eqSw_;
        float eqFreq_;
        float eqGain_;
        float eqQ_;
        
        // ui state
        struct UIState
        {
            int selectedFileBrowserTab_;
            int selectedPlaylist_;
        };
        UIState uiState_;
        
        Previous() :
        filePath_(""), pitch_(0.f),
        aRatio_(0.f), bRatio_(1.f),
        outputMode_(kOutputMode_LR), musicVolume_(1.f), metronomeVolume_(1.f), volumeBalance_(0.5f),
        /* metronomeSw_(false), */ bpm_(kBpmShouldMeasure), accent_(4), beatPositionMSec_(0.f),
        speedMode_(kSpeedMode_Basic), speed_(100), speedIncStart_(70), speedIncValue_(1), speedIncPer_(10), speedIncGoal_(100),
        eqSw_(false), eqFreq_(500), eqGain_(0.f), eqQ_(1.f),
        uiState_({0, 0})
        {}
    } previous_;
    
    FilePathList history_;
    
    struct Playlist
    {
        String name_;
        FilePathList list_;
        
        Playlist() : name_("Playlist") {}
    };
    std::vector<Playlist> playlists_;
    
    struct Song
    {
        String filePath_;
        float pitch_;
        OutputMode outputMode_;
        float musicVolume_;
        float metronomeVolume_;
        float volumeBalance_;
        
        bool metronomeSw_;
        int bpm_;
        int accent_;
        float beatPositionMSec_;
        
        SpeedMode speedMode_;
        int speed_;
        int speedIncStart_;
        int speedIncValue_;
        int speedIncPer_;
        int speedIncGoal_;
        
        bool eqSw_;
        float eqFreq_;
        float eqGain_;
        float eqQ_;
        String memo_;
        
        struct PracticeList
        {
            String name_;
            float aRatio_;
            float bRatio_;
            
#if !defined(SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST)
            OutputMode outputMode_;
            float musicVolume_;
            float metronomeVolume_;
            float volumeBalance_;
            
            bool metronomeSw_;
            int bpm_;
            int accent_;
            float beatPositionMSec_;
#endif

            int speed_;
#if defined(ENABLE_SPEED_TRAINING)
            SpeedMode speedMode_;
            int speedIncStart_;
            int speedIncValue_;
            int speedIncPer_;
            int speedIncGoal_;
#endif
            
            PracticeList() : name_(""), aRatio_(0.f), bRatio_(1.f),
#if !defined(SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST)
            outputMode_(kOutputMode_LR), musicVolume_(1.f), metronomeVolume_(1.f), volumeBalance_(0.5f),
            metronomeSw_(false), bpm_(kBpmShouldMeasure), accent_(4), beatPositionMSec_(0.f),
#endif
            speed_(100)
#if defined(ENABLE_SPEED_TRAINING)
            ,speedMode_(kSpeedMode_Basic), speedIncStart_(70), speedIncValue_(1), speedIncPer_(10), speedIncGoal_(100)
#endif
            {}
        };
        std::vector<PracticeList> practiceList_;
        
        struct Marker
        {
            float position_;
            int colourR_, colourG_, colourB_;
            String memo_;
        };
        std::vector<Marker> markers_;
        
        Song() : filePath_(""), pitch_(0.f), outputMode_(kOutputMode_LR), musicVolume_(1.f), metronomeVolume_(1.f), volumeBalance_(0.5f),
        metronomeSw_(false), bpm_(kBpmShouldMeasure), accent_(4), beatPositionMSec_(0.f),
        speedMode_(kSpeedMode_Basic), speed_(100), speedIncStart_(70), speedIncValue_(1), speedIncPer_(10), speedIncGoal_(100),
        eqSw_(false), eqFreq_(500), eqGain_(0.f), eqQ_(1.f), memo_("") {}
    };
    std::vector<Song> songs_;
    
    void setMelissaAudioEngine(MelissaAudioEngine* audioEngine) { audioEngine_ = audioEngine; }
    void addListener(MelissaDataSourceListener* listener) { listeners_.emplace_back(listener); }
    void removeListener(MelissaDataSourceListener* listener);
    
    void loadSettingsFile(const File& file);
    void validateSettings();
    void saveSettingsFile();
    const String& getCurrentSongFilePath() { return currentSongFilePath_; }
    
    // Font
    void initFontSettings(const String& fontName = "");
    bool isFontAvailable(const String& fontName) const;
    String getFontName() const { return global_.fontName_; }
    Font getFont(Global::FontSize size) const;
    
    bool isFileLoaded() const { return audioSampleBuf_ != nullptr; }
    static String getCompatibleFileExtensions();
    void loadFileAsync(const File& file, std::function<void()> functionToCallAfterFileLoad = nullptr);
    void loadFileAsync(const String& filePath, std::function<void()> functionToCallAfterFileLoad = nullptr) { loadFileAsync(File(filePath), functionToCallAfterFileLoad); }
    float readBuffer(size_t ch, size_t index);
    double getSampleRate() const { return sampleRate_; }
    size_t getBufferLength() const { return (audioSampleBuf_ == nullptr ? 0 : audioSampleBuf_->getNumSamples()); }
    void disposeBuffer();
    
    // Shortcut
    void setDefaultShortcut(const String& eventName);
    void setDefaultShortcuts(bool removeAll = false);
    std::map<String, String> getAllAssignedShortcuts() const;
    String getAssignedShortcut(const String& eventName);
    void registerShortcut(const String& eventName, const String& command);
    void deregisterShortcut(const String& eventName);
    
    // UI Theme
    String getUITheme() const;
    void setUITheme(const String& uiTheme_);
    
    // Previous
    void restorePreviousState();
    Previous::UIState getPreviousUIState() const { return previous_.uiState_; };
    void saveUIState(const Previous::UIState& uiState) { previous_.uiState_ = uiState; }
    
    // History
    void removeFromHistory(size_t index);
    
    // Playlist
    size_t getNumOfPlaylists() { return playlists_.size(); }
    String getPlaylistName(size_t index) const;
    void   setPlaylistName(size_t index, const String& name);
    void   getPlaylist(size_t index, FilePathList& list) const;
    void   setPlaylist(size_t index, const FilePathList& list);
    void   addToPlaylist(size_t index, const String& filePath);
    void   removeFromPlaylist(size_t playlistIndex, size_t index);
    size_t createPlaylist(const String& name);
    void   removePlaylist(size_t index);
    
    // Song (Current)
    void saveSongState();
    String getMemo() const;
    void saveMemo(const String& memo);
    
    // Practice list
    void getPracticeList(std::vector<Song::PracticeList>& list);
    size_t getNumPracticeList() const;
    void addPracticeList(const String& name);
    void removePracticeList(size_t index);
    void overwritePracticeList(size_t index, const String& name);
    void overwritePracticeList(size_t index, const Song::PracticeList& list);
    void swapPracticeList(size_t indexA, size_t indexB);
    
    // Marker
    void getMarkers(std::vector<Song::Marker>& markers) const;
    void addMarker(const Song::Marker& marker);
    void addDefaultMarker(float position);
    void removeMarker(size_t index);
    void overwriteMarker(size_t index, const Song::Marker& marker);
    
    // AsyncUpdater
    void handleAsyncUpdate() override;
    
    // Singleton
    static MelissaDataSource* getInstance() { return &instance_; }
    MelissaDataSource(const MelissaDataSource&) = delete;
    MelissaDataSource& operator=(const MelissaDataSource&) = delete;
    MelissaDataSource(MelissaDataSource&&) = delete;
    MelissaDataSource& operator=(MelissaDataSource&&) = delete;
    
private:
    // Singleton
    MelissaDataSource();
    ~MelissaDataSource();
    static MelissaDataSource instance_;
    
    // History
    void addToHistory(const String& filePath);
    
    MelissaAudioEngine* audioEngine_;
    MelissaModel* model_;
    double sampleRate_;
    File settingsFile_;
    String currentSongFilePath_;
    File fileToload_;
    std::function<void()> functionToCallAfterFileLoad_;
    std::vector<MelissaDataSourceListener*> listeners_;
    std::unique_ptr<AudioSampleBuffer> audioSampleBuf_;
    bool wasPlaying_;
    std::map<String, String> defaultShortcut_;
};
