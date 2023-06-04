//
//  MelissaExportComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaExportComponent : public juce::Component
{
public:
    MelissaExportComponent();
    
    void resized() override;
    
private:
    std::unique_ptr<juce::Label> formatLabel_;
    std::unique_ptr<juce::ComboBox> formatComboBox_;
    std::unique_ptr<juce::TextButton> exportButton_;
    
    std::unique_ptr<FileChooser> fileChooser_;
};
