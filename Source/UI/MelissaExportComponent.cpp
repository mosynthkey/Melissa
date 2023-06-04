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

MelissaExportComponent::MelissaExportComponent()
{
    formatLabel_ = std::make_unique<Label>();
    formatLabel_->setText(TRANS("file_format"), dontSendNotification);
    addAndMakeVisible(formatLabel_.get());
    
    formatComboBox_ = std::make_unique<ComboBox>();
    
    constexpr int kSeparator = -1;
    std::pair<int, String> formatList[] = {
        { MelissaExporter::kExportFormat_wav48000_24, "wav (48kHz, 24bit)" },
        { MelissaExporter::kExportFormat_wav44100_16, "wav (44.1kHz, 16bit)" },
        { kSeparator, "" },
        { MelissaExporter::kExportFormat_ogg48000,    "ogg (48kHz)" },
        { MelissaExporter::kExportFormat_ogg44100,    "ogg (44.1kHz)"},
    };
    for (auto&& format : formatList)
    {
        if (format.first == kSeparator)
        {
            formatComboBox_->addSeparator();
        }
        else
        {
            formatComboBox_->addItemList(format.second, format.first + 1);
        }
    }
    
    formatComboBox_->setSelectedId(MelissaExporter::kExportFormat_wav48000_24 + 1);
    addAndMakeVisible(formatComboBox_.get());
    
    exportButton_ = std::make_unique<TextButton>();
    exportButton_->setButtonText(TRANS("save"));
    exportButton_->onClick = [&]()
    {
        const auto exportFormat = static_cast<MelissaExporter::ExportFormat>(formatComboBox_->getSelectedId() - 1);
        String extension = (exportFormat == MelissaExporter::kExportFormat_wav48000_24 || exportFormat == MelissaExporter::kExportFormat_wav44100_16) ? "*.wav" : "*.ogg";
        fileChooser_ = std::make_unique<FileChooser> ("", File(MelissaDataSource::getInstance()->getCurrentSongFilePath()).getParentDirectory(), extension);
        fileChooser_->launchAsync(FileBrowserComponent::saveMode, [this, exportFormat] (const FileChooser& chooser) {
            auto exporter = std::make_unique<MelissaExporter>();
            auto dataSource = MelissaDataSource::getInstance();
            auto model = MelissaModel::getInstance();
            File file(dataSource->getCurrentSongFilePath());
            
            std::vector<MelissaExporter::FileAndVolume> fileAndVolumes;
            fileAndVolumes.emplace_back(MelissaExporter::FileAndVolume{file, 1.f});
            exporter->addInputFile(fileAndVolumes, model->getPitch(), model->getSpeed(), model->getLoopAPosRatio(), model->getLoopBPosRatio(), model->getEqSwitch(), model->getEqFreq(0), model->getEqGain(0), model->getEqQ(0), 0);
            exporter->setExportSettings(exportFormat, chooser.getResult());
            MelissaExportManager::getInstance()->regist(std::move(exporter));
            MelissaModalDialog::close();
        });
    };
    addAndMakeVisible(exportButton_.get());
}

void MelissaExportComponent::resized()
{
    constexpr int labelWidth = 100;
    constexpr int buttonWidth = 80;
    constexpr int margin = 10;
    const int comboboxWidth = getWidth() - labelWidth - buttonWidth - margin * 4;
    constexpr int y = 10;
    
    formatLabel_->setBounds(margin, y, labelWidth, 30);
    exportButton_->setBounds(getWidth() - buttonWidth - margin, y, buttonWidth, 30);
    formatComboBox_->setBounds(formatLabel_->getRight() + margin, y, comboboxWidth, 30);
}
