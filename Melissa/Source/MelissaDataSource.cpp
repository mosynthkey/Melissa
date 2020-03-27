#include "AppConfig.h"
#include "MelissaDataSource.h"

MelissaDataSource::MelissaDataSource()
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
    
    if (settings.hasProperty("current"))
    {
        auto c = settings["current"].getDynamicObject();
        if (c->hasProperty("file"))   current_.filePath_ = c->getProperty("file");
        if (c->hasProperty("volume")) current_.volume_   = c->getProperty("volume");
        if (c->hasProperty("a"))      current_.aRatio_   = c->getProperty("a");
        if (c->hasProperty("b"))      current_.bRatio_   = c->getProperty("b");
        if (c->hasProperty("speed"))  current_.speed_    = c->getProperty("speed");
        if (c->hasProperty("pitch"))  current_.pitch_    = c->getProperty("pitch");
    }
    
    if (settings.hasProperty("history"))
    {
        auto array = settings["history"].getArray();
        if (array != nullptr)
        {
            for (auto history : *array)
            {
                history_.emplace_back(history.toString());
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
                    playlist.list_.emplace_back(l.toString());
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
    
    saveSettingsFile();
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
    
    auto current = new DynamicObject();
    current->setProperty("file",   current_.filePath_);
    current->setProperty("volume", current_.volume_);
    current->setProperty("a",      current_.aRatio_);
    current->setProperty("b",      current_.bRatio_);
    current->setProperty("speed",  current_.speed_);
    current->setProperty("pitch",  current_.pitch_);
    settings->setProperty("current", current);
    
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
