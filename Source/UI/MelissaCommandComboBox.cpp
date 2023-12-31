//
//  MelissaCommandComboBox.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaCommand.h"
#include "MelissaCommandComboBox.h"
#include "MelissaDataSource.h"
#include "MelissaUISettings.h"

using namespace juce;

enum CommandCategory
{
    kCommandCategory_Transport,
    kCommandCategory_Pitch,
    kCommandCategory_Loop,
    kCommandCategory_Speed,
    kCommandCategory_Metronome,
    kCommandCategory_EQ,
    kCommandCategory_Mixer,
    kCommandCategory_PracticeList,
    kCommandCategory_Marker,
#ifdef MELISSA_FULL_VERSION
    kCommandCategory_Part,
    kCommandCategory_PartDetail,
#endif
    kNumOfCommandCategories
};

enum CommandType
{
    kCommandType_Switch,
    kCommandType_Value,
};

typedef std::pair<String, CommandType> CommandAndType;
typedef std::pair<String, std::vector<CommandAndType>> TitleAndCommandList;
static const TitleAndCommandList commandList[kNumOfCommandCategories] =
{
    {
        "Transport",
        {
            { "Start", kCommandType_Switch },
            { "Stop", kCommandType_Switch },
            { "StartStop", kCommandType_Switch },
            { "Back", kCommandType_Switch },
            { "Next", kCommandType_Switch },
            { "PlaybackPositionValue", kCommandType_Value },
            { "PlaybackPosition_Plus1Sec", kCommandType_Switch },
            { "PlaybackPosition_Minus1Sec", kCommandType_Switch },
            { "PlaybackPosition_Plus5Sec", kCommandType_Switch },
            { "PlaybackPosition_Minus5Sec", kCommandType_Switch },
        }
    },
    {
        "Pitch",
        {
            { "Pitch_Reset",    kCommandType_Switch },
            { "PitchValue",     kCommandType_Value },
            { "Pitch_Plus",     kCommandType_Switch },
            { "Pitch_Minus",    kCommandType_Switch },
            { "Pitch_1OctUp",   kCommandType_Switch },
            { "Pitch_1OctDown", kCommandType_Switch },
            { "Pitch_2OctUp",   kCommandType_Switch },
            { "Pitch_2OctDown", kCommandType_Switch },
        }
    },
    {
        "Loop",
        {
            { "ResetLoop", kCommandType_Switch },
            { "ResetLoopStart", kCommandType_Switch },
            { "ResetLoopEnd", kCommandType_Switch },
            { "SetLoopStart", kCommandType_Switch },
            { "SetLoopEnd", kCommandType_Switch },
            { "SetLoopStartValue", kCommandType_Value },
            { "SetLoopEndValue", kCommandType_Value },
            { "SetLoopStart_Plus100MSec", kCommandType_Switch },
            { "SetLoopEnd_Minus100MSec", kCommandType_Switch },
            { "SetLoopStart_Plus1Sec", kCommandType_Switch },
            { "SetLoopEnd_Minus1Sec", kCommandType_Switch },
        }
    },
    {
        "Speed",
        {
            { "SetSpeedValue", kCommandType_Value },
            { "SetSpeed_Plus5", kCommandType_Switch },
            { "SetSpeed_Minus5", kCommandType_Switch },
            { "SetSpeed_Plus1", kCommandType_Switch },
            { "SetSpeed_Minus1", kCommandType_Switch },
            { "ResetSpeed", kCommandType_Switch },
            
            { "SetSpeedPreset_40", kCommandType_Switch },
            { "SetSpeedPreset_50", kCommandType_Switch },
            { "SetSpeedPreset_60", kCommandType_Switch },
            { "SetSpeedPreset_70", kCommandType_Switch },
            { "SetSpeedPreset_75", kCommandType_Switch },
            { "SetSpeedPreset_80", kCommandType_Switch },
            { "SetSpeedPreset_85", kCommandType_Switch },
            { "SetSpeedPreset_90", kCommandType_Switch },
            { "SetSpeedPreset_95", kCommandType_Switch },
            { "SetSpeedPreset_105", kCommandType_Switch },
            { "SetSpeedPreset_150", kCommandType_Switch },
            { "SetSpeedPreset_200", kCommandType_Switch },
        }
    },
    {
        "Metronome",
        {
            { "ToggleMetronome", kCommandType_Switch },
            { "SetAccentPosition", kCommandType_Switch },
        }
    },
    {
        "EQ",
        {
            { "ToggleEq", kCommandType_Switch },
            { "SetEqFreqValue", kCommandType_Value },
            { "SetEqGainValue", kCommandType_Value },
            { "SetEqQValue", kCommandType_Value },
        }
    },
    {
        "Mixer",
        {
            { "SetMusicVolumeValue", kCommandType_Value },
            { "SetVolumeBalanceValue", kCommandType_Value },
            { "SetMetronomeVolumeValue", kCommandType_Value },
        }
    },
    {
        "PracticeList",
        {
            { "AddPracticeList", kCommandType_Switch },
            { "SelectPracticeList_0", kCommandType_Switch },
            { "SelectPracticeList_1", kCommandType_Switch },
            { "SelectPracticeList_2", kCommandType_Switch },
            { "SelectPracticeList_3", kCommandType_Switch },
            { "SelectPracticeList_4", kCommandType_Switch },
            { "SelectPracticeList_5", kCommandType_Switch },
            { "SelectPracticeList_6", kCommandType_Switch },
            { "SelectPracticeList_7", kCommandType_Switch },
            { "SelectPracticeList_8", kCommandType_Switch },
            { "SelectPracticeList_9", kCommandType_Switch },
        }
    },
    {
        "Marker",
        {
            { "AddMarker", kCommandType_Switch },
            { "SelectMarker_0", kCommandType_Switch },
            { "SelectMarker_1", kCommandType_Switch },
            { "SelectMarker_2", kCommandType_Switch },
            { "SelectMarker_3", kCommandType_Switch },
            { "SelectMarker_4", kCommandType_Switch },
            { "SelectMarker_5", kCommandType_Switch },
            { "SelectMarker_6", kCommandType_Switch },
            { "SelectMarker_7", kCommandType_Switch },
            { "SelectMarker_8", kCommandType_Switch },
            { "SelectMarker_9", kCommandType_Switch },
        }
    },
#ifdef MELISSA_FULL_VERSION
    {
        "Part",
        {
            { "Part_All",   kCommandType_Switch },
            { "Part_Inst",  kCommandType_Switch },
            { "Part_Vocal", kCommandType_Switch },
            { "Part_Piano", kCommandType_Switch },
            { "Part_Bass",  kCommandType_Switch },
            { "Part_Drums", kCommandType_Switch },
            { "Part_Others", kCommandType_Switch },
        }
    },
    {
        "Part_Detail",
        {
            { "Part_Vocal_Volume",  kCommandType_Value },
            { "Part_Piano_Volume",  kCommandType_Value },
            { "Part_Bass_Volume",   kCommandType_Value },
            { "Part_Drums_Volume",  kCommandType_Value },
            { "Part_Others_Volume", kCommandType_Value },
        }
    }
#endif
};

MelissaCommandComboBox::MelissaCommandComboBox() : onSelectedCommandChanged_(nullptr), noAssignMenuId_(1), selectedCommand_("")
{
    setWantsKeyboardFocus(false);
    
    popupMenu_.setLookAndFeel(&laf_);
    
    noAssignMenuId_ = 0;
    for (auto&& titleAndCommandList : commandList)
    {
        noAssignMenuId_ += titleAndCommandList.second.size();
    }
    ++noAssignMenuId_;
}

MelissaCommandComboBox::~MelissaCommandComboBox()
{
    popupMenu_.setLookAndFeel(nullptr);
}

void MelissaCommandComboBox::select(const String& command)
{
    if (command.isEmpty())
    {
        //setSelectedId(noAssignMenuId_);
        setText(TRANS("NoAssign"));
        return;
    }
    
    int commandItemId = 0;
    for (auto&& titleAndCommandList : commandList)
    {
        for (auto&& c : titleAndCommandList.second)
        {
            ++commandItemId;
            if (c.first == command)
            {
                //setSelectedId(commandItemId);
                selectedCommand_ = getSelectedCommandWithItemId(commandItemId);
                setText(MelissaCommand::getInstance()->getCommandDescription(selectedCommand_));
                return;
            }
        }
    }
}

void MelissaCommandComboBox::showPopup()
{
    auto option = PopupMenu::Options().withTargetComponent(this);
    
    const bool isSwitch = !(shortcut_.startsWith("CC #"));
    
    // build popup menu
    popupMenu_.clear();
    int itemId = 0;
    for (auto&& titleAndCommandList : commandList)
    {
        PopupMenu subMenu;
        
        for (auto&& command : titleAndCommandList.second)
        {
            String menu = MelissaCommand::getInstance()->getCommandDescription(command.first);
            if (command.second == kCommandType_Value) menu += " [CC]";
            subMenu.addItem(++itemId, menu, isSwitch ? (command.second == kCommandType_Switch) : true);
        }
        popupMenu_.addSubMenu(TRANS(titleAndCommandList.first), subMenu);
    }
    popupMenu_.addItem(++itemId, TRANS("NoAssign"));
    
    // show popup menu
    popupMenu_.showMenuAsync(option, [&](int itemId)
    {
        if (itemId != 0) {
            selectedCommand_ = getSelectedCommandWithItemId(itemId);
            select(selectedCommand_);
            
            if (onSelectedCommandChanged_ != nullptr) onSelectedCommandChanged_(selectedCommand_);
        }
        
        hidePopup();
    });
}

String MelissaCommandComboBox::getSelectedCommandWithItemId(int itemId) const
{
    int commandItemId = 0;
    for (auto&& titleAndCommandList : commandList)
    {
        for (auto&& command : titleAndCommandList.second)
        {
            ++commandItemId;
            if (commandItemId == itemId) return command.first;
        }
    }
    
    return "";
}

void MelissaCommandComboBox::update()
{
    
}
