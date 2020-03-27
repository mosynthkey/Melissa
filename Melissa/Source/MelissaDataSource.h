#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaDataSourceListener
{
public:
    virtual ~MelissaDataSourceListener() { }
    virtual void historyChanged() { }
};

class MelissaDataSource
{
public:
    typedef std::vector<String> FilePathList;
    
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
    
    struct Current
    {
        String filePath_;
        float volume_;
        float aRatio_;
        float bRatio_;
        int speed_;
        int pitch_;
        
        Current() : filePath_(""), volume_(1.f), aRatio_(0.f), bRatio_(1.f), speed_(100), pitch_(0) {}
    } current_;
    
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
    
    MelissaDataSource();
    ~MelissaDataSource() {}
    
    void loadSettingsFile(const File& file);
    void saveSettingsFile();
    
private:
    File settingsFile_;
};
