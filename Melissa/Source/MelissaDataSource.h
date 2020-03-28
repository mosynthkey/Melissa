#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaModel.h"

class MelissaDataSourceListener
{
public:
    virtual ~MelissaDataSourceListener() { }
    
    virtual void songChanged(const String& filePath, const float* buffer[], size_t bufferLength, int32_t sampleRate) { }
    virtual void songLoadFailed(const String& filePath) { }
    virtual void historyUpdated() { }
};

class MelissaDataSource
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
    
    void setMelissa(Melissa* melissa) { melissa_ = melissa; }
    
    void loadSettingsFile(const File& file);
    void saveSettingsFile();
    
    bool loadFile(const File& file);
    bool loadFile(const String& filePath) { return loadFile(File(filePath)); }
    
    void restorePreviousState();
    
    void addToCurrentPracticeList(const String& name);
    
    void addListener(MelissaDataSourceListener* listener);
    
    const String& getCurrentSongFilePath() { return currentSongFilePath_; }
    
    // Singleton
    static MelissaDataSource* getInstance() { return &instance_; }
    MelissaDataSource(const MelissaDataSource&) = delete;
    MelissaDataSource& operator=(const MelissaDataSource&) = delete;
    MelissaDataSource(MelissaDataSource&&) = delete;
    MelissaDataSource& operator=(MelissaDataSource&&) = delete;
    
private:
    MelissaDataSource();
    ~MelissaDataSource() {}
    
    Melissa* melissa_;
    MelissaModel* model_;
    File settingsFile_;
    
    String currentSongFilePath_;
    
    std::vector<MelissaDataSourceListener*> listeners_;
    
    std::unique_ptr<AudioSampleBuffer> audioSampleBuf_;
    
    // Singleton
    static MelissaDataSource instance_;
};
