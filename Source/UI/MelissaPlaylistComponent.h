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

class MelissaPlaylistComponent : public juce::Component,
                                 public MelissaDataSourceListener
{
public:
    MelissaPlaylistComponent();
    ~MelissaPlaylistComponent()
    {
        playlistComboBox_->setLookAndFeel(nullptr);
    }
    
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
    std::unique_ptr<juce::FileChooser> fileChooser_;
    std::unique_ptr<juce::ComboBox> playlistComboBox_;
    
    std::unique_ptr<juce::DrawableButton> createButton_;
    std::unique_ptr<juce::DrawableButton> renameButton_;
    std::unique_ptr<juce::DrawableButton> removeButton_;
    
    std::unique_ptr<juce::DrawableButton> upButton_;
    std::unique_ptr<juce::DrawableButton> downButton_;
    std::unique_ptr<juce::DrawableButton> addFileButton_;
    std::unique_ptr<juce::DrawableButton> addPlayingButton_;
    
    std::unique_ptr<MelissaFileListBox> listBox_;
    MelissaLookAndFeel_FileBrowser laf_;
    
    enum
    {
        kIcon_Up,
        kIcon_Down,
        kIcon_PlaylistAdd,
        kIcon_PlaylistAddFile,
        kIcon_PlaylistAddPlaying,
        kIcon_PlaylistEdit,
        kIcon_PlaylistRemove,
        kNumOfIcons
    };
    std::unique_ptr<juce::Drawable> iconImages_[kNumOfIcons];
    std::unique_ptr<juce::Drawable> iconHighlightedImages_[kNumOfIcons];
};
