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
    std::unique_ptr<MelissaMenuButton> menuButton_;
    std::unique_ptr<MelissaAddButton> addToPlaylistButton_;
    std::unique_ptr<MelissaFileListBox> listBox_;
    MelissaLookAndFeel laf_;
};
