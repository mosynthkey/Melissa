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

enum
{
    kTargetGroup = 40000
};

class MelissaExportComponent::TargetSelectButton : public Button
{
public:
    TargetSelectButton(ExportTarget target) : Button("Export Target Button"), target_(target)
    {
        const int targetIndex = static_cast<int>(target);
        const char* svgData[] = { BinaryData::export_current_svg, BinaryData::export_practiceList_svg, BinaryData::export_playlist_svg, BinaryData::export_playlist_practicelist_svg };
        const int svgSize[] = { BinaryData::export_current_svgSize, BinaryData::export_practiceList_svgSize, BinaryData::export_playlist_svgSize,  BinaryData::export_playlist_practicelist_svgSize };
        image_ = Drawable::createFromImageData(svgData[targetIndex], svgSize[targetIndex]);
        image_->replaceColour(Colours::white, MelissaUISettings::getTextColour());
    }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            g.fillAll(MelissaUISettings::getSubColour());
        }
        else
        {
            g.fillAll(MelissaUISettings::getMainColour());
        }
        
        image_->drawAt(g, 0.f, 0.f, 1.f);
        
        if (getToggleState())
        {
            g.setColour(MelissaUISettings::getAccentColour());
            g.drawRect(0, 0, getWidth(), getHeight(), 2);
        }
    }
    
private:
    ExportTarget target_;
    std::unique_ptr<Drawable> image_;
};

using IdAndText = std::pair<int, String>;

MelissaExportComponent::MelissaExportComponent() : selectedTarget_(kExportTarget_Current_Setting)
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
    
    for (int targetIndex = 0; targetIndex < kNumExportTargets; ++targetIndex)
    {
        auto t = std::make_unique<TargetSelectButton>(static_cast<ExportTarget>(targetIndex));
        t->setRadioGroupId(kTargetGroup);
        t->setToggleable(true);
        t->onClick = [&, targetIndex]()
        {
            targetComponents_[targetIndex]->setToggleState(true, dontSendNotification);
            selectedTarget_ = static_cast<ExportTarget>(targetIndex);
            update();
        };
        addAndMakeVisible(t.get());
        targetComponents_[targetIndex] = std::move(t);
    }
    targetComponents_[0]->setToggleState(true, dontSendNotification);
    selectedTarget_ = kExportTarget_Current_Setting;
    
    targetExplanationLabel_ = std::make_unique<Label>();
    targetExplanationLabel_->setJustificationType(Justification::centred);
    addAndMakeVisible(targetExplanationLabel_.get());
    
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
            if (!chooser.getResult().getFullPathName().isEmpty())
            {
                if (selectedTarget_ == kExportTarget_Current_Setting)
                {
                    exportCurrentSong(exportFormat, chooser.getResult());
                }
                else if (selectedTarget_ == kExportTarget_Current_AllPractice)
                {
                    exportCurrentSongPracticelist(exportFormat, chooser.getResult());
                }
                else if (selectedTarget_ == kExportTarget_Playlist)
                {
                    if (playlistComboBox_->getSelectedId() != 0)
                    {
                        exportPlaylist(playlistComboBox_->getSelectedId() - 1, exportFormat, chooser.getResult(), false);
                    }
                }
                else if (selectedTarget_ == kExportTarget_Playlist_AllPractice)
                {
                    if (playlistComboBox_->getSelectedId() != 0)
                    {
                        exportPlaylist(playlistComboBox_->getSelectedId() - 1, exportFormat, chooser.getResult(), true);
                    }
                }
            }
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
    constexpr int labelWidth = 100;
    constexpr int buttonWidth = 80;
    constexpr int margin = 10;
    const int comboboxWidth = getWidth() - labelWidth - margin * 3;
    
    int y = 10;
    constexpr int kImageWidth = 100;
    constexpr int kImageHeight = 60;
    constexpr int kImageMargin = 50;
    
    targetLabel_->setBounds(margin, y, labelWidth, 30 + kImageHeight + margin + 30 + margin);
    const int buttonX0 = targetLabel_->getRight() + margin + comboboxWidth / 2 - (kImageWidth * kNumExportTargets + kImageMargin * (kNumExportTargets - 1)) / 2;
    for (int targetIndex = 0; targetIndex < kNumExportTargets; ++targetIndex)
    {
        targetComponents_[targetIndex]->setBounds(buttonX0 + (kImageWidth + kImageMargin) * targetIndex, y, kImageWidth, kImageHeight);
    }
    y = targetComponents_[0]->getBottom() + margin;
    
    targetExplanationLabel_->setBounds(margin + labelWidth, y, comboboxWidth, 30);
    y = targetExplanationLabel_->getBottom() + margin;
    
    playlistLabel_->setBounds(margin + labelWidth + margin, y, labelWidth, 30);
    playlistComboBox_->setBounds(playlistLabel_->getRight() + margin, y, comboboxWidth - labelWidth - margin, 30);
    y = playlistLabel_->getBottom() + margin * 2;
    
    formatLabel_->setBounds(margin, y, labelWidth, 30);
    formatComboBox_->setBounds(formatLabel_->getRight() + margin, y, comboboxWidth, 30);
    
    y = formatLabel_->getBottom() + margin;
    eqButton_->setBounds(margin, y, getWidth() - margin * 2, 30);
    
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

void MelissaExportComponent::exportCurrentSongPracticelist(MelissaExporter::ExportFormat format, juce::File fileToExport)
{
    auto exporter = std::make_unique<MelissaExporter>();
    auto dataSource = MelissaDataSource::getInstance();
    auto model = MelissaModel::getInstance();
    
    const auto filePath = dataSource->getCurrentSongFilePath();
    
    MelissaDataSource::Song song;
    dataSource->getSong(filePath, song);
    const bool eqSwitch = eqButton_->getToggleState() && model->getEqSwitch();
    
    for (auto&& prac : song.practiceList_)
    {
        std::vector<MelissaExporter::FileAndVolume> fileAndVolumes;
        fileAndVolumes.emplace_back(MelissaExporter::FileAndVolume{filePath, 1.f});
        exporter->addInputFile(fileAndVolumes, song.pitch_, prac.speed_, prac.aRatio_, prac.bRatio_, eqSwitch, model->getEqFreq(0), model->getEqGain(0), model->getEqQ(0), 3000, 1000, 1000);
    }
    
    exporter->setExportSettings(format, fileToExport);
    MelissaExportManager::getInstance()->regist(std::move(exporter));
}

void MelissaExportComponent::exportPlaylist(int practiceListIndex, MelissaExporter::ExportFormat format, juce::File fileToExport, bool exportPracticeList)
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
        
        if (exportPracticeList)
        {
            for (auto&& prac : song.practiceList_)
            {
                std::vector<MelissaExporter::FileAndVolume> fileAndVolumes;
                fileAndVolumes.emplace_back(MelissaExporter::FileAndVolume{filePath, 1.f});
                exporter->addInputFile(fileAndVolumes, song.pitch_, prac.speed_, prac.aRatio_, prac.bRatio_, false, 0.f, 0.f, 0.f, 3000, 1000, 1000);
            }
        }
        else
        {
            std::vector<MelissaExporter::FileAndVolume> fileAndVolumes;
            fileAndVolumes.emplace_back(MelissaExporter::FileAndVolume{filePath, 1.f});
            exporter->addInputFile(fileAndVolumes, song.pitch_, song.speed_, 0.f, 1.f, false, 0.f, 0.f, 0.f, 3000, 1000, 1000);
        }

    }
    
    exporter->setExportSettings(format, fileToExport);
    MelissaExportManager::getInstance()->regist(std::move(exporter));
}

void MelissaExportComponent::update()
{
    const bool enablePlaylist = (selectedTarget_ == kExportTarget_Playlist || selectedTarget_ == kExportTarget_Playlist_AllPractice);
    playlistLabel_->setEnabled(enablePlaylist);
    playlistComboBox_->setEnabled(enablePlaylist);
    
    String explanation[] = { TRANS("export_current"), TRANS("export_current_practice"), TRANS("export_playlist_all"), TRANS("export_playlist_practice") };
    targetExplanationLabel_->setText(explanation[static_cast<int>(selectedTarget_)], dontSendNotification);
    
    auto dataSource = MelissaDataSource::getInstance();
    
    const int selectedId = playlistComboBox_->getSelectedId();
    playlistComboBox_->clear();
    const auto numPlaylists = dataSource->getNumOfPlaylists();
    for (int playlistIndex = 0; playlistIndex < numPlaylists; ++playlistIndex)
    {
        playlistComboBox_->addItem(dataSource->getPlaylistName(playlistIndex), playlistIndex + 1);
    }
    if (selectedId == 0 && numPlaylists != 0)
    {
        playlistComboBox_->setSelectedId(1);
    }
    else if (numPlaylists == 0)
    {
        playlistComboBox_->setSelectedId(0);
    }
    else
    {
        playlistComboBox_->setSelectedId(selectedId);
    }
}
