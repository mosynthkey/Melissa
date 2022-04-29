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

class MelissaPracticeTableListBox : public Label::Listener,
                                    public MelissaDataSourceListener,
                                    public TableListBox,
                                    public TableListBoxModel
{
public:
    enum Column
    {
        kColumn_Name,
        kColumn_LoopRange,
        kColumn_Speed,
        kNumOfColumn
    };
    
    MelissaPracticeTableListBox(const String& componentName = "");
    ~MelissaPracticeTableListBox();
    
    void updatePracticeList();
    
    void resized() override;
    int getNumRows() override;
    
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
    int  getColumnAutoSizeWidth(int columnId) override;
    void cellClicked(int rowNumber, int columnId, const MouseEvent& e) override;
    void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e) override;
    void selectedRowsChanged(int row) override;

    
    // MelissaDataSourceListener
    void songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate) override;
    void practiceListUpdated() override;
    
    // Label
    void labelTextChanged(Label* label) override;
    
    void moveSelected(int direction);
    
private:
    MelissaLookAndFeel_SimpleTextEditor laf_;
    MelissaDataSource* dataSource_;
    std::vector<MelissaDataSource::Song::PracticeList> practiceList_;
    float totalLengthMSec_;
    std::shared_ptr<PopupMenu> popupMenu_;
    int selectedRow_;
};
