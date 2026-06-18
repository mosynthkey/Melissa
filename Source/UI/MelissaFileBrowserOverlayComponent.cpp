//
//  MelissaFileBrowserOverlayComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaFileBrowserOverlayComponent.h"
#include "MelissaModalDialog.h"
#include "MelissaUISettings.h"

using namespace juce;

MelissaFileBrowserOverlayComponent::MelissaFileBrowserOverlayComponent()
    : dataSource_(MelissaDataSource::getInstance())
{
    // Labels
    browserLabel_ = std::make_unique<Label>("", TRANS("browser"));
    browserLabel_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Main));
    browserLabel_->setColour(Label::textColourId, MelissaUISettings::getTextColour());
    browserLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(browserLabel_.get());

    playlistLabel_ = std::make_unique<Label>("", TRANS("playlist"));
    playlistLabel_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Main));
    playlistLabel_->setColour(Label::textColourId, MelissaUISettings::getTextColour());
    playlistLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(playlistLabel_.get());

    historyLabel_ = std::make_unique<Label>("", TRANS("history"));
    historyLabel_->setFont(dataSource_->getFont(MelissaDataSource::Global::kFontSize_Main));
    historyLabel_->setColour(Label::textColourId, MelissaUISettings::getTextColour());
    historyLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(historyLabel_.get());

    // File browser - open in current song's directory if available
    wildCardFilter_ = std::make_unique<WildcardFileFilter>(MelissaDataSource::getCompatibleFileExtensions(), "*", "Filter");
    File initialDirectory = File::getSpecialLocation(File::userHomeDirectory);
    const auto currentSongPath = dataSource_->getCurrentSongFilePath();
    if (currentSongPath.isNotEmpty())
    {
        File currentFile(currentSongPath);
        if (currentFile.existsAsFile())
            initialDirectory = currentFile.getParentDirectory();
    }
    fileBrowserComponent_ = std::make_unique<FileBrowserComponent>(
        FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
        initialDirectory,
        wildCardFilter_.get(),
        nullptr
    );
    fileBrowserComponent_->setLookAndFeel(&browserLaf_);
    fileBrowserComponent_->addListener(this);
    addAndMakeVisible(fileBrowserComponent_.get());

    // Playlist
    playlistComponent_ = std::make_unique<MelissaPlaylistComponent>();
    playlistComponent_->onFileSelected = [this]()
    {
        if (onFileSelected)
            onFileSelected();
    };
    addAndMakeVisible(playlistComponent_.get());

    // History
    historyListBox_ = std::make_unique<MelissaFileListBox>();
    historyListBox_->setTarget(MelissaFileListBox::kTarget_History);
    historyListBox_->onFileSelected = [this]()
    {
        if (onFileSelected)
            onFileSelected();
    };
    addAndMakeVisible(historyListBox_.get());
}

MelissaFileBrowserOverlayComponent::~MelissaFileBrowserOverlayComponent()
{
    fileBrowserComponent_->setLookAndFeel(nullptr);
}

void MelissaFileBrowserOverlayComponent::resized()
{
    const int width = getWidth();
    const int height = getHeight();
    const int margin = 10;
    const int titleHeight = 30;
    const int columnWidth = (width - margin * 4) / 3;
    const int innerMargin = 10;

    int x = margin;
    const int columnHeight = height - margin * 2;

    // Browser column
    browserColumnBounds_ = { x, margin, columnWidth, columnHeight };
    browserLabel_->setBounds(x, margin, columnWidth, titleHeight);
    fileBrowserComponent_->setBounds(x + innerMargin, margin + titleHeight + innerMargin,
                                     columnWidth - innerMargin * 2, columnHeight - titleHeight - innerMargin * 2);
    x += columnWidth + margin;

    // Playlist column
    playlistColumnBounds_ = { x, margin, columnWidth, columnHeight };
    playlistLabel_->setBounds(x, margin, columnWidth, titleHeight);
    playlistComponent_->setBounds(x + innerMargin, margin + titleHeight + innerMargin,
                                  columnWidth - innerMargin * 2, columnHeight - titleHeight - innerMargin * 2);
    x += columnWidth + margin;

    // History column
    historyColumnBounds_ = { x, margin, columnWidth, columnHeight };
    historyLabel_->setBounds(x, margin, columnWidth, titleHeight);
    historyListBox_->setBounds(x + innerMargin, margin + titleHeight + innerMargin,
                               columnWidth - innerMargin * 2, columnHeight - titleHeight - innerMargin * 2);
}

void MelissaFileBrowserOverlayComponent::paint(Graphics& g)
{
    g.fillAll(MelissaUISettings::getMainColour());

    constexpr float kRound = 6.f;
    g.setColour(MelissaUISettings::getSubColour());

    // Draw column backgrounds
    g.fillRoundedRectangle(browserColumnBounds_.toFloat(), kRound);
    g.fillRoundedRectangle(playlistColumnBounds_.toFloat(), kRound);
    g.fillRoundedRectangle(historyColumnBounds_.toFloat(), kRound);
}

void MelissaFileBrowserOverlayComponent::fileDoubleClicked(const File& file)
{
    if (file.existsAsFile())
    {
        dataSource_->loadFileAsync(file);
        if (onFileSelected)
            onFileSelected();
    }
}

void MelissaFileBrowserOverlayComponent::browserRootChanged(const File& newRoot)
{
    newRoot.setAsCurrentWorkingDirectory();
}
