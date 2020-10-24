//
//  MelissaShortcutComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaCommand.h"
#include "MelissaDataSource.h"
#include "MelissaLookAndFeel.h"
#include "MelissaShortcutComponent.h"
#include "MelissaUISettings.h"

class MelissaShortcutComponent::ShortcutListBox : public MelissaDataSourceListener,
                                                  public TableListBox,
                                                  public TableListBoxModel
{
public:
    enum Column
    {
        kColumn_Shortcut,
        kColumn_Command,
        kNumOfColumn
    };
    
    ShortcutListBox(const String& componentName = "") :
    TableListBox(componentName, this)
    {
        dataSource_ = MelissaDataSource::getInstance();
        update();
        
        String headerTitles[kNumOfColumn] = { "Shortcut", "Command" };
        for (int i = 0; i < kNumOfColumn; ++i)
        {
            getHeader().addColumn(headerTitles[i], i + 1, 300);
        }
        setOutlineThickness(1);
    }
    
    ~ShortcutListBox()
    {
        
    }
    
    void update()
    {
        shortcuts_.clear();
        auto shortcuts = dataSource_->getAllAssignedShortcuts();
        for (auto&& shortcut : shortcuts)
        {
            shortcuts_.emplace_back(std::make_pair(shortcut.first, shortcut.second));
        }
    }
    
    int getNumRows() override
    {
        return static_cast<int>(shortcuts_.size());
    }
    
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        const auto colour = Colour(MelissaUISettings::getMainColour()).withAlpha(rowIsSelected ? 0.2f : 0.f);
        g.fillAll(colour);
    }
    
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        auto shortcut = shortcuts_[rowNumber];
        String text = (columnId == 1) ? shortcut.first :  MelissaCommand::getCommandDescription(shortcut.second);
        
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.setFont(MelissaUISettings::getFontSizeMain());
        constexpr int xMargin = 10;
        g.drawText(text, xMargin, 0, width - xMargin * 2, height, Justification::left);
    }
    
    /*
    int  getColumnAutoSizeWidth(int columnId) override
    {
        
    }
     */
    
    void cellClicked(int rowNumber, int columnId, const MouseEvent& e) override
    {
        
    }
    
private:
    MelissaDataSource* dataSource_;
    
    std::vector<std::pair<String, String>> shortcuts_;
};

MelissaShortcutComponent::MelissaShortcutComponent()
{
    shortcutListBox_ = std::make_unique<ShortcutListBox>();
    addAndMakeVisible(shortcutListBox_.get());
}

MelissaShortcutComponent::~MelissaShortcutComponent()
{
    
}

void MelissaShortcutComponent::resized()
{
    shortcutListBox_->setBounds(getLocalBounds().reduced(10, 10));
}
