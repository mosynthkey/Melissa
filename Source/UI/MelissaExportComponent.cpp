//
//  MelissaExportComponent.cpp
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaDataSource.h"
#include "MelissaExportComponent.h"
#include "MelissaExportManager.h"
#include "MelissaModalDialog.h"

using namespace juce;

enum ExportTarget
{
    kExportTarget_Current_Setting,
    kExportTarget_Current_AllPractice,
    kExportTarget_Playlist_AllPractice,
    kNumExportTargets
};

using IdAndText = std::pair<int, String>;

MelissaExportComponent::MelissaExportComponent()
{
    constexpr int kSeparator = -1;
    auto setComboBoxContent = [&](const std::vector<IdAndText>& idAndTexts, ComboBox* comboBox)
    {
        for (auto&& idAndText : idAndTexts)
        {
            if (idAndText.first == kSeparator)
            {
                comboBox->addSeparator();
            }
            else
            {
                comboBox->addItemList(idAndText.second, idAndText.first + 1);
            }
        }
    };
    
    formatLabel_ = std::make_unique<Label>();
    formatLabel_->setText(TRANS("file_format"), dontSendNotification);
    addAndMakeVisible(formatLabel_.get());
    
    formatComboBox_ = std::make_unique<ComboBox>();
    std::vector<IdAndText> formatList = {
        { MelissaExporter::kExportFormat_wav48000_24, "wav (48kHz, 24bit)" },
        { MelissaExporter::kExportFormat_wav44100_16, "wav (44.1kHz, 16bit)" },
        { kSeparator, "" },
        { MelissaExporter::kExportFormat_ogg48000,    "ogg (48kHz)" },
        { MelissaExporter::kExportFormat_ogg44100,    "ogg (44.1kHz)"},
    };
    setComboBoxContent(formatList, formatComboBox_.get());
    formatComboBox_->setSelectedId(MelissaExporter::kExportFormat_wav48000_24 + 1);
    addAndMakeVisible(formatComboBox_.get());

    targetLabel_= std::make_unique<Label>();
    targetLabel_->setText(TRANS("export_target"), dontSendNotification);
    addAndMakeVisible(targetLabel_.get());
    
    targetComboBox_ = std::make_unique<ComboBox>();
    std::vector<IdAndText> targetList = {
        { kExportTarget_Current_Setting,      TRANS("export_current") },
        { kExportTarget_Current_AllPractice,  TRANS("export_current_practice") },
        { kSeparator, "" },
        { kExportTarget_Playlist_AllPractice, TRANS("export_playlist_all")},
    };
    setComboBoxContent(targetList, targetComboBox_.get());
    targetComboBox_->setSelectedId(kExportTarget_Current_Setting + 1);
    targetComboBox_->onChange = [&]() { update(); };
    addAndMakeVisible(targetComboBox_.get());
    
    playlistLabel_= std::make_unique<Label>();
    playlistLabel_->setText(TRANS("playlist"), dontSendNotification);
    addAndMakeVisible(playlistLabel_.get());
    
    playlistComboBox_ = std::make_unique<ComboBox>();
    addAndMakeVisible(playlistComboBox_.get());
    
    eqButton_ = std::make_unique<ToggleButton>();
    eqButton_->setButtonText(TRANS("export_include_eq"));
    eqButton_->setLookAndFeel(&toggleLaf_);
    addAndMakeVisible(eqButton_.get());
    
    explanationLabel_= std::make_unique<Label>();
    explanationLabel_->setText(TRANS("export_explanation"), dontSendNotification);
    addAndMakeVisible(explanationLabel_.get());
    
    exportButton_ = std::make_unique<TextButton>();
    exportButton_->setButtonText(TRANS("save"));
    exportButton_->onClick = [&]()
    {
        const auto exportFormat = static_cast<MelissaExporter::ExportFormat>(formatComboBox_->getSelectedId() - 1);
        String extension = (exportFormat == MelissaExporter::kExportFormat_wav48000_24 || exportFormat == MelissaExporter::kExportFormat_wav44100_16) ? "*.wav" : "*.ogg";
        fileChooser_ = std::make_unique<FileChooser> ("", File(MelissaDataSource::getInstance()->getCurrentSongFilePath()).getParentDirectory(), extension);
        fileChooser_->launchAsync(FileBrowserComponent::saveMode, [this, exportFormat] (const FileChooser& chooser) {
            exportPlaylist(0, exportFormat, chooser.getResult());
            MelissaModalDialog::close();
        });
    };
    addAndMakeVisible(exportButton_.get());
    
    update();
}

MelissaExportComponent::~MelissaExportComponent()
{
    eqButton_->setLookAndFeel(nullptr);
}

void MelissaExportComponent::resized()
{
    constexpr int labelWidth = 120;
    constexpr int buttonWidth = 80;
    constexpr int margin = 10;
    const int comboboxWidth = getWidth() - labelWidth - margin * 3;
    
    int y = 10;
    formatLabel_->setBounds(margin, y, labelWidth, 30);
    formatComboBox_->setBounds(formatLabel_->getRight() + margin, y, comboboxWidth, 30);
    
    y = formatLabel_->getBottom() + margin;
    targetLabel_->setBounds(margin, y, labelWidth, 30);
    targetComboBox_->setBounds(targetLabel_->getRight() + margin, y, comboboxWidth, 30);
    
    y = targetLabel_->getBottom() + margin;
    playlistLabel_->setBounds(margin, y, labelWidth, 30);
    playlistComboBox_->setBounds(playlistLabel_->getRight() + margin, y, comboboxWidth, 30);
    
    y = playlistLabel_->getBottom() + margin;
    eqButton_->setBounds(margin, y, 160, 30);
    
    y = eqButton_->getBottom() + margin;
    explanationLabel_->setBounds(margin, y, getWidth() - margin * 2, 60);
    
    y = explanationLabel_->getBottom() + margin;
    exportButton_->setBounds(getWidth() - buttonWidth - margin, y, buttonWidth, 30);
}

void MelissaExportComponent::exportCurrentSong(MelissaExporter::ExportFormat format, juce::File fileToExport)
{
    auto exporter = std::make_unique<MelissaExporter>();
    auto dataSource = MelissaDataSource::getInstance();
    auto model = MelissaModel::getInstance();
    File file(dataSource->getCurrentSongFilePath());
    
    std::vector<MelissaExporter::FileAndVolume> fileAndVolumes;
    fileAndVolumes.emplace_back(MelissaExporter::FileAndVolume{file, 1.f});
    exporter->addInputFile(fileAndVolumes, model->getPitch(), model->getSpeed(), model->getLoopAPosRatio(), model->getLoopBPosRatio(), model->getEqSwitch(), model->getEqFreq(0), model->getEqGain(0), model->getEqQ(0), 0);
    exporter->setExportSettings(format, fileToExport);
    MelissaExportManager::getInstance()->regist(std::move(exporter));
}

void MelissaExportComponent::exportPlaylist(int practiceListIndex, MelissaExporter::ExportFormat format, juce::File fileToExport)
{
    auto exporter = std::make_unique<MelissaExporter>();
    exporter->setExportSettings(format, fileToExport);
    
    auto dataSource = MelissaDataSource::getInstance();
    
    MelissaDataSource::FilePathList filePathList;
    dataSource->getPlaylist(practiceListIndex, filePathList);
    
    for (auto&& filePath : filePathList)
    {
        MelissaDataSource::Song song;
        dataSource->getSong(filePath, song);
        
        for (auto&& prac : song.practiceList_)
        {
            std::vector<MelissaExporter::FileAndVolume> fileAndVolumes;
            fileAndVolumes.emplace_back(MelissaExporter::FileAndVolume{filePath, 1.f});
            exporter->addInputFile(fileAndVolumes, song.pitch_, prac.speed_, prac.aRatio_, prac.bRatio_, false, 0.f, 0.f, 0.f, 3000, 1000, 1000);
        }
    }
    
    MelissaExportManager::getInstance()->regist(std::move(exporter));
}

void MelissaExportComponent::update()
{
    const bool enablePlaylist = ((targetComboBox_->getSelectedId() - 1) == kExportTarget_Playlist_AllPractice);
    playlistLabel_->setEnabled(enablePlaylist);
    playlistComboBox_->setEnabled(enablePlaylist);
    
    auto dataSource = MelissaDataSource::getInstance();
    
    const int selectedId = playlistComboBox_->getSelectedId();
    playlistComboBox_->clear();
    const auto numPlaylists = dataSource->getNumOfPlaylists();
    for (int playlistIndex = 0; playlistIndex < numPlaylists; ++playlistIndex)
    {
        playlistComboBox_->addItem(dataSource->getPlaylistName(playlistIndex), playlistIndex + 1);
    }
    if (selectedId == 0 && 0 < numPlaylists)
    {
        playlistComboBox_->setSelectedId(1);
    }
}
