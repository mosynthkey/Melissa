//
//  MelissaDataSource.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

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
currentSongFilePath_(""),
wasPlaying_(false)
{
    validateSettings();
}

MelissaDataSource::~MelissaDataSource()
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
        if (g->hasProperty("device"))   global_.device_  = g->getProperty("device");
    }
    
    if (settings.hasProperty("previous"))
    {
        auto p = settings["previous"].getDynamicObject();
        if (p->hasProperty("file"))   previous_.filePath_    = p->getProperty("file");
        if (p->hasProperty("pitch"))  previous_.pitch_       = p->getProperty("pitch");
        
        if (p->hasProperty("a"))      previous_.aRatio_      = p->getProperty("a");
        if (p->hasProperty("b"))      previous_.bRatio_      = p->getProperty("b");
        
        const int outputMode = p->getProperty("output_mode");
        if (p->hasProperty("output_mode"))     previous_.outputMode_       = static_cast<OutputMode>(outputMode);
        if (p->hasProperty("volume"))           previous_.musicVolume_     = p->getProperty("volume");
        if (p->hasProperty("metronome_volume")) previous_.metronomeVolume_ = p->getProperty("metronome_volume");
        if (p->hasProperty("volume_balance"))   previous_.volumeBalance_   = p->getProperty("volume_balance");
        
        if (p->hasProperty("metronome_sw"))  previous_.metronomeSw_      = p->getProperty("metronome_sw");
        if (p->hasProperty("bpm"))           previous_.bpm_              = p->getProperty("bpm");
        if (p->hasProperty("accent"))        previous_.accent_           = p->getProperty("accent");
        if (p->hasProperty("beat_position")) previous_.beatPositionMSec_ = p->getProperty("beat_position");
        
        const int speedMode = p->getProperty("speed_mode");
        if (p->hasProperty("speed_mode"))      previous_.speedMode_     = static_cast<SpeedMode>(speedMode);
        if (p->hasProperty("speed"))           previous_.speed_         = p->getProperty("speed");
        if (p->hasProperty("speed_inc_start")) previous_.speedIncStart_ = p->getProperty("speed_inc_start");
        if (p->hasProperty("speed_inc_value")) previous_.speedIncValue_ = p->getProperty("speed_inc_value");
        if (p->hasProperty("speed_inc_per"))   previous_.speedIncPer_   = p->getProperty("speed_inc_per");
        if (p->hasProperty("speed_inc_goal"))  previous_.speedIncGoal_  = p->getProperty("speed_inc_goal");
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
                song.filePath_         = obj->getProperty("file");
                song.pitch_            = obj->getProperty("pitch");
                const int outputMode   = obj->getProperty("output_mode");
                song.outputMode_  = static_cast<OutputMode>(outputMode);
                song.musicVolume_      = obj->getProperty("music_volume");
                song.metronomeVolume_  = obj->getProperty("metronome_volume");
                song.volumeBalance_    = obj->getProperty("volume_balance");
                song.metronomeSw_      = obj->getProperty("metronome_sw");
                song.bpm_              = obj->getProperty("bpm");
                song.accent_           = obj->getProperty("accent");
                song.beatPositionMSec_ = obj->getProperty("beat_position");
                song.memo_             = obj->getProperty("memo");
                for (auto l : *(obj->getProperty("list").getArray()))
                {
                    Song::PracticeList list;
                    list.name_       = l.getProperty("name", "");
                    list.aRatio_     = l.getProperty("a", 0.f);
                    list.bRatio_     = l.getProperty("b", 1.f);
                    
                    const int outputMode = l.getProperty("output_mode", kOutputMode_LR);
                    list.outputMode_ = static_cast<OutputMode>(outputMode);
                    list.musicVolume_     = l.getProperty("volume", 1.f);
                    list.metronomeVolume_ = l.getProperty("metronome_volume", 1.f);
                    list.volumeBalance_   = l.getProperty("volume_balance", 0.5f);
                    
                    list.metronomeSw_      = l.getProperty("metronome_sw", false);
                    list.bpm_              = l.getProperty("bpm", 120);
                    list.accent_           = l.getProperty("accent", 4);
                    list.beatPositionMSec_ = l.getProperty("beat_position", 0.f);
                    
                    const int speedMode = l.getProperty("speed_mode", kSpeedMode_Basic);
                    list.speedMode_     = static_cast<SpeedMode>(speedMode);
                    list.speed_         = l.getProperty("speed", 100);
                    list.speedIncStart_ = l.getProperty("speed_inc_start", 70);
                    list.speedIncValue_ = l.getProperty("speed_inc_value", 1);
                    list.speedIncPer_   = l.getProperty("speed_inc_per", 10);
                    list.speedIncGoal_  = l.getProperty("speed_inc_goal", 100);
                    
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
    previous->setProperty("pitch",  model_->getPitch());
    previous->setProperty("a",      model_->getLoopAPosRatio());
    previous->setProperty("b",      model_->getLoopBPosRatio());
    
    previous->setProperty("output_mode",      model_->getOutputMode());
    previous->setProperty("volume",           model_->getMusicVolume());
    previous->setProperty("metronome_volume", model_->getMetronomeVolume());
    previous->setProperty("volume_balance",   model_->getMusicMetronomeBalance());
    
    previous->setProperty("metronome_sw",  model_->getMetronomeSwitch());
    previous->setProperty("bpm",           model_->getBpm());
    previous->setProperty("accent",        model_->getAccent());
    previous->setProperty("beat_position", model_->getBeatPositionMSec());
    
    previous->setProperty("speed_mode",      model_->getSpeedMode());
    previous->setProperty("speed",           model_->getSpeed());
    previous->setProperty("speed_inc_start", model_->getSpeedIncStart());
    previous->setProperty("speed_inc_value", model_->getSpeedIncValue());
    previous->setProperty("speed_inc_per",   model_->getSpeedIncPer());
    previous->setProperty("speed_inc_goal",  model_->getSpeedIncGoal());
    
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
        obj->setProperty("file",             song.filePath_);
        obj->setProperty("output_mode",      song.outputMode_);
        obj->setProperty("music_volume",     song.musicVolume_);
        obj->setProperty("metronome_volume", song.metronomeVolume_);
        obj->setProperty("volume_balance",   song.volumeBalance_);
        obj->setProperty("metronome_sw",     song.metronomeSw_);
        obj->setProperty("bpm",              song.bpm_);
        obj->setProperty("accent",           song.accent_);
        obj->setProperty("beat_position",    song.beatPositionMSec_);
        obj->setProperty("memo",             song.memo_);
        Array<var> list;
        for (auto l : song.practiceList_)
        {
            auto obj = new DynamicObject();
            obj->setProperty("name",   l.name_);
            obj->setProperty("a",      l.aRatio_);
            obj->setProperty("b",      l.bRatio_);
            
            obj->setProperty("output", l.outputMode_);
            obj->setProperty("volume", l.musicVolume_);
            obj->setProperty("metronome_volume", l.metronomeVolume_);
            obj->setProperty("volume_balance",   l.volumeBalance_);
            
            obj->setProperty("metronome_sw",  l.metronomeSw_);
            obj->setProperty("bpm",           l.bpm_);
            obj->setProperty("accent",        l.accent_);
            obj->setProperty("beat_position", l.beatPositionMSec_);
            
            obj->setProperty("speed_mode",      l.speedMode_);
            obj->setProperty("speed",           l.speed_);
            obj->setProperty("speed_inc_start", l.speedIncStart_);
            obj->setProperty("speed_inc_value", l.speedIncValue_);
            obj->setProperty("speed_inc_per",   l.speedIncPer_);
            obj->setProperty("speed_inc_goal",  l.speedIncGoal_);
            
            list.add(obj);
        }
        obj->setProperty("list", list);
        songs.add(obj);
    }
    settings->setProperty("songs", songs);
    
    settingsFile_.replaceWithText(JSON::toString(settings));
}



String MelissaDataSource::getCompatibleFileExtensions()
{
#ifdef JUCE_MAC
    return "*.mp3;*.wav;*.m4a;*.flac;*.ogg";
#else
    return "*.mp3;*.wav;*.flac;*.ogg";
#endif
}

void MelissaDataSource::loadFileAsync(const File& file, std::function<void()> functionToCallAfterFileLoad)
{
    functionToCallAfterFileLoad_ = functionToCallAfterFileLoad;
    if (file.existsAsFile())
    {
        fileToload_ = file;
        wasPlaying_ = (model_->getPlaybackStatus() == kPlaybackStatus_Playing);
        model_->setPlaybackStatus(kPlaybackStatus_Stop);
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
    
    return audioSampleBuf_->getSample(static_cast<int>(ch), static_cast<int>(index));
}

void MelissaDataSource::disposeBuffer()
{
    if (audioSampleBuf_ == nullptr) return;
    audioSampleBuf_->clear();
    audioSampleBuf_ = nullptr;
}

void MelissaDataSource::restorePreviousState()
{
    File file(previous_.filePath_);
    if (!file.existsAsFile()) return;
    
    loadFileAsync(file, [&]() {
        model_->setPitch(previous_.pitch_);
        model_->setLoopPosRatio(previous_.aRatio_, previous_.bRatio_);
        model_->setOutputMode(previous_.outputMode_);
        model_->setMusicVolume(previous_.musicVolume_);
        model_->setMetronomeVolume(previous_.metronomeVolume_);
        model_->setMusicMetronomeBalance(previous_.volumeBalance_);
        model_->setMetronomeSwitch(previous_.metronomeSw_);
        model_->setBpm(previous_.bpm_);
        model_->setAccent(previous_.accent_);
        model_->setBeatPositionMSec(previous_.beatPositionMSec_);
        model_->setSpeedMode(previous_.speedMode_);
        model_->setSpeed(previous_.speed_);
        model_->setSpeedIncStart(previous_.speedIncStart_);
        model_->setSpeedIncValue(previous_.speedIncValue_);
        model_->setSpeedIncPer(previous_.speedIncPer_);
        model_->setSpeedIncGoal(previous_.speedIncGoal_);
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
            song.pitch_  = model_->getPitch();
            return;
        }
    }
    
    Song song;
    song.filePath_ = currentSongFilePath_;
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
    list.clear();
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
            
            plist.outputMode_      = model_->getOutputMode();
            plist.musicVolume_     = model_->getMusicVolume();
            plist.metronomeVolume_ = model_->getMetronomeVolume();
            plist.volumeBalance_   = model_->getMusicMetronomeBalance();
            
            plist.metronomeSw_      = model_->getMetronomeSwitch();
            plist.bpm_              = model_->getBpm();
            plist.accent_           = model_->getAccent();
            plist.beatPositionMSec_ = model_->getBeatPositionMSec();

            plist.speedMode_     = model_->getSpeedMode();
            plist.speed_         = model_->getSpeed();
            plist.speedIncStart_ = model_->getSpeedIncStart();
            plist.speedIncValue_ = model_->getSpeedIncValue();
            plist.speedIncPer_   = model_->getSpeedIncPer();
            plist.speedIncGoal_  = model_->getSpeedIncGoal();
            
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

void MelissaDataSource::overwritePracticeList(size_t index, const String& name)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            if (index < song.practiceList_.size())
            {
                 song.practiceList_[index].name_   = name;
                 song.practiceList_[index].aRatio_ = model_->getLoopAPosRatio();
                 song.practiceList_[index].bRatio_ = model_->getLoopBPosRatio();
                 
                 song.practiceList_[index].outputMode_      = model_->getOutputMode();
                 song.practiceList_[index].musicVolume_     = model_->getMusicVolume();
                 song.practiceList_[index].metronomeVolume_ = model_->getMetronomeVolume();
                 song.practiceList_[index].volumeBalance_   = model_->getMusicMetronomeBalance();
                 
                 song.practiceList_[index].metronomeSw_ = model_->getMetronomeSwitch();
                 song.practiceList_[index].bpm_         = model_->getBpm();
                 song.practiceList_[index].accent_      = model_->getAccent();
                 song.practiceList_[index].beatPositionMSec_ = model_->getBeatPositionMSec();

                 song.practiceList_[index].speedMode_     = model_->getSpeedMode();
                 song.practiceList_[index].speed_         = model_->getSpeed();
                 song.practiceList_[index].speedIncStart_ = model_->getSpeedIncStart();
                 song.practiceList_[index].speedIncValue_ = model_->getSpeedIncValue();
                 song.practiceList_[index].speedIncPer_   = model_->getSpeedIncPer();
                 song.practiceList_[index].speedIncGoal_  = model_->getSpeedIncGoal();
                 
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
    if (reader == nullptr)
    {
        for (auto l : listeners_) l->fileLoadStatusChanged(kFileLoadStatus_Failed, fileToload_.getFullPathName());
        return;
    }
    
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
            model_->setPitch(song.pitch_);
            model_->setOutputMode(song.outputMode_);
            model_->setMusicVolume(song.musicVolume_);
            model_->setMetronomeVolume(song.metronomeVolume_);
            model_->setMusicMetronomeBalance(song.volumeBalance_);
            model_->setMetronomeSwitch(song.metronomeSw_);
            model_->setBpm(song.bpm_);
            model_->setAccent(song.accent_);
            model_->setBeatPositionMSec(song.beatPositionMSec_);
        }
    }
    if (!found)
    {
        model_->setPitch(0);
        model_->setOutputMode(kOutputMode_LR);
        model_->setMusicVolume(1.f);
        model_->setMetronomeVolume(1.f);
        model_->setMusicMetronomeBalance(0.5f);
        model_->setMetronomeSwitch(false);
        model_->setBpm(120);
        model_->setAccent(4);
        model_->setBeatPositionMSec(0.f);
    }
    model_->setLengthMSec(lengthInSamples / reader->sampleRate * 1000.f);
    model_->setLoopPosRatio(0.f, 1.f);
    model_->setPlayingPosRatio(0.f);
    
    addToHistory(currentSongFilePath_);
    saveSongState();
    
    if (functionToCallAfterFileLoad_ != nullptr) functionToCallAfterFileLoad_();

    if (wasPlaying_) model_->setPlaybackStatus(kPlaybackStatus_Playing);
    
    saveSongState();
    
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
