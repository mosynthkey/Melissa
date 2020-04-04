#include "AppConfig.h"
#include "MelissaDataSource.h"

enum
{
    kMaxSizeOfHistoryList = 20,
};

MelissaDataSource MelissaDataSource::instance_;

MelissaDataSource::MelissaDataSource() :
model_(MelissaModel::getInstance()),
sampleRate_(0.f),
currentSongFilePath_("")
{
    validateSettings();
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
        if (g->hasProperty("device"))   global_.device_  = g->getProperty("device");
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
    
    history_.clear();
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
    
    playlists_.clear();
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
    
    songs_.clear();
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

    validateSettings();
}

void MelissaDataSource::validateSettings()
{
    if (playlists_.size() == 0)
    {
        Playlist playlist;
        playlists_.emplace_back(playlist);
    }
    for (auto l : listeners_) l->playlistUpdated(0);
}

void MelissaDataSource::saveSettingsFile()
{
    auto settings = new DynamicObject();
    
    auto global = new DynamicObject();
    global->setProperty("version",  global_.version_);
    global->setProperty("root_dir", global_.rootDir_);
    global->setProperty("width",    global_.width_);
    global->setProperty("height",   global_.height_);
    global->setProperty("device",   global_.device_);
    settings->setProperty("global", global);
    
    auto previous = new DynamicObject();
    previous->setProperty("file",   currentSongFilePath_);
    previous->setProperty("volume", model_->getVolume());
    previous->setProperty("a",      model_->getLoopAPosRatio());
    previous->setProperty("b",      model_->getLoopBPosRatio());
    previous->setProperty("speed",  model_->getSpeed());
    previous->setProperty("pitch",  model_->getPitch());
    settings->setProperty("previous", previous);
    
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
        obj->setProperty("pitch",  song.pitch_);
        obj->setProperty("memo",   song.memo_);
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

void MelissaDataSource::loadFileAsync(const File& file, std::function<void()> functionToCallAfterFileLoad)
{
    functionToCallAfterFileLoad_ = functionToCallAfterFileLoad;
    if (file.existsAsFile())
    {
        fileToload_ = file;
        for (auto l : listeners_) l->fileLoadStatusChanged(kFileLoadStatus_Loading, file.getFullPathName());
        cancelPendingUpdate();
        triggerAsyncUpdate();
    }
    else
    {
        for (auto l : listeners_) l->fileLoadStatusChanged(kFileLoadStatus_Failed, file.getFullPathName());
    }
}

float MelissaDataSource::readBuffer(size_t ch, size_t index)
{
    if (audioSampleBuf_ == nullptr) return 0.f;
    
    const int numOfChs   = audioSampleBuf_->getNumChannels();
    const int bufferSize = audioSampleBuf_->getNumSamples();
    
    if (2 <= ch || numOfChs <= ch) ch = 0;
    if (bufferSize <= index) return 0.f;
    
    return audioSampleBuf_->getSample(ch, index);
}

void MelissaDataSource::restorePreviousState()
{
    File file(previous_.filePath_);
    if (!file.existsAsFile()) return;
    
    loadFileAsync(file, [&]() {
        model_->setVolume(previous_.volume_);
        model_->setLoopPosRatio(previous_.aRatio_, previous_.bRatio_);
        model_->setSpeed(previous_.speed_);
        model_->setPitch(previous_.pitch_);
    });
}

void MelissaDataSource::removeFromHistory(size_t index)
{
    if (history_.size() <= index) return;
    history_.remove(static_cast<int>(index));
    for (auto l : listeners_) l->historyUpdated();
}

String MelissaDataSource::getPlaylistName(size_t index) const
{
    if (playlists_.size() <= index) return "Out of range";
    return playlists_[index].name_;
}

void MelissaDataSource::setPlaylistName(size_t index, const String& name)
{
    if (playlists_.size() <= index) return;
    playlists_[index].name_ = name;
    for (auto l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::getPlaylist(size_t index, FilePathList& list) const
{
    if (playlists_.size() <= index) return;
    list = playlists_[index].list_;
}

void MelissaDataSource::setPlaylist(size_t index, const FilePathList& list)
{
    if (playlists_.size() <= index) return;
    playlists_[index].list_ = list;
    for (auto l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::addToPlaylist(size_t index, const String& filePath)
{
    if (playlists_.size() <= index) return;
    playlists_[index].list_.addIfNotAlreadyThere(filePath);
    for (auto l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::removeFromPlaylist(size_t playlistIndex, size_t index)
{
    if (playlists_.size() <= playlistIndex) return;
    if (playlists_[playlistIndex].list_.size() <= index) return;
    playlists_[playlistIndex].list_.remove(static_cast<int>(index));
    for (auto l : listeners_) l->playlistUpdated(index);
}

size_t MelissaDataSource::createPlaylist(const String& name)
{
    Playlist playlist;
    playlist.name_ = name;
    playlists_.emplace_back(playlist);
    const size_t index = playlists_.size() - 1;
    for (auto l : listeners_) l->playlistUpdated(index);
    return index;
}

void MelissaDataSource::removePlaylist(size_t index)
{
    const auto numOfPlaylists = playlists_.size();
    if (numOfPlaylists <= 1 || numOfPlaylists <= index) return;
    playlists_.erase(playlists_.begin() + index);
    for (auto l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::saveSongState()
{
    if (currentSongFilePath_.isEmpty()) return;
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            song.volume_ = model_->getVolume();
            song.pitch_  = model_->getPitch();
            return;
        }
    }
    
    Song song;
    song.filePath_ = currentSongFilePath_;
    song.volume_ = model_->getVolume();
    song.pitch_  = model_->getPitch();
    songs_.emplace_back(song);
}

void MelissaDataSource::saveMemo(const String& memo)
{
    if (currentSongFilePath_.isEmpty()) return;
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            song.memo_ = memo;
            return;
        }
    }
}

String MelissaDataSource::getMemo() const
{
    if (currentSongFilePath_.isEmpty()) return "";
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            return song.memo_;
        }
    }
    
    return "";
}

void MelissaDataSource::getPracticeList(std::vector<MelissaDataSource::Song::PracticeList>& list)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            list = song.practiceList_;
            return;
        }
    }
}

void MelissaDataSource::addPracticeList(const String& name)
{
    if (currentSongFilePath_.isEmpty()) return;
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            Song::PracticeList plist;
            plist.name_   = name;
            plist.aRatio_ = model_->getLoopAPosRatio();
            plist.bRatio_ = model_->getLoopBPosRatio();
            plist.speed_  = model_->getSpeed();
            song.practiceList_.emplace_back(plist);
            for (auto l : listeners_) l->practiceListUpdated();
            return;
        }
    }
}

void MelissaDataSource::removePracticeList(size_t index)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            if (index < song.practiceList_.size())
            {
                song.practiceList_.erase(song.practiceList_.begin() + index);
                for (auto l : listeners_) l->practiceListUpdated();
                return;
            }
        }
    }
}

void MelissaDataSource::overwritePracticeList(size_t index, const String& name, float aRatio, float bRatio, int speed)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            if (index < song.practiceList_.size())
            {
                song.practiceList_[index].name_   = name;
                song.practiceList_[index].aRatio_ = aRatio;
                song.practiceList_[index].bRatio_ = bRatio;
                song.practiceList_[index].speed_  = speed;
                for (auto l : listeners_) l->practiceListUpdated();
            }
            return;
        }
    }
}

void MelissaDataSource::handleAsyncUpdate()
{
    // load file asynchronously
    
    saveSongState();
    
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    auto* reader = formatManager.createReaderFor(fileToload_);
    if (reader == nullptr) return;
    
    // read audio data from reader
    const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
    audioSampleBuf_ = std::make_unique<AudioSampleBuffer>(2, lengthInSamples);
    reader->read(audioSampleBuf_.get(), 0, lengthInSamples, 0, true, true);
    sampleRate_ = reader->sampleRate;
    
    currentSongFilePath_ = fileToload_.getFullPathName();
    audioEngine_->updateBuffer();
    
    for (auto l : listeners_)
    {
        l->fileLoadStatusChanged(kFileLoadStatus_Success, currentSongFilePath_);
        l->songChanged(currentSongFilePath_, lengthInSamples, sampleRate_);
    }
    
    bool found = false;
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            found = true;
            model_->setVolume(song.volume_);
            model_->setPitch(song.pitch_);
        }
    }
    if (!found)
    {
        model_->setVolume(1.f);
        model_->setPitch(0);
    }
    model_->setLengthMSec(lengthInSamples / reader->sampleRate * 1000.f);
    model_->setLoopPosRatio(0.f, 1.f);
    model_->setPlayingPosRatio(0.f);
    
    addToHistory(currentSongFilePath_);
    saveSongState();
    
    if (functionToCallAfterFileLoad_ != nullptr) functionToCallAfterFileLoad_();
    
    delete reader;
}

void MelissaDataSource::addToHistory(const String& filePath)
{
    history_.removeFirstMatchingValue(filePath);
    history_.insert(0, filePath);
    if (history_.size() >= kMaxSizeOfHistoryList)
    {
        history_.resize(kMaxSizeOfHistoryList);
    }
    
    for (auto l : listeners_) l->historyUpdated();
}
