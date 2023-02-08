//
//  MelissaPracticeTableListBox.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaLookAndFeel.h"

class MelissaPracticeTableListBox : public juce::Label::Listener,
                                    public MelissaDataSourceListener,
                                    public juce::TableListBox,
                                    public juce::TableListBoxModel
{    
public:
    enum Column
    {
        kColumn_Name,
        kColumn_LoopRange,
        kColumn_Speed,
        kNumOfColumn
    };
    
    MelissaPracticeTableListBox(const juce::String& componentName = "");
    ~MelissaPracticeTableListBox();
    
    void updatePracticeList();
    
    void resized() override;
    int getNumRows() override;
    
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
    int  getColumnAutoSizeWidth(int columnId) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override;
    void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override;
    void selectedRowsChanged(int row) override;

    
    // MelissaDataSourceListener
    void songChanged(const juce::String& filePath, size_t bufferLength, int32_t sampleRate) override;
    void practiceListUpdated() override;
    
    // juce::Label
    void labelTextChanged(juce::Label* label) override;
    
    void moveSelected(int direction);
    
private:
    MelissaLookAndFeel_SimpleTextEditor laf_;
    MelissaDataSource* dataSource_;
    std::vector<MelissaDataSource::Song::PracticeList> practiceList_;
    float totalLengthMSec_;
    std::shared_ptr<juce::PopupMenu> popupMenu_;
    int selectedRow_;
};
