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
    iconImages_[kIcon_PlaylistAdd] = Drawable::createFromImageData(BinaryData::playlist_add_svg, BinaryData::playlist_add_svgSize);
    iconImages_[kIcon_PlaylistAddHighlighted] = Drawable::createFromImageData(BinaryData::playlist_add_highlighted_svg, BinaryData::playlist_add_highlighted_svgSize);
    
    iconImages_[kIcon_PlaylistAddFile] = Drawable::createFromImageData(BinaryData::playlist_add_file_svg, BinaryData::playlist_add_file_svgSize);
    iconImages_[kIcon_PlaylistAddFileHighlighted] = Drawable::createFromImageData(BinaryData::playlist_add_file_highlighted_svg, BinaryData::playlist_add_file_highlighted_svgSize);
    
    iconImages_[kIcon_PlaylistAddPlaying] = Drawable::createFromImageData(BinaryData::playlist_add_playing_svg, BinaryData::playlist_add_playing_svgSize);
    iconImages_[kIcon_PlaylistAddPlayingHighlighted] = Drawable::createFromImageData(BinaryData::playlist_add_playing_highlighted_svg, BinaryData::playlist_add_playing_highlighted_svgSize);

    iconImages_[kIcon_PlaylistEdit] = Drawable::createFromImageData(BinaryData::playlist_edit_svg, BinaryData::playlist_edit_svgSize);
    iconImages_[kIcon_PlaylistEditHighlighted] = Drawable::createFromImageData(BinaryData::playlist_edit_highlighted_svg, BinaryData::playlist_edit_highlighted_svgSize);
    
    iconImages_[kIcon_PlaylistRemove] = Drawable::createFromImageData(BinaryData::playlist_remove_svg, BinaryData::playlist_remove_svgSize);
    iconImages_[kIcon_PlaylistRemoveHighlighted] = Drawable::createFromImageData(BinaryData::playlist_remove_highlighted_svg, BinaryData::playlist_remove_highlighted_svgSize);
    
    playlistComboBox_ = std::make_unique<ComboBox>();
    playlistComboBox_->onChange = [&]() { updateList(); };
    addAndMakeVisible(playlistComboBox_.get());
    
    createButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    createButton_->setTooltip(TRANS("create_playlist"));
    createButton_->setImages(iconImages_[kIcon_PlaylistAdd].get(), iconImages_[kIcon_PlaylistAddHighlighted].get());
    createButton_->onClick = [&]()
    {
        auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), "Playlist", [&](const String& name) {
            if (name.isEmpty()) return;
            const size_t index = dataSource_->createPlaylist(name);
            select(static_cast<int>(index));
        });
        MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("create_playlist"));
    };
    addAndMakeVisible(createButton_.get());
    
    renameButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    renameButton_->setTooltip(TRANS("rename_playlist"));
    renameButton_->setImages(iconImages_[kIcon_PlaylistEdit].get(), iconImages_[kIcon_PlaylistEditHighlighted].get());
    renameButton_->onClick = [&]()
    {
        const int index = playlistComboBox_->getSelectedItemIndex();
        if (index < 0) return;
        const String currentName = dataSource_->getPlaylistName(index);
        auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), currentName, [&, index](const String& name) {
            if (name.isEmpty()) return;
            dataSource_->setPlaylistName(index, name);
        });
        MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("rename_playlist"));
    };
    addAndMakeVisible(renameButton_.get());
    
    removeButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    removeButton_->setTooltip(TRANS("remove_playlist"));
    removeButton_->setImages(iconImages_[kIcon_PlaylistRemove].get(), iconImages_[kIcon_PlaylistRemoveHighlighted].get());
    removeButton_->onClick = [&]()
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
    };
    addAndMakeVisible(removeButton_.get());
    
    addFileButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    addFileButton_->setTooltip(TRANS("addtolist_select"));
    addFileButton_->setImages(iconImages_[kIcon_PlaylistAddFile].get(), iconImages_[kIcon_PlaylistAddFileHighlighted].get());
    addFileButton_->onClick = [&]()
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
    };
    addAndMakeVisible(addFileButton_.get());
    
    addPlayingButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    addPlayingButton_->setTooltip(TRANS("addtolist_current"));
    addPlayingButton_->setImages(iconImages_[kIcon_PlaylistAddPlaying].get(), iconImages_[kIcon_PlaylistAddPlayingHighlighted].get());
    addPlayingButton_->onClick = [&]()
    {
        const int index = playlistComboBox_->getSelectedItemIndex();
        dataSource_->addToPlaylist(index, dataSource_->getCurrentSongFilePath());
    };
    addAndMakeVisible(addPlayingButton_.get());
    
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

void MelissaPlaylistComponent::select(int index)
{
    playlistComboBox_->setSelectedItemIndex(index);
    updateList();
}

void MelissaPlaylistComponent::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int margin = 10;
    const int controlWidth = 28;
    const int controlHeight = 30;
    
    playlistComboBox_->setBounds(0, 0, w - (controlWidth + margin) * 3, controlHeight);
    
    createButton_->setBounds(w - (controlWidth + margin) * 3 + margin, 0, controlWidth, controlHeight);
    renameButton_->setBounds(w - (controlWidth + margin) * 2 + margin, 0, controlWidth, controlHeight);
    removeButton_->setBounds(w - (controlWidth + margin) * 1 + margin, 0, controlWidth, controlHeight);
    
    listBox_->setBounds(0, 40, w, h - (controlHeight + margin) * 2);
    
    addFileButton_->setBounds(w - controlWidth - (controlWidth + margin), h - controlHeight, controlWidth, controlHeight);
    addPlayingButton_->setBounds(w - controlWidth, h - controlHeight, controlWidth, controlHeight);
}

void MelissaPlaylistComponent::playlistUpdated(size_t index)
{
    updateComboBox();
    updateList();
}
