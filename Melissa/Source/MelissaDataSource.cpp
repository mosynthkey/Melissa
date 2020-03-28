#include "AppConfig.h"
#include "MelissaDataSource.h"

MelissaDataSource MelissaDataSource::instance_;

MelissaDataSource::MelissaDataSource() :
model_(MelissaModel::getInstance())
{
}

void MelissaDataSource::loadSettingsFile(const File& file)
{
    settingsFile_ = file;
    
    if (!file.existsAsFile()) return;
    
    var settings = JSON::parse(file.loadFileAsString());
    settings = JSON::parse(settingsFile_.loadFileAsString());
    
    if (settings.hasProperty("global"))
    {
        auto g = settings["global"].getDynamicObject();
        if (g->hasProperty("version"))  global_.version_ = g->getProperty("version");
        if (g->hasProperty("root_dir")) global_.rootDir_ = g->getProperty("root_dir");
        if (g->hasProperty("width"))    global_.width_   = g->getProperty("width");
        if (g->hasProperty("height"))   global_.height_  = g->getProperty("height");
    }
    
    if (settings.hasProperty("previous"))
    {
        auto p = settings["previous"].getDynamicObject();
        if (p->hasProperty("file"))   previous_.filePath_ = p->getProperty("file");
        if (p->hasProperty("volume")) previous_.volume_   = p->getProperty("volume");
        if (p->hasProperty("a"))      previous_.aRatio_   = p->getProperty("a");
        if (p->hasProperty("b"))      previous_.bRatio_   = p->getProperty("b");
        if (p->hasProperty("speed"))  previous_.speed_    = p->getProperty("speed");
        if (p->hasProperty("pitch"))  previous_.pitch_    = p->getProperty("pitch");
    }
    
    if (settings.hasProperty("history"))
    {
        auto array = settings["history"].getArray();
        if (array != nullptr)
        {
            for (auto history : *array)
            {
                history_.add(history.toString());
            }
        }
    }
    
    if (settings.hasProperty("playlist"))
    {
        auto playlists = settings["playlist"].getArray();
        if (playlists != nullptr)
        {
            for (auto p : *playlists)
            {
                auto obj = p.getDynamicObject();
                Playlist playlist;
                playlist.name_ = obj->getProperty("name");
                for (auto l : *(obj->getProperty("list").getArray()))
                {
                    playlist.list_.add(l.toString());
                }
                playlists_.emplace_back(playlist);
            }
        }
    }
    
    if (settings.hasProperty("songs"))
    {
        auto songs = settings["songs"].getArray();
        if (songs != nullptr)
        {
            for (auto s : *songs)
            {
                auto obj = s.getDynamicObject();
                Song song;
                song.filePath_ = obj->getProperty("file");
                song.volume_   = obj->getProperty("volume");
                song.pitch_    = obj->getProperty("pitch");
                song.memo_     = obj->getProperty("memo");
                for (auto l : *(obj->getProperty("list").getArray()))
                {
                    Song::PracticeList list;
                    list.name_   = l.getProperty("name", "");
                    list.aRatio_ = l.getProperty("a", 0.f);
                    list.bRatio_ = l.getProperty("b", 1.f);
                    list.speed_  = l.getProperty("speed", 100);
                    song.practiceList_.emplace_back(list);
                }
                songs_.emplace_back(song);
            }
        }
    }
}

void MelissaDataSource::saveSettingsFile()
{
    auto settings = new DynamicObject();
    
    auto global = new DynamicObject();
    global->setProperty("version",  global_.version_);
    global->setProperty("root_dir", global_.rootDir_);
    global->setProperty("width",    global_.width_);
    global->setProperty("height",   global_.height_);
    settings->setProperty("global", global);
    
    auto previous = new DynamicObject();
    previous->setProperty("file",   previous_.filePath_);
    previous->setProperty("volume", previous_.volume_);
    previous->setProperty("a",      previous_.aRatio_);
    previous->setProperty("b",      previous_.bRatio_);
    previous->setProperty("speed",  previous_.speed_);
    previous->setProperty("pitch",  previous_.pitch_);
    settings->setProperty("current", previous);
    
    Array<var> history;
    for (auto h : history_)
    {
        history.addIfNotAlreadyThere(h);
    }
    settings->setProperty("history", history);
    
    Array<var> playlists;
    for (auto playlist : playlists_)
    {
        auto obj = new DynamicObject();
        obj->setProperty("name", playlist.name_);
        Array<var> list;
        for (auto l : playlist.list_) list.add(l);
        obj->setProperty("list", list);
        playlists.add(obj);
    }
    settings->setProperty("playlist", playlists);
    
    Array<var> songs;
    for (auto song : songs_)
    {
        auto obj = new DynamicObject();
        obj->setProperty("file",   song.filePath_);
        obj->setProperty("volume", song.volume_);
        obj->setProperty("pitch",   song.pitch_);
        Array<var> list;
        for (auto l : song.practiceList_)
        {
            auto obj = new DynamicObject();
            obj->setProperty("name",  l.name_);
            obj->setProperty("a",     l.aRatio_);
            obj->setProperty("b",     l.bRatio_);
            obj->setProperty("speed", l.speed_);
            list.add(obj);
        }
        obj->setProperty("list", list);
        songs.add(obj);
    }
    settings->setProperty("songs", songs);
    
    settingsFile_.replaceWithText(JSON::toString(settings));
}

bool MelissaDataSource::loadFile(const File& file)
{
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
    {
        for (auto l : listeners_) l->songLoadFailed(file.getFullPathName());
        return false;
    }
    
    // read audio data from reader
    const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
    audioSampleBuf_ = std::make_unique<AudioSampleBuffer>(2, lengthInSamples);
    reader->read(audioSampleBuf_.get(), 0, lengthInSamples, 0, true, true);
    
    currentSongFilePath_ = file.getFullPathName();
    
    const float* buffer[] = { audioSampleBuf_->getReadPointer(0), audioSampleBuf_->getReadPointer(1) };
    melissa_->setBuffer(buffer, lengthInSamples, reader->sampleRate);
    for (auto l : listeners_) l->songChanged(currentSongFilePath_, buffer, lengthInSamples, reader->sampleRate);
    
    model_->setVolume(1.f);
    model_->setLengthMSec(lengthInSamples / reader->sampleRate * 1000.f);
    
    delete reader;
    
    return true;
}

void MelissaDataSource::restorePreviousState()
{
    File file(previous_.filePath_);
    if (!file.existsAsFile()) return;
    
    model_->setVolume(previous_.volume_);
    model_->setLoopPosRatio(previous_.aRatio_, previous_.bRatio_);
    model_->setSpeed(previous_.speed_);
    model_->setPitch(previous_.pitch_);
}

void MelissaDataSource::addToCurrentPracticeList(const String& name)
{
    
}

void MelissaDataSource::addListener(MelissaDataSourceListener* listener)
{
    listeners_.emplace_back(listener);
}
