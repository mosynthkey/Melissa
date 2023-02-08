//
//  MelissaFileListBox.h
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

class MelissaFileListBox : public juce::ListBox,
                           public juce::ListBoxModel,
                           public MelissaDataSourceListener
{
public:
    enum Target
    {
        kTarget_History = -1,
        kTarget_Playlist,
    };
    
    MelissaFileListBox(const juce::String& componentName = "") :
    target_(kTarget_History),
    dataSource_(MelissaDataSource::getInstance())
    {
        juce::ListBox(componentName, this);
        setModel(this);
        
        dataSource_->addListener(this);
        popupMenu_ = std::make_unique<juce::PopupMenu>();
        popupMenu_->setLookAndFeel(&laf_);
        setOutlineThickness(1.f);
    }
    
    ~MelissaFileListBox()
    {
        popupMenu_->setLookAndFeel(nullptr);
    }
    
    void setTarget(Target target)
    {
        target_ = target;
        updateList();
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            enum
            {
                kMenuId_Remove = 1,
                kMenuId_Reveal,
            };
            popupMenu_->clear();
            popupMenu_->addItem(kMenuId_Remove, TRANS("remove"), true);
            popupMenu_->addItem(kMenuId_Reveal, TRANS("reveal"), true);
            
            popupMenu_->showMenuAsync(juce::PopupMenu::Options(), [&, row](int result) {
                if (result == kMenuId_Remove)
                {
                    if (target_ == kTarget_History)
                    {
                        dataSource_->removeFromHistory(row);
                    }
                    else
                    {
                        const size_t index = static_cast<size_t>(target_);
                        dataSource_->removeFromPlaylist(index, row);
                    }
                }
                else if (result == kMenuId_Reveal)
                {
                    juce::File file = list_[row];
                    if (file.existsAsFile()) file.revealToUser();
                }
            });
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override
    {
        dataSource_->loadFileAsync(list_[row]);
    }
    
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
    {
        const juce::String fullPath = (rowNumber < list_.size()) ?  list_[rowNumber] : "";
        const juce::String fileName = juce::File(fullPath).getFileName();
        
        if (rowIsSelected)
        {
            g.fillAll(MelissaUISettings::getAccentColour(0.4f));
        }
        
        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        g.drawText(fileName, 10, 0, width - 20, height, juce::Justification::left);
    }
    
    void moveSelected(int incDecValue)
    {
        if (target_ == kTarget_History) return;
        
        const int fromIndex = getSelectedRow();
        const int toIndex   = getSelectedRow() + incDecValue;
        if (toIndex < 0 || list_.size() <= toIndex) return;
        
        const size_t index = static_cast<size_t>(target_);
        dataSource_->playlists_[index].list_.swap(fromIndex, toIndex);
        selectRow(toIndex);
        
        updateList();
    }
    
    void updateList()
    {
        if (target_ == kTarget_History)
        {
            list_ = dataSource_->history_;
        }
        else
        {
            const size_t index = static_cast<size_t>(target_);
            list_ = dataSource_->playlists_[index].list_;
        }
        updateContent();
        repaint();
    }
    
    void historyUpdated() override
    {
        if (target_ != kTarget_History) return;
        updateList();
        selectRow(0);
    }
    
    void playlistUpdated(size_t index) override
    {
        updateList();
    }
    
private:
    std::unique_ptr<juce::PopupMenu> popupMenu_;
    Target target_;
    MelissaDataSource* dataSource_;
    MelissaDataSource::FilePathList list_;
    MelissaLookAndFeel laf_;
};
