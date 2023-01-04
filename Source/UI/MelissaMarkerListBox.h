//
//  MelissaMarkerListBox.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"
#include "MelissaLookAndFeel.h"

class MelissaMarkerListBox : public Label::Listener,
                             public MelissaDataSourceListener,
                             public TableListBox,
                             public TableListBoxModel
{
public:
    enum Column
    {
        kColumn_Time,
        kColumn_Colour,
        kColumn_Memo,
        kNumOfColumn
    };
    
    MelissaMarkerListBox(const String& componentName = "");
    ~MelissaMarkerListBox();
    
    void updateMarkers();
    
    void resized() override;
    int getNumRows() override;
    
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
    int  getColumnAutoSizeWidth(int columnId) override;
    void cellClicked(int rowNumber, int columnId, const MouseEvent& e) override;
    void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e) override;
    
    // Label
    void labelTextChanged(Label* label) override;
    
    // MelissaDataSourceListener
    void songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate) override;
    void markerUpdated() override;
    
private:
    MelissaLookAndFeel_SimpleTextEditor laf_;
    MelissaDataSource* dataSource_;
    std::vector<MelissaDataSource::Song::Marker> markers_;
    float totalLengthMSec_;
    std::shared_ptr<PopupMenu> popupMenu_;
};
