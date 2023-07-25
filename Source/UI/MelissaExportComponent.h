//
//  MelissaExportComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaExporter.h"
#include "MelissaLookAndFeel.h"

class MelissaExportComponent : public juce::Component
{
public:
    MelissaExportComponent();
    ~MelissaExportComponent();
    
    void resized() override;
    
private:
    enum ExportTarget
    {
        kExportTarget_Current_Setting,
        kExportTarget_Current_AllPractice,
        kExportTarget_Playlist_AllPractice,
        kNumExportTargets
    };
    ExportTarget selectedTarget_;
    
    class TargetSelectButton;
    std::unique_ptr<juce::Label> targetLabel_;
    std::unique_ptr<TargetSelectButton> targetComponents_[kNumExportTargets];
    std::unique_ptr<juce::Label> targetExplanationLabel_;
    
    std::unique_ptr<juce::Label> playlistLabel_;
    std::unique_ptr<juce::ComboBox> playlistComboBox_;
    
    std::unique_ptr<juce::Label> formatLabel_;
    std::unique_ptr<juce::ComboBox> formatComboBox_;
    
    std::unique_ptr<juce::ToggleButton> eqButton_;
    
    std::unique_ptr<juce::Label> explanationLabel_;
    
    std::unique_ptr<juce::TextButton> exportButton_;
    
    std::unique_ptr<juce::FileChooser> fileChooser_;
    
    void exportCurrentSong(MelissaExporter::ExportFormat format, juce::File fileToExport);
    void exportCurrentSongPracticelist(MelissaExporter::ExportFormat format, juce::File fileToExport);
    void exportPlaylist(int practiceListIndex, MelissaExporter::ExportFormat format, juce::File fileToExport);
    
    MelissaLookAndFeel_CircleToggleButton toggleLaf_;
    
    void update();
};
