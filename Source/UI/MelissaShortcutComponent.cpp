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

using namespace juce;

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
    
    ShortcutListBox(MelissaShortcutComponent* owner, const String& componentName = "") :
    TableListBox(componentName, this), owner_(owner)
    {
        dataSource_ = MelissaDataSource::getInstance();
        dataSource_->addListener(this);
        
        update();
        
        String headerTitles[kNumOfColumn] = { "Shortcut", "Command" };
        for (int i = 0; i < kNumOfColumn; ++i)
        {
            getHeader().addColumn(headerTitles[i], i + 1, 300);
        }
        setOutlineThickness(1);
        
        setWantsKeyboardFocus(false);
    }
    
    ~ShortcutListBox()
    {
        dataSource_->removeListener(this);
    }
    
    void update()
    {
        shortcuts_.clear();
        const auto shortcuts = dataSource_->getAllAssignedShortcuts();
        for (auto&& shortcut : shortcuts)
        {
            shortcuts_.emplace_back(std::make_pair(shortcut.first, shortcut.second));
        }
        updateContent();
        repaint();
    }
    
    int getNumRows() override
    {
        return static_cast<int>(shortcuts_.size());
    }
    
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
        {
            g.fillAll(MelissaUISettings::getAccentColour(0.4f));
        }
    }
    
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        const auto shortcut = shortcuts_[rowNumber];
        String text = (columnId == 1) ? shortcut.first :  MelissaCommand::getCommandDescription(shortcut.second);
        
        g.setColour(MelissaUISettings::getTextColour());
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
        constexpr int xMargin = 10;
        g.drawText(text, xMargin, 0, width - xMargin * 2, height, Justification::left);
    }
    
    void cellClicked(int rowNumber, int columnId, const MouseEvent& e) override
    {
        const auto shortcut = shortcuts_[rowNumber];
        owner_->controlMessageReceived(shortcut.first);
    }
    
    void shortcutUpdated() override
    {
        update();
    }
    
private:
    MelissaShortcutComponent* owner_;
    MelissaDataSource* dataSource_;
    
    std::vector<std::pair<String, String>> shortcuts_;
};

MelissaShortcutComponent::MelissaShortcutComponent() : registerEditY_(0)
{
    commandLabel_ = std::make_unique<Label>();
    commandLabel_->setJustificationType(Justification::centred);
    commandLabel_->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    commandLabel_->setColour(Label::backgroundColourId, Colours::black);
    commandLabel_->setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(commandLabel_.get());
    
    assignCombobox_ = std::make_unique<MelissaCommandComboBox>();
    assignCombobox_->onSelectedCommandChanged_ = [&](const String& command)
    {
        MelissaDataSource::getInstance()->registerShortcut(commandLabel_->getText(), assignCombobox_->getSelectedCommand());
        grabKeyboardFocus();
    };
    addAndMakeVisible(assignCombobox_.get());
    
    shortcutListBox_ = std::make_unique<ShortcutListBox>(this);
    addAndMakeVisible(shortcutListBox_.get());
    
    resetButton_ = std::make_unique<TextButton>(TRANS("shortcut_reset"));
    resetButton_->onClick = [&]()
    {
        MelissaDataSource::getInstance()->setDefaultShortcut(commandLabel_->getText());
    };
    addAndMakeVisible(resetButton_.get());
    
    resetAllButton_ = std::make_unique<TextButton>(TRANS("shortcut_reset_all"));
    resetAllButton_->onClick = [&]()
    {
        MelissaDataSource::getInstance()->setDefaultShortcuts(true);
    };
    addAndMakeVisible(resetAllButton_.get());
    
    initAssignBox();
    
    shortcutManager_ = MelissaShortcutManager::getInstance();
    shortcutManager_->setEnable(false);
    shortcutManager_->addListener(this);
    
    MelissaDataSource::getInstance()->addListener(this);
}

MelissaShortcutComponent::~MelissaShortcutComponent()
{
    shortcutManager_->removeListener(this);
    shortcutManager_->setEnable(true);
    MelissaDataSource::getInstance()->removeListener(this);
}

void MelissaShortcutComponent::controlMessageReceived(const String& controlMessage)
{
    const auto assignedCommand = MelissaDataSource::getInstance()->getAssignedShortcut(controlMessage);
    commandLabel_->setText(controlMessage, dontSendNotification);
    assignCombobox_->select(assignedCommand);
    assignCombobox_->setShortcut(controlMessage);
}

void MelissaShortcutComponent::resized()
{
    registerEditY_ = getHeight() - 200;
    
    int y = 40;
    
    listRect_ = Rectangle(60, y, getWidth() - 120, registerEditY_ - 10 - y);
    shortcutListBox_->setBounds(listRect_.reduced(2));
    
    int margin = 20;
    int width = (getWidth() - 120 - margin * 2) / 5;
    y = registerEditY_ + 30 + 10;
    commandLabel_->setBounds(60, y, width * 2, 30);
    assignCombobox_->setBounds(commandLabel_->getRight() + margin, y, width * 2, 30);
    resetButton_->setBounds(assignCombobox_->getRight() + margin, y, width, 30);
    
    width = 240;
    resetAllButton_->setBounds(getWidth() - 60 - width, getHeight() - 60, width, 30);
}

void MelissaShortcutComponent::paint(Graphics& g)
{
    g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    
    g.setColour(MelissaUISettings::getTextColour());
    g.drawText(TRANS("shortcut_list"), 60, 0, getWidth() - 120, 30, Justification::left);
    
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(listRect_.toFloat(), 0);
    
    g.setColour(MelissaUISettings::getTextColour());
    g.drawText(TRANS("shortcut_register_edit"), 60, registerEditY_, getWidth() - 120, 30, Justification::left);
    
    g.setColour(MelissaUISettings::getTextColour());
    g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
    g.drawFittedText(TRANS("shortcut_explanation"), 60, registerEditY_ + 80, getWidth() - 120, 30 * 4, Justification::left, 4);
}

void MelissaShortcutComponent::initAssignBox()
{
    const auto shortcuts = MelissaDataSource::getInstance()->getAllAssignedShortcuts();
    if (shortcuts.size() == 0) return;
    
    controlMessageReceived((shortcuts.begin())->first);
    shortcutListBox_->selectRow(0);
}

void MelissaShortcutComponent::shortcutUpdated()
{
    controlMessageReceived(commandLabel_->getText());
}
