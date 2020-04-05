//
//  MelissaPlaylistComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaInputDialog.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaOptionDialog.h"
#include "MelissaPlaylistComponent.h"

enum
{
    kMenuID_New = 1,
    kMenuID_Rename,
    kMenuID_Remove
};

enum
{
    kMenuIDAddToList_Select = 1,
    kMenuIDAddToList_Current
};

MelissaPlaylistComponent::MelissaPlaylistComponent() :
dataSource_(MelissaDataSource::getInstance())
{
    createUI();
    dataSource_->addListener(this);
    
    updateComboBox();
    updateList();
}

void MelissaPlaylistComponent::createUI()
{
    playlistComboBox_ = std::make_unique<ComboBox>();
    playlistComboBox_->onChange = [&]() { updateList(); };
    addAndMakeVisible(playlistComboBox_.get());
    
    menuButton_ = std::make_unique<MelissaMenuButton>();
    menuButton_->onClick = [&]()
    {
        PopupMenu menu;
        menu.setLookAndFeel(&lookAndFeel_);
        menu.addItem(kMenuID_New, TRANS("create_playlist"));
        menu.addItem(kMenuID_Rename, TRANS("rename_playlist"));
        menu.addItem(kMenuID_Remove, TRANS("remove_playlist"));
        
        const auto result = menu.show();
        if (result == kMenuID_New)
        {
            auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), "Playlist", [&](const String& name) {
                if (name.isEmpty()) return;
                const size_t index = dataSource_->createPlaylist(name);
                select(index);
            });
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("create_playlist"));
        }
        else if (result == kMenuID_Rename)
        {
            const int index = playlistComboBox_->getSelectedItemIndex();
            if (index < 0) return;
            const String currentName = dataSource_->getPlaylistName(index);
            auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), currentName, [&, index](const String& name) {
                if (name.isEmpty()) return;
                dataSource_->setPlaylistName(index, name);
            });
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("rename_playlist"));
        }
        else if (result == kMenuID_Remove)
        {
            const std::vector<String> options = { TRANS("remove"), TRANS("cancel") };
            auto dialog = std::make_shared<MelissaOptionDialog>(TRANS("are_you_sure"), options, [&](size_t yesno) {
                if (yesno == 1 /* no */ ) return;
                
                const int index = playlistComboBox_->getSelectedItemIndex();
                if (index < 0) return;
                dataSource_->removePlaylist(index);
                
                int indexToSelect = index - 1;
                if (indexToSelect < 0) indexToSelect = 0;
                select(indexToSelect);
            });
            MelissaModalDialog::show(dialog, TRANS("remove_playlist"));
            

        }
    };
    addAndMakeVisible(menuButton_.get());
    
    addToPlaylistButton_ = std::make_unique<MelissaAddButton>();
    addToPlaylistButton_->setTooltip(TRANS("tooltip_addto_playlist"));
    addToPlaylistButton_->onClick = [&]()
    {
        PopupMenu menu;
        menu.setLookAndFeel(&lookAndFeel_);
        menu.addItem(kMenuID_New, TRANS("addtolist_select"));
        menu.addItem(kMenuID_Rename, TRANS("addtolist_current"));
        
        const auto result = menu.show();
        if (result == kMenuIDAddToList_Select)
        {
            fileChooser_ = std::make_unique<FileChooser>(TRANS("choose_file_playlist"), File::getCurrentWorkingDirectory(), MelissaDataSource::getCompatibleFileExtensions(), true);
            fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::canSelectMultipleItems, [&] (const FileChooser& chooser)
            {
                const int playlistIndex = playlistComboBox_->getSelectedItemIndex();
                auto results = chooser.getResults();
                results.sort();
                for (auto fileUrl : results)
                {
                    if (fileUrl.existsAsFile())
                    {
                        auto filePath = fileUrl.getFullPathName();
                        dataSource_->addToPlaylist(playlistIndex, filePath);
                    }
                }
            });
        }
        else if (result == kMenuIDAddToList_Current)
        {
            const int index = playlistComboBox_->getSelectedItemIndex();
            dataSource_->addToPlaylist(index, dataSource_->getCurrentSongFilePath());
        }
    };
    addAndMakeVisible(addToPlaylistButton_.get());
    
    listBox_ = std::make_unique<MelissaFileListBox>();
    addAndMakeVisible(listBox_.get());
}

void MelissaPlaylistComponent::updateComboBox()
{
    auto index = playlistComboBox_->getSelectedItemIndex();
    playlistComboBox_->clear();
    
    int id = 1;
    const auto numOfLists = dataSource_->getNumOfPlaylists();
    for (size_t index = 0; index < numOfLists; ++index)
    {
        const auto name = dataSource_->getPlaylistName(index);
        playlistComboBox_->addItem(name, id++);
    }
    
    if (dataSource_->getNumOfPlaylists() <= index)
    {
        index = static_cast<int>(dataSource_->getNumOfPlaylists()) - 1;
    }
    
    select(index);
}

void MelissaPlaylistComponent::updateList()
{
    const auto index = playlistComboBox_->getSelectedItemIndex();
    if (index < 0) return;
    
    listBox_->setTarget(static_cast<MelissaFileListBox::Target>(index));
}

void MelissaPlaylistComponent::select(size_t index)
{
    playlistComboBox_->setSelectedItemIndex(static_cast<int>(index));
    updateList();
}

void MelissaPlaylistComponent::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int margin = 10;
    const int controlWidth = 30;
    const int controlHeight = 30;
    
    playlistComboBox_->setBounds(0, 0, w - (26 + margin + 10), controlHeight);
    menuButton_->setBounds(playlistComboBox_->getRight() + margin + 5, (playlistComboBox_->getHeight() - 14) / 2, 26, 14);
    
    listBox_->setBounds(0, 40, w, h - (controlHeight + margin) * 2);
    addToPlaylistButton_->setBounds(w - controlWidth, h - controlHeight, controlWidth, controlHeight);
}

void MelissaPlaylistComponent::playlistUpdated(size_t index)
{
    updateComboBox();
    updateList();
}
