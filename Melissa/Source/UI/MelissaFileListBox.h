#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "MelissaDataSource.h"
#include "MelissaHost.h"
#include "MelissaLookAndFeel.h"
#include "MelissaUISettings.h"

class MelissaFileListBox : public ListBox,
                           public ListBoxModel,
                           public MelissaDataSourceListener
{
public:
    enum Target
    {
        kTarget_History = -1,
        kTarget_Playlist,
    };
    
    MelissaFileListBox(const String& componentName = "") :
    ListBox(componentName, this),
    target_(kTarget_History),
    dataSource_(MelissaDataSource::getInstance())
    {
        dataSource_->addListener(this);
        popupMenu_ = std::make_unique<PopupMenu>();
        popupMenu_->setLookAndFeel(&lookAndFeel_);
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
    
    void paint(Graphics& g) override
    {
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRect(0, 0, getWidth(), getHeight());
    }
    
    int getNumRows() override
    {
        return list_.size();
    }
    
    void listBoxItemClicked(int row, const MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown())
        {
            enum { kMenuId_Remove = 1 };
            popupMenu_->clear();
            popupMenu_->addItem(kMenuId_Remove, TRANS("remove"), true);
            if (popupMenu_->show() == kMenuId_Remove)
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
        }
    }
    
    void listBoxItemDoubleClicked(int row, const MouseEvent& e) override
    {
        dataSource_->loadFileAsync(list_[row]);
    }
    
    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override
    {
        const String fullPath = (rowNumber < list_.size()) ?  list_[rowNumber] : "";
        const String fileName = File(fullPath).getFileName();
        
        g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(rowIsSelected ? 0.1f : 0.f));
        g.fillAll();
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(MelissaUISettings::getFontSizeMain());
        g.drawText(fileName, 10, 0, width - 20, height, Justification::left);
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
        updateList();
        selectRow(0);
    }
    
    void playlistUpdated(size_t index) override
    {
        updateList();
    }
    
private:
    std::unique_ptr<PopupMenu> popupMenu_;
    Target target_;
    MelissaDataSource* dataSource_;
    MelissaDataSource::FilePathList list_;
    MelissaLookAndFeel lookAndFeel_;
};
