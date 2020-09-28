//
//  MelissaCommand.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaDefinitions.h"
#include "MelissaCommand.h"
#include "MelissaUtility.h"

MelissaCommand MelissaCommand::instance_;

MelissaCommand::MelissaCommand()
{
    dataSource_ = MelissaDataSource::getInstance();
    model_ = MelissaModel::getInstance();
    
    commands_["StartStop"] = { "Start/Stop", [&](float value)
    {
        if (value == 1.f) model_->togglePlaybackStatus();
    }};
    commands_["Back"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosRatio(0.f);
    }};
    commands_["PlaybackPositionValue"] = { TRANS(""), [&](float value)
    {
        model_->setPlayingPosRatio(value);
    }};
    commands_["PlaybackPosition_Plus1Sec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() + 1000);
    }};
    commands_["PlaybackPosition_Minus1Sec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() - 1000);
    }};
    commands_["PlaybackPosition_Plus5Sec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() + 5000);
    }};
    commands_["PlaybackPosition_Minus5Sec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() - 5000);
    }};
    commands_["PitchValue"] = { TRANS(""), [&](float value)
    {
        model_->setPitch(value * (kPitchMax - kPitchMin) + kPitchMin);
    }};
    commands_["Pitch_Plus"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPitch(model_->getPitch() + 1);
    }};
    commands_["Pitch_Minus"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setPitch(model_->getPitch() - 1);
    }};

    // Loop
    commands_["ResetLoop"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopPosRatio(0.f, 1.f);
    }};
    commands_["ResetLoopStart"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosRatio(0.f);
    }};
    commands_["ResetLoopEnd"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopBPosRatio(1.f);
    }};
    commands_["SetLoopStart"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosRatio(model_->getPlayingPosRatio());
    }};
    commands_["SetLoopEnd"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopBPosRatio(model_->getPlayingPosRatio());
    }};
    commands_["SetLoopStartValue"] = { TRANS(""), [&](float value)
    {
        model_->setLoopAPosRatio(value);
    }};
    commands_["SetLoopEndValue"] = { TRANS(""), [&](float value)
    {
        model_->setLoopBPosRatio(value);
    }};
    commands_["SetLoopStart_Plus100MSec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() + 100);
    }};
    commands_["SetLoopEnd_Minus100MSec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() - 100);
    }};
    commands_["SetLoopStart_Plus1Sec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() + 1000);
    }};
    commands_["SetLoopEnd_Minus1Sec"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() - 1000);
    }};

    // Speed
    commands_["SetSpeedValue"] = { TRANS(""), [&](float value)
    {
        model_->setSpeed(value * (kSpeedMax - kSpeedMin) + kSpeedMin);
    }};
    commands_["SetSpeed_Plus5"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() + 5);
    }};
    commands_["SetSpeed_Minus5"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() - 5);
    }};
    commands_["SetSpeed_Plus1"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() + 1);
    }};
    commands_["SetSpeed_Minus1"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() - 1);
    }};
    commands_["ResetSpeed"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setSpeed(100);
    }};

    // Metronome
    commands_["ToggleMetronome"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setMetronomeSwitch(!model_->getMetronomeSwitch());
    }};
    commands_["SetAccentPosition"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setBeatPositionMSec(model_->getPlayingPosRatio());
    }};

    // EQ
    commands_["ToggleEq"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setEqSwitch(!model_->getEqSwitch());
    }};
    commands_["SetEqFreqValue"] = { TRANS(""), [&](float value)
    {
        model_->setEqFreq(0, 20 * std::pow(1000, value));
    }};
    commands_["SetEqGainValue"] = { TRANS(""), [&](float value)
    {
        model_->setEqGain(0, value * (kEqGainMax - kEqGainMin) + kEqGainMin);
    }};
    commands_["SetEqQValue"] = { TRANS(""), [&](float value)
    {
        model_->setEqQ(0, value * (kEqQMax - kEqQMin) + kEqQMin);
    }};

    // Output
    commands_["SetMusicVolumeValue"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) model_->setMusicVolume(2.f * value);
    }};
    commands_["SetVolumeBalanceValue"] = { TRANS(""), [&](float value)
    {
        model_->setMusicMetronomeBalance(value);
    }};
    commands_["SetMetronomeVolumeValue"] = { TRANS(""), [&](float value)
    {
        model_->setMetronomeVolume(value);
    }};

    // Practice list
    commands_["AddPracticeList"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f)
        {
            const auto name = MelissaUtility::getFormattedTimeSec(model_->getLoopAPosMSec() / 1000.f) + " - " + MelissaUtility::getFormattedTimeSec(model_->getLoopBPosMSec() / 1000.f);
            dataSource_->addPracticeList(name);
        }
    }};
    
    auto selectPracticeList = [](size_t index)
    {
        auto dataSource = MelissaDataSource::getInstance();
        auto model = MelissaModel::getInstance();
        
        std::vector<MelissaDataSource::Song::PracticeList> list;
        dataSource->getPracticeList(list);
        if (list.size() <= index) return;
        
        model->setLoopPosRatio(list[index].aRatio_, list[index].bRatio_);
        model->setSpeed(list[index].speed_);
    };
    commands_["SelectPracticeList_0"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(0);
    }};
    commands_["SelectPracticeList_1"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(1);
    }};
    commands_["SelectPracticeList_2"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(2);
    }};
    commands_["SelectPracticeList_3"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(3);
    }};
    commands_["SelectPracticeList_4"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(4);
    }};
    commands_["SelectPracticeList_5"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(5);
    }};
    commands_["SelectPracticeList_6"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(6);
    }};
    commands_["SelectPracticeList_7"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(7);
    }};
    commands_["SelectPracticeList_8"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(8);
    }};
    commands_["SelectPracticeList_9"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectPracticeList(9);
    }};

    // Marker
    auto selectMarker = [&](size_t index)
    {
        auto dataSource = MelissaDataSource::getInstance();
        auto model = MelissaModel::getInstance();
        
        std::vector<MelissaDataSource::Song::Marker> markers;
        dataSource->getMarkers(markers);
        if (markers.size() <= index) return;
        
        model->setPlayingPosRatio(markers[index].position_);
    };
    commands_["AddMarker"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) dataSource_->addDefaultMarker(model_->getPlayingPosRatio());
    }};
    commands_["SelectMarker_0"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(0);
    }};
    commands_["SelectMarker_1"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(1);
    }};
    commands_["SelectMarker_2"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(2);
    }};
    commands_["SelectMarker_3"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(3);
    }};
    commands_["SelectMarker_4"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(4);
    }};
    commands_["SelectMarker_5"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(5);
    }};
    commands_["SelectMarker_6"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(6);
    }};
    commands_["SelectMarker_7"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(7);
    }};
    commands_["SelectMarker_8"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(8);
    }};
    commands_["SelectMarker_9"] = { TRANS(""), [&](float value)
    {
        if (value == 1.f) selectMarker(9);
    }};
}

void MelissaCommand::excuteCommand(const String& commandAsString, float value)
{
    if (commands_.find(commandAsString) == commands_.end()) return;
    MessageManager::callAsync([&, commandAsString,  value]() { (commands_[commandAsString].second)(value); });
}

String MelissaCommand::getFunctionDescription(const String& commandAsString)
{
    if (commands_.find(commandAsString) == commands_.end()) return "Unknown";
    return commands_[commandAsString].first;
}

