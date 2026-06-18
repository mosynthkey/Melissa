//
//  MelissaFileBrowserOverlayComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaFileListBox.h"
#include "MelissaLookAndFeel.h"
#include "MelissaPlaylistComponent.h"

class MelissaFileBrowserOverlayComponent : public juce::Component,
                                           public juce::FileBrowserListener
{
public:
    MelissaFileBrowserOverlayComponent();
    ~MelissaFileBrowserOverlayComponent();

    void resized() override;
    void paint(juce::Graphics& g) override;

    // FileBrowserListener
    void selectionChanged() override {}
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override {}
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;

    std::function<void()> onFileSelected;

private:
    MelissaDataSource* dataSource_;

    // Labels
    std::unique_ptr<juce::Label> browserLabel_;
    std::unique_ptr<juce::Label> playlistLabel_;
    std::unique_ptr<juce::Label> historyLabel_;

    // File browser
    std::unique_ptr<juce::WildcardFileFilter> wildCardFilter_;
    std::unique_ptr<juce::FileBrowserComponent> fileBrowserComponent_;

    // Playlist
    std::unique_ptr<MelissaPlaylistComponent> playlistComponent_;

    // History
    std::unique_ptr<MelissaFileListBox> historyListBox_;

    // Column bounds for painting
    juce::Rectangle<int> browserColumnBounds_;
    juce::Rectangle<int> playlistColumnBounds_;
    juce::Rectangle<int> historyColumnBounds_;

    MelissaLookAndFeel_FileBrowser browserLaf_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelissaFileBrowserOverlayComponent)
};
