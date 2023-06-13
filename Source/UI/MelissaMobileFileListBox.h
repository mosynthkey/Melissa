//
//  MelissaMobileFileListBox.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "MelissaDataSource.h"
#include "MelissaHost.h"
#include "MelissaLookAndFeel.h"
#include "MelissaUISettings.h"

class MelissaMobileFileListBox : public juce::ListBox,
                                 public juce::ListBoxModel,
                                 public MelissaDataSourceListener
{
public:
    MelissaMobileFileListBox(const juce::String& componentName = "") :
    dataSource_(MelissaDataSource::getInstance())
    {
        list_.clear();
        
        constexpr int numDummyData = 50;
        list_.resize(numDummyData);
        
        using namespace juce;
        for (int i = 0; i < numDummyData; ++i) list_[i] = (String("Dummy ") + String(i));
        
        juce::ListBox(componentName, this);
        setModel(this);
        
        dataSource_->addListener(this);
        setOutlineThickness(1.f);
        
        setRowHeight(60);
    }
    
    ~MelissaMobileFileListBox()
    {
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override
    {
        dataSource_->loadFileAsync(list_[row]);
    }
    
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
    {
        const juce::String fullPath = (rowNumber < list_.size()) ?  list_[rowNumber] : "";
        //const juce::String fileName = juce::File(fullPath).getFileName();
        
        g.setColour(juce::Colours::pink);
        g.drawRect(0, 0, getWidth(), getHeight());
        
        if (rowIsSelected)
        {
            g.fillAll(MelissaUISettings::getAccentColour(0.4f));
        }
        
        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(fullPath, 10, 0, width - 20, height, juce::Justification::left);
    }
    
private:
    MelissaDataSource* dataSource_;
    std::vector<juce::String> list_;
};
