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
    commandLabel_ = std::make_unique<Label>();
    commandLabel_->setJustificationType(Justification::centred);
    commandLabel_->setFont(MelissaUISettings::getFontSizeMain());
    commandLabel_->setText("Command", dontSendNotification);
    commandLabel_->setColour(Label::backgroundColourId, Colours::black);
    commandLabel_->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(commandLabel_.get());
    
    assignCombobox_ = std::make_unique<MelissaCommandComboBox>();
    assignCombobox_->onSelectedCommandChanged_ = [&](const String& command)
    {
        printf("command = %s\n", command.toRawUTF8());
    };
    addAndMakeVisible(assignCombobox_.get());
    
    shortcutListBox_ = std::make_unique<ShortcutListBox>();
    addAndMakeVisible(shortcutListBox_.get());
    
    initAssignBox();
    
    shortcutManager_ = MelissaShortcutManager::getInstance();
    shortcutManager_->setEnable(false);
    shortcutManager_->addListener(this);
}

MelissaShortcutComponent::~MelissaShortcutComponent()
{
    shortcutManager_->removeListener(this);
    shortcutManager_->setEnable(true);
}

void MelissaShortcutComponent::controlMessageReceived(const String& controlMessage)
{
    printf("controlMessageReceived : %s\n", controlMessage.toRawUTF8());
    
    const auto assignedCommand = MelissaDataSource::getInstance()->getAssignedShortcut(controlMessage);
    commandLabel_->setText(controlMessage, dontSendNotification);
    assignCombobox_->select(assignedCommand);
}

void MelissaShortcutComponent::resized()
{
    commandLabel_->setBounds(10, 10, 200, 30);
    
    int width = getWidth() - 30 - commandLabel_->getRight();
    assignCombobox_->setBounds(commandLabel_->getRight(), 10, width, 30);
    
    width = getWidth() - 20;
    int y = assignCombobox_->getBottom() + 40;
    shortcutListBox_->setBounds(10, y, width, getHeight() - y);
}

void MelissaShortcutComponent::initAssignBox()
{
}
