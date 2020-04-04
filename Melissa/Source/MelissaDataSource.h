#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaAudioEngine.h"
#include "MelissaModel.h"

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
    virtual void fileLoadStatusChanged(FileLoadStatus status, const String& filePath) { }
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
        
        Global() : version_(ProjectInfo::versionString), width_(1400), height_(860)
        {
            rootDir_ = File::getSpecialLocation(File::userMusicDirectory).getFullPathName();
        }
    } global_;
    
    struct Previous
    {
        String filePath_;
        float volume_;
        float aRatio_;
        float bRatio_;
        int speed_;
        int pitch_;
        
        Previous() : filePath_(""), volume_(1.f), aRatio_(0.f), bRatio_(1.f), speed_(100), pitch_(0) {}
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
        float volume_;
        int pitch_;
        String memo_;
        struct PracticeList
        {
            String name_;
            float aRatio_;
            float bRatio_;
            int speed_;
            
            PracticeList() : name_(""), aRatio_(0.f), bRatio_(1.f), speed_(100) {}
        };
        std::vector<PracticeList> practiceList_;
        
        Song() : filePath_(""), volume_(1.f), pitch_(0), memo_("") {}
    };
    std::vector<Song> songs_;
    
    void setMelissaAudioEngine(MelissaAudioEngine* audioEngine) { audioEngine_ = audioEngine; }
    void addListener(MelissaDataSourceListener* listener) { listeners_.emplace_back(listener); }
    
    void loadSettingsFile(const File& file);
    void validateSettings();
    void saveSettingsFile();
    const String& getCurrentSongFilePath() { return currentSongFilePath_; }
    
    static String getCompatibleFileExtensions() { return "*.mp3;*.wav;*.m4a;*.flac;*.ogg"; }
    void loadFileAsync(const File& file, std::function<void()> functionToCallAfterFileLoad = nullptr);
    void loadFileAsync(const String& filePath, std::function<void()> functionToCallAfterFileLoad = nullptr) { loadFileAsync(File(filePath), functionToCallAfterFileLoad); }
    float readBuffer(size_t ch, size_t index);
    double getSampleRate() const { return sampleRate_; }
    size_t getBufferLength() const { return (audioSampleBuf_ == nullptr ? 0 : audioSampleBuf_->getNumSamples()); }
    
    // Previous
    void restorePreviousState();
    
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
    void getPracticeList(std::vector<Song::PracticeList>& list);
    void addPracticeList(const String& name);
    void removePracticeList(size_t index);
    void overwritePracticeList(size_t index, const String& name, float aRatio, float bRatio, int speed);
    
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
    ~MelissaDataSource() {}
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
};
