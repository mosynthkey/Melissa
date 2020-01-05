#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaHost.h"

class MelissaFileListBox : public ListBox, public ListBoxModel
{
public:
    MelissaFileListBox(MelissaHost* host, const String& componentName = String()) :
    ListBox(componentName, this), host_(host)
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
        host_->loadFile(list_[row].toString());
    }
    
    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
    {
        const String fullPath = (rowNumber < list_.size()) ?  list_[rowNumber].toString() : "";
        const String fileName = File(fullPath).getFileName();
        
        g.fillAll(Colour::fromFloatRGBA(1.f, 1.f, 1.f, rowIsSelected ? 0.1f : 0.f));
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.drawText(fileName, 10, 0, width - 20, height, Justification::left);
    }
    
    void setList(const Array<var>& list)
    {
        list_ = list;
        updateContent();
        repaint();
    }
    
private:
    Array<var> list_;
    MelissaHost* host_;
};
