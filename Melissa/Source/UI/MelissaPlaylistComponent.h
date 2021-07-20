//
//  MelissaPlaylistComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaButtons.h"
#include "MelissaDataSource.h"
#include "MelissaFileListBox.h"
#include "MelissaLookAndFeel.h"

class MelissaPlaylistComponent : public Component,
                                 public MelissaDataSourceListener
{
public:
    MelissaPlaylistComponent();
    
    void createUI();
    void updateComboBox();
    void updateList();
    void select(int index);
    int  getSelected() const { return playlistComboBox_->getSelectedItemIndex(); }
    
    // Component
    void resized() override;
    
    // MelissaDataSourceListener
    void playlistUpdated(size_t index) override;
    
private:
    MelissaDataSource* dataSource_;
    MelissaDataSource::FilePathList list_;
    std::unique_ptr<FileChooser> fileChooser_;
    std::unique_ptr<ComboBox> playlistComboBox_;
    
    std::unique_ptr<DrawableButton> createButton_;
    std::unique_ptr<DrawableButton> renameButton_;
    std::unique_ptr<DrawableButton> removeButton_;
    
    std::unique_ptr<DrawableButton> upButton_;
    std::unique_ptr<DrawableButton> downButton_;
    std::unique_ptr<DrawableButton> addFileButton_;
    std::unique_ptr<DrawableButton> addPlayingButton_;
    
    std::unique_ptr<MelissaFileListBox> listBox_;
    MelissaLookAndFeel_FileBrowser laf_;
    
    enum
    {
        kIcon_Up,
        kIcon_UpHighlighted,
        kIcon_Down,
        kIcon_DownHighlighted,
        kIcon_PlaylistAdd,
        kIcon_PlaylistAddHighlighted,
        kIcon_PlaylistAddFile,
        kIcon_PlaylistAddFileHighlighted,
        kIcon_PlaylistAddPlaying,
        kIcon_PlaylistAddPlayingHighlighted,
        kIcon_PlaylistEdit,
        kIcon_PlaylistEditHighlighted,
        kIcon_PlaylistRemove,
        kIcon_PlaylistRemoveHighlighted,
        kNumOfIcons
    };
    std::unique_ptr<Drawable> iconImages_[kNumOfIcons];
};
