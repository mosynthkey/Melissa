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
    
    commands_["StartStop"] = [&](float value)
    {
        if (value == 1.f) model_->togglePlaybackStatus();
    };
    commands_["Back"] = [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosRatio(0.f);
    };
    commands_["PlaybackPositionValue"] = [&](float value)
    {
        model_->setPlayingPosRatio(value);
    };
    commands_["PlaybackPosition_Plus1Sec"] = [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() + 1000);
    };
    commands_["PlaybackPosition_Minus1Sec"] = [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() - 1000);
    };
    commands_["PlaybackPosition_Plus5Sec"] = [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() + 5000);
    };
    commands_["PlaybackPosition_Minus5Sec"] = [&](float value)
    {
        if (value == 1.f) model_->setPlayingPosMSec(model_->getPlayingPosMSec() - 5000);
    };
    commands_["PitchValue"] = [&](float value)
    {
        model_->setPitch(static_cast<int>(value * (kPitchMax - kPitchMin) + kPitchMin));
    };
    commands_["Pitch_Plus"] = [&](float value)
    {
        if (value == 1.f) model_->setPitch(model_->getPitch() + 1);
    };
    commands_["Pitch_Minus"] = [&](float value)
    {
        if (value == 1.f) model_->setPitch(model_->getPitch() - 1);
    };

    // Loop
    commands_["ResetLoop"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopPosRatio(0.f, 1.f);
    };
    commands_["ResetLoopStart"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosRatio(0.f);
    };
    commands_["ResetLoopEnd"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopBPosRatio(1.f);
    };
    commands_["SetLoopStart"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosRatio(model_->getPlayingPosRatio());
    };
    commands_["SetLoopEnd"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopBPosRatio(model_->getPlayingPosRatio());
    };
    commands_["SetLoopStartValue"] = [&](float value)
    {
        model_->setLoopAPosRatio(value);
    };
    commands_["SetLoopEndValue"] = [&](float value)
    {
        model_->setLoopBPosRatio(value);
    };
    commands_["SetLoopStart_Plus100MSec"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() + 100);
    };
    commands_["SetLoopEnd_Minus100MSec"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() - 100);
    };
    commands_["SetLoopStart_Plus1Sec"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() + 1000);
    };
    commands_["SetLoopEnd_Minus1Sec"] = [&](float value)
    {
        if (value == 1.f) model_->setLoopAPosMSec(model_->getLoopAPosMSec() - 1000);
    };

    // Speed
    commands_["SetSpeedValue"] = [&](float value)
    {
        model_->setSpeed(value * (kSpeedMax - kSpeedMin) + kSpeedMin);
    };
    commands_["SetSpeed_Plus5"] = [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() + 5);
    };
    commands_["SetSpeed_Minus5"] = [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() - 5);
    };
    commands_["SetSpeed_Plus1"] = [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() + 1);
    };
    commands_["SetSpeed_Minus1"] = [&](float value)
    {
        if (value == 1.f) model_->setSpeed(model_->getSpeed() - 1);
    };
    commands_["ResetSpeed"] = [&](float value)
    {
        if (value == 1.f) model_->setSpeed(100);
    };
    
    for (int speed : speedPresets)
    {
        if (speed == 100) continue;
        String name = "SetSpeedPreset_" + String(speed);
        commands_[name] = [&, speed](float value)
        {
            if (value == 1.f) model_->setSpeed(speed);
        };
    }

    // Metronome
    commands_["ToggleMetronome"] = [&](float value)
    {
        if (value == 1.f) model_->setMetronomeSwitch(!model_->getMetronomeSwitch());
    };
    commands_["SetAccentPosition"] = [&](float value)
    {
        if (value == 1.f) model_->setBeatPositionMSec(model_->getPlayingPosRatio());
    };

    // EQ
    commands_["ToggleEq"] = [&](float value)
    {
        if (value == 1.f) model_->setEqSwitch(!model_->getEqSwitch());
    };
    commands_["SetEqFreqValue"] = [&](float value)
    {
        model_->setEqFreq(0, 20 * std::pow(1000, value));
    };
    commands_["SetEqGainValue"] = [&](float value)
    {
        model_->setEqGain(0, value * (kEqGainMax - kEqGainMin) + kEqGainMin);
    };
    commands_["SetEqQValue"] = [&](float value)
    {
        model_->setEqQ(0, value * (kEqQMax - kEqQMin) + kEqQMin);
    };

    // Output
    commands_["SetMusicVolumeValue"] = [&](float value)
    {
        model_->setMusicVolume(2.f * value);
    };
    commands_["SetVolumeBalanceValue"] = [&](float value)
    {
        model_->setMusicMetronomeBalance(value);
    };
    commands_["SetMetronomeVolumeValue"] = [&](float value)
    {
        model_->setMetronomeVolume(value);
    };

    // Practice list
    commands_["AddPracticeList"] = [&](float value)
    {
        if (value == 1.f)
        {
            const auto name = MelissaUtility::getFormattedTimeSec(model_->getLoopAPosMSec() / 1000.f) + " - " + MelissaUtility::getFormattedTimeSec(model_->getLoopBPosMSec() / 1000.f);
            dataSource_->addPracticeList(name);
        }
    };
    
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
    commands_["SelectPracticeList_0"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(0);
    };
    commands_["SelectPracticeList_1"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(1);
    };
    commands_["SelectPracticeList_2"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(2);
    };
    commands_["SelectPracticeList_3"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(3);
    };
    commands_["SelectPracticeList_4"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(4);
    };
    commands_["SelectPracticeList_5"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(5);
    };
    commands_["SelectPracticeList_6"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(6);
    };
    commands_["SelectPracticeList_7"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(7);
    };
    commands_["SelectPracticeList_8"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(8);
    };
    commands_["SelectPracticeList_9"] = [&](float value)
    {
        if (value == 1.f) selectPracticeList(9);
    };

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
    commands_["AddMarker"] = [&](float value)
    {
        if (value == 1.f) dataSource_->addDefaultMarker(model_->getPlayingPosRatio());
    };
    commands_["SelectMarker_0"] = [&](float value)
    {
        if (value == 1.f) selectMarker(0);
    };
    commands_["SelectMarker_1"] = [&](float value)
    {
        if (value == 1.f) selectMarker(1);
    };
    commands_["SelectMarker_2"] = [&](float value)
    {
        if (value == 1.f) selectMarker(2);
    };
    commands_["SelectMarker_3"] = [&](float value)
    {
        if (value == 1.f) selectMarker(3);
    };
    commands_["SelectMarker_4"] = [&](float value)
    {
        if (value == 1.f) selectMarker(4);
    };
    commands_["SelectMarker_5"] = [&](float value)
    {
        if (value == 1.f) selectMarker(5);
    };
    commands_["SelectMarker_6"] = [&](float value)
    {
        if (value == 1.f) selectMarker(6);
    };
    commands_["SelectMarker_7"] = [&](float value)
    {
        if (value == 1.f) selectMarker(7);
    };
    commands_["SelectMarker_8"] = [&](float value)
    {
        if (value == 1.f) selectMarker(8);
    };
    commands_["SelectMarker_9"] = [&](float value)
    {
        if (value == 1.f) selectMarker(9);
    };
}

void MelissaCommand::excuteCommand(const String& commandAsString, float value)
{
    if (commands_.find(commandAsString) == commands_.end()) return;
    MessageManager::callAsync([&, commandAsString,  value]() { commands_[commandAsString](value); });
}

String MelissaCommand::getCommandDescription(const String& commandAsString)
{
    if (commandAsString.startsWith("SetSpeedPreset_"))
    {
        return (TRANS("SetSpeedPreset") + commandAsString.substring(15) + " %");
    }
    else
    {
        return TRANS(commandAsString);
    }
}

