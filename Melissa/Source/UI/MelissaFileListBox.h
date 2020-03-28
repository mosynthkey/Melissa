#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "MelissaDataSource.h"
#include "MelissaHost.h"
#include "MelissaUISettings.h"

class MelissaFileListBox : public ListBox, public ListBoxModel
{
public:
    MelissaFileListBox(const String& componentName = String()) :
    ListBox(componentName, this)
    {
    }
    
    void paint(Graphics& g) override
    {
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRect(0, 0, getWidth(), getHeight());
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void listBoxItemDoubleClicked(int row, const MouseEvent& e) override
    {
        MelissaDataSource::getInstance()->loadFile(list_[row]);
    }
    
    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
    {
        const String fullPath = (rowNumber < list_.size()) ?  list_[rowNumber] : "";
        const String fileName = File(fullPath).getFileName();
        
        g.setColour(Colour(MelissaUISettings::mainColour()).withAlpha(rowIsSelected ? 0.1f : 0.f));
        g.fillAll();
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(MelissaUISettings::fontSizeMain());
        g.drawText(fileName, 10, 0, width - 20, height, Justification::left);
    }
    
    void setList(const Array<String>& list)
    {
        list_ = list;
        updateContent();
        repaint();
    }
    
private:
    Array<String> list_;
};
