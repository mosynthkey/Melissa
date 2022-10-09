//
//  MelissaDataSource.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "AppConfig.h"
#include "MelissaDataSource.h"
#include "MelissaStemProvider.h"
#include "MelissaUISettings.h"

enum
{
    kMaxSizeOfHistoryList = 40,
};

MelissaDataSource MelissaDataSource::instance_;

MelissaDataSource::MelissaDataSource() :
model_(MelissaModel::getInstance()),
sampleRate_(0.f),
currentSongFilePath_(""),
wasPlaying_(false)
{
    // Default shortcuts
    defaultShortcut_["spacebar"] = "StartStop";
    defaultShortcut_[","] = "Back";
    defaultShortcut_["A"] = "SetLoopStart";
    defaultShortcut_["B"] = "SetLoopEnd";
    defaultShortcut_["M"] = "AddMarker";
    defaultShortcut_["backspace"] = "ResetLoop";
    defaultShortcut_["cursor up"] = "SetSpeed_Plus1";
    defaultShortcut_["cursor down"] = "SetSpeed_Minus1";
    defaultShortcut_["cursor left"] = "PlaybackPosition_Minus1Sec";
    defaultShortcut_["cursor right"] = "PlaybackPosition_Plus1Sec";
}

MelissaDataSource::~MelissaDataSource()
{
}

void MelissaDataSource::removeListener(MelissaDataSourceListener* listener)
{
    for (size_t listenerIndex = 0; listenerIndex < listeners_.size(); ++listenerIndex)
    {
        if (listeners_[listenerIndex] == listener)
        {
            listeners_.erase(listeners_.begin() + listenerIndex);
            return;
        }
    }
}

void MelissaDataSource::loadSettingsFile(const File& file)
{
    settingsFile_ = file;
    
    if (!file.existsAsFile())
    {
        validateSettings();
        initFontSettings();
        return;
    }
    
    var settings = JSON::parse(file.loadFileAsString());
    settings = JSON::parse(settingsFile_.loadFileAsString());
    
    if (settings.hasProperty("global"))
    {
        auto g = settings["global"].getDynamicObject();
        if (g->hasProperty("version"))  global_.version_  = g->getProperty("version");
        if (g->hasProperty("root_dir")) global_.rootDir_  = g->getProperty("root_dir");
        if (g->hasProperty("width"))    global_.width_    = g->getProperty("width");
        if (g->hasProperty("height"))   global_.height_   = g->getProperty("height");
        if (g->hasProperty("device"))   global_.device_   = g->getProperty("device");
        if (g->hasProperty("playmode")) global_.playMode_ = g->getProperty("playmode");
        
        bool shortcutRegistered = false;
        if (g->hasProperty("shortcut"))
        {
            auto shortcutDynamicObject = g->getProperty("shortcut").getDynamicObject();
            if (shortcutDynamicObject != nullptr)
            {
                shortcutRegistered = true;
                for (auto&& shortcut : shortcutDynamicObject->getProperties())
                {
                    registerShortcut(shortcut.name.toString(), shortcut.value.toString());
                }
            }
        }
        if (!shortcutRegistered) setDefaultShortcuts();
        
        if (g->hasProperty("ui_theme")) global_.uiTheme_ = g->getProperty("ui_theme");
        initFontSettings(g->hasProperty("font_name") ? g->getProperty("font_name") : "");
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
        
        //if (p->hasProperty("metronome_sw"))  previous_.metronomeSw_      = p->getProperty("metronome_sw");
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
        
        if (p->hasProperty("eq_sw"))     previous_.eqSw_   = p->getProperty("eq_sw");
        if (p->hasProperty("eq_0_freq")) previous_.eqFreq_ = p->getProperty("eq_0_freq");
        if (p->hasProperty("eq_0_gain")) previous_.eqGain_ = p->getProperty("eq_0_gain");
        if (p->hasProperty("eq_0_q"))    previous_.eqQ_    = p->getProperty("eq_0_q");
        
        if (p->hasProperty("ui_state"))
        {
            auto uiState = p->getProperty("ui_state");
            if (uiState.hasProperty("browser_tab")) previous_.uiState_.selectedFileBrowserTab_ = uiState.getProperty("browser_tab", 0);
            if (uiState.hasProperty("playlist")) previous_.uiState_.selectedPlaylist_ = uiState.getProperty("playlist", 0);
        }
    }
    
    history_.clear();
    if (settings.hasProperty("history"))
    {
        auto array = settings["history"].getArray();
        if (array != nullptr)
        {
            for (auto history : *array)
            {
                File file(history);
                if (file.existsAsFile()) history_.add(history.toString());
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
                song.outputMode_       = static_cast<OutputMode>(outputMode);
                song.musicVolume_      = obj->getProperty("volume");
                song.metronomeVolume_  = obj->getProperty("metronome_volume");
                song.volumeBalance_    = obj->getProperty("volume_balance");
                song.metronomeSw_      = obj->getProperty("metronome_sw");
                song.bpm_              = obj->getProperty("bpm");
                song.accent_           = obj->getProperty("accent");
                song.beatPositionMSec_ = obj->getProperty("beat_position");
                song.eqSw_             = obj->getProperty("eq_sw");
                song.eqFreq_           = obj->getProperty("eq_0_freq");
                song.eqGain_           = obj->getProperty("eq_0_gain");
                song.eqQ_              = obj->getProperty("eq_0_q");
                song.memo_             = obj->getProperty("memo");
                for (auto l : *(obj->getProperty("list").getArray()))
                {
                    Song::PracticeList list;
                    list.name_       = l.getProperty("name", "");
                    list.aRatio_     = l.getProperty("a", 0.f);
                    list.bRatio_     = l.getProperty("b", 1.f);

#if !defined(SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST)
                    const int outputMode = l.getProperty("output_mode", kOutputMode_LR);
                    list.outputMode_ = static_cast<OutputMode>(outputMode);
                    list.musicVolume_     = l.getProperty("volume", 1.f);
                    list.metronomeVolume_ = l.getProperty("metronome_volume", 1.f);
                    list.volumeBalance_   = l.getProperty("volume_balance", 0.5f);
                    
                    list.metronomeSw_      = l.getProperty("metronome_sw", false);
                    list.bpm_              = l.getProperty("bpm", -1);
                    list.accent_           = l.getProperty("accent", 4);
                    list.beatPositionMSec_ = l.getProperty("beat_position", 0.f);
#endif
                    
                    list.speed_         = l.getProperty("speed", 100);
#if defined(ENABLE_SPEED_TRAINING)
                    const int speedMode = l.getProperty("speed_mode", kSpeedMode_Basic);
                    list.speedMode_     = static_cast<SpeedMode>(speedMode);
                    list.speedIncStart_ = l.getProperty("speed_inc_start", 70);
                    list.speedIncValue_ = l.getProperty("speed_inc_value", 1);
                    list.speedIncPer_   = l.getProperty("speed_inc_per", 10);
                    list.speedIncGoal_  = l.getProperty("speed_inc_goal", 100);
#endif
                    
                    song.practiceList_.emplace_back(list);
                }
                if (obj->hasProperty("marker"))
                {
                    for (auto m : *(obj->getProperty("marker").getArray()))
                    {
                        Song::Marker marker;
                        marker.position_ = m.getProperty("position", 0.f);
                        marker.colourR_  = m.getProperty("r", 0);
                        marker.colourG_  = m.getProperty("g", 0);
                        marker.colourB_  = m.getProperty("b", 0);
                        marker.memo_     = m.getProperty("memo", "");
                        song.markers_.emplace_back(marker);
                    }
                    std::sort(song.markers_.begin(), song.markers_.end(), [](auto const& lhs, auto const& rhs) { return lhs.position_ < rhs.position_; });
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
    for (auto&& l : listeners_) l->playlistUpdated(0);
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
    global->setProperty("playmode", global_.playMode_);
    auto shortcut = new DynamicObject();
    {
        for (auto&& s : global_.shortcut_)
        {
            shortcut->setProperty(s.first, s.second);
        }
    }
    global->setProperty("shortcut", shortcut);
    global->setProperty("ui_theme", global_.uiTheme_);
    global->setProperty("font_name", global_.fontName_);
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
    
    previous->setProperty("speed",           model_->getSpeed());
    
#if defined(ENABLE_SPEED_TRAINING)
    previous->setProperty("speed_mode",      model_->getSpeedMode());
    previous->setProperty("speed_inc_start", model_->getSpeedIncStart());
    previous->setProperty("speed_inc_value", model_->getSpeedIncValue());
    previous->setProperty("speed_inc_per",   model_->getSpeedIncPer());
    previous->setProperty("speed_inc_goal",  model_->getSpeedIncGoal());
#endif
    
    previous->setProperty("eq_sw",     model_->getEqSwitch());
    previous->setProperty("eq_0_freq", model_->getEqFreq(0));
    previous->setProperty("eq_0_gain", model_->getEqGain(0));
    previous->setProperty("eq_0_q",    model_->getEqQ(0));
    
    auto uiState = new DynamicObject();
    uiState->setProperty("browser_tab", previous_.uiState_.selectedFileBrowserTab_);
    uiState->setProperty("playlist",    previous_.uiState_.selectedPlaylist_);
    previous->setProperty("ui_state", uiState);
    
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
        obj->setProperty("volume",           song.musicVolume_);
        obj->setProperty("metronome_volume", song.metronomeVolume_);
        obj->setProperty("volume_balance",   song.volumeBalance_);
        obj->setProperty("metronome_sw",     song.metronomeSw_);
        obj->setProperty("bpm",              song.bpm_);
        obj->setProperty("accent",           song.accent_);
        obj->setProperty("beat_position",    song.beatPositionMSec_);
        obj->setProperty("eq_sw",            song.eqSw_);
        obj->setProperty("eq_0_freq",        song.eqFreq_);
        obj->setProperty("eq_0_gain",        song.eqGain_);
        obj->setProperty("eq_0_q",           song.eqQ_);
        obj->setProperty("memo",             song.memo_);
        
        Array<var> list;
        for (auto l : song.practiceList_)
        {
            auto obj = new DynamicObject();
            obj->setProperty("name",   l.name_);
            obj->setProperty("a",      l.aRatio_);
            obj->setProperty("b",      l.bRatio_);
            
#if !defined(SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST)
            obj->setProperty("output", l.outputMode_);
            obj->setProperty("volume", l.musicVolume_);
            obj->setProperty("metronome_volume", l.metronomeVolume_);
            obj->setProperty("volume_balance",   l.volumeBalance_);
            
            obj->setProperty("metronome_sw",  l.metronomeSw_);
            obj->setProperty("bpm",           l.bpm_);
            obj->setProperty("accent",        l.accent_);
            obj->setProperty("beat_position", l.beatPositionMSec_);
#endif
            obj->setProperty("speed",           l.speed_);
#if defined(ENABLE_SPEED_TRAINING)
            obj->setProperty("speed_mode",      l.speedMode_);
            obj->setProperty("speed_inc_start", l.speedIncStart_);
            obj->setProperty("speed_inc_value", l.speedIncValue_);
            obj->setProperty("speed_inc_per",   l.speedIncPer_);
            obj->setProperty("speed_inc_goal",  l.speedIncGoal_);
#endif
            
            list.add(obj);
        }
        obj->setProperty("list", list);
        
         Array<var> marker;
        for (auto&& m : song.markers_)
        {
            auto obj = new DynamicObject();
            obj->setProperty("position", m.position_);
            obj->setProperty("r", m.colourR_);
            obj->setProperty("g", m.colourG_);
            obj->setProperty("b", m.colourB_);
            obj->setProperty("memo", m.memo_);
            
            marker.add(obj);
        }
        obj->setProperty("marker", marker);
       
        songs.add(obj);
    }
    settings->setProperty("songs", songs);
    
    settingsFile_.replaceWithText(JSON::toString(settings));
}

void MelissaDataSource::initFontSettings(const String& fontName)
{
    StringArray fontCandidates;
    if (!fontName.isEmpty()) fontCandidates.add(fontName);
    
#if defined(JUCE_MAC)
    fontCandidates.mergeArray(StringArray { "YuGothic", "San Francisco" });
#elif defined(JUCE_WINDOWS)
    fontCandidates.mergeArray(StringArray { "Meiryo UI", "Tahoma" });
#else
    fontCandidates.mergeArray(StringArray { "DejaVu Sans", "IPAGothic", "Verdana", "Bitstream Vera Sans", "Luxi Sans", "Liberation Sans", "Sans" });
#endif
    
    for (auto&& font : fontCandidates)
    {
        if (isFontAvailable(font))
        {
            global_.fontName_ = font;
            break;
        }
    }
}

bool MelissaDataSource::isFontAvailable(const String& fontName) const
{
    const StringArray availableFontNames = Font::findAllTypefaceNames();
    return availableFontNames.contains(fontName);
}

Font MelissaDataSource::getFont(Global::FontSize size) const
{
    int fontSizeOffset = 0;
#ifdef JUCE_WINDOWS
    fontSizeOffset = 2;
#endif

    if (size == Global::kFontSize_Large)
    {
        return Font(global_.fontName_, 20 + fontSizeOffset, Font::plain);
    }
    else if (size == Global::kFontSize_Main)
    {
        return Font(global_.fontName_, 17 + fontSizeOffset, Font::plain);
    }
    else if (size == Global::kFontSize_Sub)
    {
        return Font(global_.fontName_, 15 + fontSizeOffset, Font::plain);
    }
    else
    {
        return Font(global_.fontName_, 13 + fontSizeOffset, Font::plain);
    }
}

String MelissaDataSource::getCompatibleFileExtensions()
{
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    return formatManager.getWildcardForAllFormats();
}

void MelissaDataSource::loadFileAsync(const File& file, std::function<void()> functionToCallAfterFileLoad)
{
    functionToCallAfterFileLoad_ = functionToCallAfterFileLoad;
    
    if (file.existsAsFile())
    {
        MelissaStemProvider::getInstance()->prepareForLoadStems(file, fileToload_, stemFiles_);
        
        wasPlaying_ = (model_->getPlaybackStatus() == kPlaybackStatus_Playing);
        model_->setPlaybackStatus(kPlaybackStatus_Stop);
        for (auto&& l : listeners_) l->fileLoadStatusChanged(kFileLoadStatus_Loading, file.getFullPathName());
        cancelPendingUpdate();
        triggerAsyncUpdate();
    }
    else
    {
        for (auto&& l : listeners_) l->fileLoadStatusChanged(kFileLoadStatus_Failed, file.getFullPathName());
    }
}

float MelissaDataSource::readBuffer(size_t ch, size_t index, StemType playPart)
{
    if (originalAudioSampleBuf_ == nullptr) return 0.f;
    
    const int numOfChs   = originalAudioSampleBuf_->getNumChannels();
    const int bufferSize = originalAudioSampleBuf_->getNumSamples();
    
    if (2 <= ch || numOfChs <= ch) ch = 0;
    if (bufferSize <= index) return 0.f;
    
    if (playPart == kStemType_All)
    {
        return originalAudioSampleBuf_->getSample(static_cast<int>(ch), static_cast<int>(index));
    }
    else if (playPart < kNumStemTypes)
    {
        if (stemAudioSampleBuf_[playPart] != nullptr && index < stemAudioSampleBuf_[playPart]->getNumSamples())
        {
            return stemAudioSampleBuf_[playPart]->getSample(static_cast<int>(ch), static_cast<int>(index));
        }
    }
    
    return 0.f;
}

void MelissaDataSource::disposeBuffer()
{
    if (originalAudioSampleBuf_ == nullptr) return;
    originalAudioSampleBuf_->clear();
    originalAudioSampleBuf_ = nullptr;
}

void MelissaDataSource::setDefaultShortcut(const String& eventName)
{
    if (defaultShortcut_.find(eventName) == defaultShortcut_.end())
    {
        deregisterShortcut(eventName);
    }
    else
    {
        global_.shortcut_[eventName] = defaultShortcut_[eventName];
        for (auto&& l : listeners_) l->shortcutUpdated();
    }
}

void MelissaDataSource::setDefaultShortcuts(bool removeAll)
{
    if (removeAll) global_.shortcut_.clear();
    
    for (auto&& shortcut : defaultShortcut_)
    {
        global_.shortcut_[shortcut.first] = shortcut.second;
    }
    
    for (auto&& l : listeners_) l->shortcutUpdated();
}

std::map<String, String> MelissaDataSource::getAllAssignedShortcuts() const
{
    return global_.shortcut_;
}

String MelissaDataSource::getAssignedShortcut(const String& eventName)
{
    if (global_.shortcut_.find(eventName) == global_.shortcut_.end()) return "";
    return global_.shortcut_[eventName];
}

void MelissaDataSource::registerShortcut(const String& eventName, const String& command)
{
    if (command.isEmpty())
    {
        deregisterShortcut(eventName);
        return;
    }
    
    global_.shortcut_[eventName] = command;
    
    for (auto&& l : listeners_) l->shortcutUpdated();
}

void MelissaDataSource::deregisterShortcut(const String& eventName)
{
    global_.shortcut_.erase(eventName);
    
    for (auto&& l : listeners_) l->shortcutUpdated();
}

String MelissaDataSource::getUITheme() const
{

    if (global_.uiTheme_ == "System_Auto")
    {
#ifdef JUCE_MAC
        return global_.uiTheme_;
#else
        return "System_Dark";
#endif
    }

    return global_.uiTheme_;
}

void MelissaDataSource::setUITheme(const String& uiTheme)
{
#ifdef JUCE_MAC
    if (uiTheme == "System_Auto")
    {
        global_.uiTheme_ = uiTheme;
    }
    else
#endif
    if (uiTheme == "System_Dark" || uiTheme == "System_Light")
    {
        global_.uiTheme_ = uiTheme;
    }
    else
    {
        global_.uiTheme_ = "System_Dark";
    }
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
        model_->setMetronomeSwitch(false /* previous_.metronomeSw_ */);
        model_->setBpm(previous_.bpm_);
        model_->setAccent(previous_.accent_);
        model_->setBeatPositionMSec(previous_.beatPositionMSec_);
        model_->setSpeed(previous_.speed_);
#if defined(ENABLE_SPEED_TRAINING)
        model_->setSpeedMode(previous_.speedMode_);
        model_->setSpeedIncStart(previous_.speedIncStart_);
        model_->setSpeedIncValue(previous_.speedIncValue_);
        model_->setSpeedIncPer(previous_.speedIncPer_);
        model_->setSpeedIncGoal(previous_.speedIncGoal_);
#endif
        model_->setEqSwitch(previous_.eqSw_);
        model_->setEqFreq(0, previous_.eqFreq_);
        model_->setEqGain(0, previous_.eqGain_);
        model_->setEqQ(0, previous_.eqQ_);
    });
}

void MelissaDataSource::removeFromHistory(size_t index)
{
    if (history_.size() <= index) return;
    history_.remove(static_cast<int>(index));
    for (auto&& l : listeners_) l->historyUpdated();
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
    for (auto&& l : listeners_) l->playlistUpdated(index);
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
    for (auto&& l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::addToPlaylist(size_t index, const String& filePath)
{
    if (playlists_.size() <= index) return;
    playlists_[index].list_.addIfNotAlreadyThere(filePath);
    for (auto&& l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::removeFromPlaylist(size_t playlistIndex, size_t index)
{
    if (playlists_.size() <= playlistIndex) return;
    if (playlists_[playlistIndex].list_.size() <= index) return;
    playlists_[playlistIndex].list_.remove(static_cast<int>(index));
    for (auto&& l : listeners_) l->playlistUpdated(index);
}

size_t MelissaDataSource::createPlaylist(const String& name)
{
    Playlist playlist;
    playlist.name_ = name;
    playlists_.emplace_back(playlist);
    const size_t index = playlists_.size() - 1;
    for (auto&& l : listeners_) l->playlistUpdated(index);
    return index;
}

void MelissaDataSource::removePlaylist(size_t index)
{
    const auto numOfPlaylists = playlists_.size();
    if (numOfPlaylists <= 1 || numOfPlaylists <= index) return;
    playlists_.erase(playlists_.begin() + index);
    for (auto&& l : listeners_) l->playlistUpdated(index);
}

void MelissaDataSource::saveSongState()
{
    if (currentSongFilePath_.isEmpty()) return;
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            song.pitch_            = model_->getPitch();
            song.outputMode_       = model_->getOutputMode();
            song.musicVolume_      = model_->getMusicVolume();
            song.metronomeVolume_  = model_->getMetronomeVolume();
            song.volumeBalance_    = model_->getMusicMetronomeBalance();
            song.metronomeSw_      = model_->getMetronomeSwitch();
            song.bpm_              = model_->getBpm();
            song.accent_           = model_->getAccent();
            song.beatPositionMSec_ = model_->getBeatPositionMSec();
            
            song.speed_         = model_->getSpeed();
#if defined(ENABLE_SPEED_TRAINING)
            song.speedMode_     = model_->getSpeedMode();
            song.speedIncStart_ = model_->getSpeedIncStart();
            song.speedIncValue_ = model_->getSpeedIncValue();
            song.speedIncPer_   = model_->getSpeedIncPer();
            song.speedIncGoal_  = model_->getSpeedIncGoal();
#endif
            
            song.eqSw_             = model_->getEqSwitch();
            song.eqFreq_           = model_->getEqFreq(0);
            song.eqGain_           = model_->getEqGain(0);
            song.eqQ_              = model_->getEqQ(0);
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

size_t MelissaDataSource::getNumPracticeList() const
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            return song.practiceList_.size();
        }
    }
    
    return 0;
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

#if !defined(SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST)
            plist.outputMode_      = model_->getOutputMode();
            plist.musicVolume_     = model_->getMusicVolume();
            plist.metronomeVolume_ = model_->getMetronomeVolume();
            plist.volumeBalance_   = model_->getMusicMetronomeBalance();
            
            plist.metronomeSw_      = model_->getMetronomeSwitch();
            plist.bpm_              = model_->getBpm();
            plist.accent_           = model_->getAccent();
            plist.beatPositionMSec_ = model_->getBeatPositionMSec();
#endif

            plist.speed_         = model_->getSpeed();
#if defined(ENABLE_SPEED_TRAINING)
            plist.speedMode_     = model_->getSpeedMode();
            plist.speedIncStart_ = model_->getSpeedIncStart();
            plist.speedIncValue_ = model_->getSpeedIncValue();
            plist.speedIncPer_   = model_->getSpeedIncPer();
            plist.speedIncGoal_  = model_->getSpeedIncGoal();
#endif
            
            song.practiceList_.emplace_back(plist);
            for (auto&& l : listeners_) l->practiceListUpdated();
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
                for (auto&& l : listeners_) l->practiceListUpdated();
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
                 
#if !defined(SAVE_ONLY_LOOP_AND_SPEED_IN_PRACTICE_LIST)
                 song.practiceList_[index].outputMode_      = model_->getOutputMode();
                 song.practiceList_[index].musicVolume_     = model_->getMusicVolume();
                 song.practiceList_[index].metronomeVolume_ = model_->getMetronomeVolume();
                 song.practiceList_[index].volumeBalance_   = model_->getMusicMetronomeBalance();
                 
                 song.practiceList_[index].metronomeSw_ = model_->getMetronomeSwitch();
                 song.practiceList_[index].bpm_         = model_->getBpm();
                 song.practiceList_[index].accent_      = model_->getAccent();
                 song.practiceList_[index].beatPositionMSec_ = model_->getBeatPositionMSec();
#endif
                song.practiceList_[index].speed_         = model_->getSpeed();

#if defined(ENABLE_SPEED_TRAINING)
                song.practiceList_[index].speedMode_     = model_->getSpeedMode();
                song.practiceList_[index].speedIncStart_ = model_->getSpeedIncStart();
                song.practiceList_[index].speedIncValue_ = model_->getSpeedIncValue();
                song.practiceList_[index].speedIncPer_   = model_->getSpeedIncPer();
                song.practiceList_[index].speedIncGoal_  = model_->getSpeedIncGoal();
#endif
                 
                for (auto&& l : listeners_) l->practiceListUpdated();
            }
            return;
        }
    }
}

void MelissaDataSource::overwritePracticeList(size_t index, const Song::PracticeList& list)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            if (index < song.practiceList_.size())
            {
                song.practiceList_[index] = list;
                for (auto&& l : listeners_) l->practiceListUpdated();
            }
            return;
        }
    }
}

void MelissaDataSource::swapPracticeList(size_t indexA, size_t indexB)
{
    const auto size = getNumPracticeList();
    if (size <= indexA || size <= indexB) return;
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            auto swap = song.practiceList_[indexA];
            song.practiceList_[indexA] = song.practiceList_[indexB];
            song.practiceList_[indexB] = swap;
            for (auto&& l : listeners_) l->practiceListUpdated();
            return;
        }
    }
}

void MelissaDataSource::getMarkers(std::vector<Song::Marker>& markers) const
{
    markers.clear();
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            markers = song.markers_;
            return;
        }
    }
}

void MelissaDataSource::addMarker(const Song::Marker& marker)
{
    if (currentSongFilePath_.isEmpty()) return;
    
    const int lengthSec = model_->getLengthMSec() / 1000.f;
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            for (auto&& m : song.markers_)
            {
                if (static_cast<int>(marker.position_ * lengthSec) == static_cast<int>(m.position_ * lengthSec))
                {
                    return;
                }
            }
        }
    }
    
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            song.markers_.emplace_back(marker);
            std::sort(song.markers_.begin(), song.markers_.end(), [](auto const& lhs, auto const& rhs) { return lhs.position_ < rhs.position_; });
            for (auto&& l : listeners_) l->markerUpdated();
            return;
        }
    }
}

void MelissaDataSource::addDefaultMarker(float position)
{
    Song::Marker marker;
    
    marker.position_ = position;
    Colour colour = Colour::fromRGB(255, 160, 160);
    colour = colour.withHue(marker.position_);
    marker.colourR_  = colour.getRed();
    marker.colourG_  = colour.getGreen();
    marker.colourB_  = colour.getBlue();
    marker.memo_     = "";
    addMarker(marker);
}

void MelissaDataSource::removeMarker(size_t index)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            if (index < song.markers_.size())
            {
                song.markers_.erase(song.markers_.begin() + index);
                for (auto&& l : listeners_) l->markerUpdated();
                return;
            }
        }
    }
}

void MelissaDataSource::overwriteMarker(size_t index, const Song::Marker& marker)
{
    for (auto&& song : songs_)
    {
        if (song.filePath_ == currentSongFilePath_)
        {
            if (index < song.markers_.size())
            {
                song.markers_[index] = marker;
                for (auto&& l : listeners_) l->markerUpdated();
                return;
            }
        }
    }
}

void MelissaDataSource::handleAsyncUpdate()
{
    // load file asynchronously
    
    saveSongState();
    
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    File originalFile;
    std::map<std::string, File> stemFiles;
    
    auto* reader = formatManager.createReaderFor(fileToload_);
    if (reader == nullptr)
    {
        for (auto&& l : listeners_) l->fileLoadStatusChanged(kFileLoadStatus_Failed, fileToload_.getFullPathName());
        return;
    }
    
    // read audio data from reader
    
    // original file
    const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
    originalAudioSampleBuf_ = std::make_unique<AudioSampleBuffer>(2, lengthInSamples);
    reader->read(originalAudioSampleBuf_.get(), 0, lengthInSamples, 0, true, true);
    sampleRate_ = reader->sampleRate;
    
    // stem files
    if (stemFiles_.size() == kNumStemTypes)
    {
        for (int stemTypeIndex = 0; stemTypeIndex < kNumStemTypes; ++stemTypeIndex)
        {
            const auto stemName = MelissaStemProvider::partNames_[stemTypeIndex];
            auto* reader = formatManager.createReaderFor(stemFiles_[stemName]);
            if (reader == nullptr)
            {
                stemFiles_.clear();
                MelissaStemProvider::getInstance()->failedToReadPreparedStems();
                break;
            }
            
            const int lengthInSamples = static_cast<int>(reader->lengthInSamples);
            stemAudioSampleBuf_[stemTypeIndex] = std::make_unique<AudioSampleBuffer>(2, lengthInSamples);
            reader->read(stemAudioSampleBuf_[stemTypeIndex].get(), 0, lengthInSamples, 0, true, true);
            const auto sampleRate = reader->sampleRate;
            DBG(String(stemName) + String(".lengthInSamples : ") + String(lengthInSamples) + String(" / sampleRate : ") + String(sampleRate));
        }
    }
    
    currentSongFilePath_ = fileToload_.getFullPathName();
    audioEngine_->updateBuffer();
    
    for (auto&& l : listeners_)
    {
        l->fileLoadStatusChanged(kFileLoadStatus_Success, currentSongFilePath_);
        l->songChanged(currentSongFilePath_, lengthInSamples, sampleRate_);
        l->markerUpdated();
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
            model_->setSpeed(song.speed_);
            
#if defined(ENABLE_SPEED_TRAINING)
            model_->setSpeedMode(song.speedMode_);
            model_->setSpeedIncStart(song.speedIncStart_);
            model_->setSpeedIncValue(song.speedIncValue_);
            model_->setSpeedIncPer(song.speedIncPer_);
            model_->setSpeedIncGoal(song.speedIncGoal_);
#endif
            
            model_->setEqSwitch(song.eqSw_);
            model_->setEqFreq(0, song.eqFreq_);
            model_->setEqGain(0, song.eqGain_);
            model_->setEqQ(0, song.eqQ_);
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
        model_->setBpm(kBpmShouldMeasure);
        model_->setAccent(4);
        model_->setBeatPositionMSec(0.f);
        
        model_->setSpeed(100);
#if defined(ENABLE_SPEED_TRAINING)
        model_->setSpeedMode(kSpeedMode_Basic);
        model_->setSpeedIncStart(75);
        model_->setSpeedIncValue(1);
        model_->setSpeedIncPer(10);
        model_->setSpeedIncGoal(100);
#endif
        
        model_->setEqSwitch(false);
        model_->setEqFreq(0, 500);
        model_->setEqGain(0, 0.f);
        model_->setEqQ(0, 7.f);
    }
    model_->setLengthMSec(lengthInSamples / reader->sampleRate * 1000.f);
    model_->setLoopPosRatio(0.f, 1.f);
    model_->setPlayingPosRatio(0.f);
    model_->setPlayPart(kStemType_All);
    
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
    
    for (auto&& l : listeners_) l->historyUpdated();
}
