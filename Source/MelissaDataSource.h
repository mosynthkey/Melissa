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
    
    virtual void songChanged(const juce::String& filePath, size_t bufferLength, int32_t sampleRate) { }
    virtual void historyUpdated() { }
    virtual void playlistUpdated(size_t index) { }
    virtual void practiceListUpdated() { }
    virtual void markerUpdated() { }
    virtual void fileLoadStatusChanged(FileLoadStatus status, const juce::String& filePath) { }
    virtual void shortcutUpdated() { }
    virtual void colourChanged(const juce::Colour& mainColour, const juce::Colour& subColour, const juce::Colour& accentColour, const juce::Colour& textColour, const juce::Colour& waveformColour) { }
    virtual void fontChanged(const juce::Font& mainFont, const juce::Font& subFont, const juce::Font& miniFont) { }
};

class MelissaDataSource : public juce::AsyncUpdater
{
public:
    typedef juce::Array<juce::String> FilePathList;
    
    struct Global
    {
        juce::String version_;
        juce::String rootDir_;
        int width_;
        int height_;
        juce::String device_;
        int playMode_;
        std::map<juce::String, juce::String> shortcut_;
        juce::String uiTheme_;
        juce::String fontName_;
        enum FontSize
        {
            kFontSize_Large,
            kFontSize_Main,
            kFontSize_Sub,
            kFontSize_Small,
            kFontSize_Smallest,
            kNumFontSizes
        };
        
        Global() : version_(ProjectInfo::versionString), width_(1400), height_(860), uiTheme_("System_Dark")
        {
            rootDir_ = juce::File::getSpecialLocation(juce::File::userMusicDirectory).getFullPathName();
        }
    } global_;
    
    struct Previous
    {
        juce::String filePath_;
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

        PlayPart playPart_;
        float vocalVolume_;
        float pianoVolume_;
        float bassVolume_;
        float drumsVolume_;
        float othersVolume_;
        
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
        eqSw_(false), eqFreq_(500), eqGain_(0.f), eqQ_(1.f), playPart_(kPlayPart_All), vocalVolume_(0.f), pianoVolume_(0.f), bassVolume_(0.f), drumsVolume_(0.f), othersVolume_(0.f),
        uiState_({0, 0})
        {}
    } previous_;
    
    FilePathList history_;
    
    struct Playlist
    {
        juce::String name_;
        FilePathList list_;
        
        Playlist() : name_("Playlist") {}
    };
    std::vector<Playlist> playlists_;
    
    struct Song
    {
        juce::String filePath_;
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
        juce::String memo_;
        
        struct PracticeList
        {
            juce::String name_;
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
            juce::String memo_;
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
    
    void loadSettingsFile(const juce::File& file);
    void validateSettings();
    void saveSettingsFile();
    const juce::String& getCurrentSongFilePath() { return currentSongFilePath_; }
    
    // juce::Font
    void initFontSettings(const juce::String& fontName = "");
    bool isFontAvailable(const juce::String& fontName) const;
    juce::String getFontName() const { return global_.fontName_; }
    juce::Font getFont(Global::FontSize size) const;
    
    bool isFileLoaded() const { return originalAudioReaders_[kReader_Playback] != nullptr; }
    static juce::String getCompatibleFileExtensions();
    void loadFileAsync(const juce::File& file, std::function<void()> functionToCallAfterFileLoad = nullptr);
    void loadFileAsync(const juce::String& filePath, std::function<void()> functionToCallAfterFileLoad = nullptr) { loadFileAsync(juce::File(filePath), functionToCallAfterFileLoad); }
    
    enum Reader
    {
        kReader_Playback,
        kReader_Waveform,
        kReader_BPM,
        kNumReaders
    };
    bool readBuffer(Reader reader, size_t startIndex, int numSamplesToRead, PlayPart playPart, float* const* destChannels);
    
    double getSampleRate() const { return sampleRate_; }
    size_t getBufferLength() const { return (originalAudioReaders_[kReader_Playback] == nullptr ? 0 : originalAudioReaders_[kReader_Playback]->lengthInSamples); }
    void disposeBuffer();
    
    // Shortcut
    void setDefaultShortcut(const juce::String& eventName);
    void setDefaultShortcuts(bool removeAll = false);
    std::map<juce::String, juce::String> getAllAssignedShortcuts() const;
    juce::String getAssignedShortcut(const juce::String& eventName);
    void registerShortcut(const juce::String& eventName, const juce::String& command);
    void deregisterShortcut(const juce::String& eventName);
    
    // UI Theme
    juce::String getUITheme() const;
    void setUITheme(const juce::String& uiTheme_);
    
    // Previous
    void restorePreviousState();
    Previous::UIState getPreviousUIState() const { return previous_.uiState_; };
    void saveUIState(const Previous::UIState& uiState) { previous_.uiState_ = uiState; }
    
    // History
    void removeFromHistory(size_t index);
    
    // Playlist
    size_t getNumOfPlaylists() { return playlists_.size(); }
    juce::String getPlaylistName(size_t index) const;
    void   setPlaylistName(size_t index, const juce::String& name);
    void   getPlaylist(size_t index, FilePathList& list) const;
    void   setPlaylist(size_t index, const FilePathList& list);
    void   addToPlaylist(size_t index, const juce::String& filePath);
    void   removeFromPlaylist(size_t playlistIndex, size_t index);
    size_t createPlaylist(const juce::String& name);
    void   removePlaylist(size_t index);
    
    // Song (Current)
    void saveSongState();
    juce::String getMemo() const;
    void saveMemo(const juce::String& memo);
    
    // Practice list
    void getPracticeList(std::vector<Song::PracticeList>& list);
    size_t getNumPracticeList() const;
    void addPracticeList(const juce::String& name);
    void removePracticeList(size_t index);
    void overwritePracticeList(size_t index, const juce::String& name);
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
    void addToHistory(const juce::String& filePath);
    
    MelissaAudioEngine* audioEngine_;
    MelissaModel* model_;
    double sampleRate_;
    juce::File settingsFile_;
    juce::String currentSongFilePath_;
    juce::File fileToload_;
    std::map<std::string, juce::File> stemFiles_;
    std::function<void()> functionToCallAfterFileLoad_;
    std::vector<MelissaDataSourceListener*> listeners_;
    
    std::unique_ptr<juce::AudioFormatReader> originalAudioReaders_[kNumReaders];
    std::unique_ptr<juce::AudioFormatReader> stemAudioReaders_[kNumStemFiles];
    
    bool wasPlaying_;
    std::map<juce::String, juce::String> defaultShortcut_;
};
