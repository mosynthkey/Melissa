//
//  MelissaCommandComboBox.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaCommandComboBox.h"
#include "MelissaDataSource.h"
#include "MelissaUISettings.h"


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
    kNumOfCommandCategories
};

typedef std::pair<String, std::vector<String>> TitleAndCommandList;
static const TitleAndCommandList commandList[kNumOfCommandCategories] =
{
    {
        "Transport",
        {
            "StartStop",
            "Back",
            "Next",
            "PlaybackPositionValue",
            "PlaybackPosition_Plus1Sec",
            "PlaybackPosition_Minus1Sec",
            "PlaybackPosition_Plus5Sec",
            "PlaybackPosition_Minus5Sec",
        }
    },
    {
        "Pitch",
        {
            "PitchValue",
            "Pitch_Plus",
            "Pitch_Minus",
        }
    },
    {
        "Loop",
        {
            "ResetLoop",
            "ResetLoopStart",
            "ResetLoopEnd",
            "SetLoopStart",
            "SetLoopEnd",
            "SetLoopStartValue",
            "SetLoopEndValue",
            "SetLoopStart_Plus100MSec",
            "SetLoopEnd_Minus100MSec",
            "SetLoopStart_Plus1Sec",
            "SetLoopEnd_Minus1Sec",
        }
    },
    {
        "Speed",
        {
            "SetSpeedValue",
            "SetSpeed_Plus5",
            "SetSpeed_Minus5",
            "SetSpeed_Plus1",
            "SetSpeed_Minus1",
            "ResetSpeed",
        }
    },
    {
        "Metronome",
        {
            "ToggleMetronome",
            "SetAccentPosition",
        }
    },
    {
        "EQ",
        {
            "ToggleEq",
            "SetEqFreqValue",
            "SetEqGainValue",
            "SetEqQValue",
        }
    },
    {
        "Mixer",
        {
            "SetMusicVolumeValue",
            "SetVolumeBalanceValue",
            "SetMetronomeVolumeValue",
        }
    },
    {
        "PracticeList",
        {
            "AddPracticeList",
            "SelectPracticeList_0",
            "SelectPracticeList_1",
            "SelectPracticeList_2",
            "SelectPracticeList_3",
            "SelectPracticeList_4",
            "SelectPracticeList_5",
            "SelectPracticeList_6",
            "SelectPracticeList_7",
            "SelectPracticeList_8",
            "SelectPracticeList_9",
        }
    },
    {
        "Marker",
        {
            "AddMarker",
            "SelectMarker_0",
            "SelectMarker_1",
            "SelectMarker_2",
            "SelectMarker_3",
            "SelectMarker_4",
            "SelectMarker_5",
            "SelectMarker_6",
            "SelectMarker_7",
            "SelectMarker_8",
            "SelectMarker_9",
        }
    },
};

MelissaCommandComboBox::MelissaCommandComboBox() : onSelectedCommandChanged_(nullptr), noAssignMenuId_(1)
{
    popupMenu.setLookAndFeel(&laf_);
    
    int itemId = 0;
    
    for (auto&& titleAndCommandList : commandList)
    {
        PopupMenu subMenu;
        
        for (auto&& command : titleAndCommandList.second)
        {
            subMenu.addItem(++itemId, TRANS(command));
        }
        popupMenu.addSubMenu(TRANS(titleAndCommandList.first), subMenu);
    }
    
    popupMenu.addItem(++itemId, TRANS("NoAssign"));
    noAssignMenuId_ = itemId;
}

MelissaCommandComboBox::~MelissaCommandComboBox()
{
    popupMenu.setLookAndFeel(nullptr);
}

void MelissaCommandComboBox::select(const String& command)
{
    if (command.isEmpty())
    {
        setSelectedId(noAssignMenuId_);
        setText(TRANS("NoAssign"));
        return;
    }
    
    int commandItemId = 0;
    for (auto&& titleAndCommandList : commandList)
    {
        for (auto&& c : titleAndCommandList.second)
        {
            ++commandItemId;
            if (c == command)
            {
                setSelectedId(commandItemId);
                setText(TRANS(getSelectedCommand(commandItemId)));
                return;
            }
        }
    }
}

void MelissaCommandComboBox::showPopup()
{
    auto option = PopupMenu::Options().withTargetComponent(this);
    
    popupMenu.showMenuAsync(option, [&](int itemId)
    {
        const auto command = getSelectedCommand(itemId);
        setSelectedId(itemId);
        setText(TRANS(command));
        if (onSelectedCommandChanged_ != nullptr) onSelectedCommandChanged_(command);
        
        hidePopup();
    });
}

String MelissaCommandComboBox::getSelectedCommand(int itemId) const
{
    int commandItemId = 0;
    for (auto&& titleAndCommandList : commandList)
    {
        for (auto&& command : titleAndCommandList.second)
        {
            ++commandItemId;
            if (commandItemId == itemId) return command;
        }
    }
    
    return "";
}

void MelissaCommandComboBox::update()
{
    
}
