//
//  MelissaMobileMenuComponent.cpp
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "MelissaMobileMenuComponent.h"
#include "MelissaUISettings.h"

using namespace juce;

enum
{
    kMenuCategoryGroupId = 230731,
    kBorderWidth = 2,
};

class MelissaMobileMenuComponent::MenuCategoryButton : public Button
{
public:
    MenuCategoryButton() : Button(""), text_("")
    {
        
    }
    
    ~MenuCategoryButton() {}
    
    void setText(const String& text)
    {
        text_ = text;
        repaint();
    }
    
    
private:
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (getToggleState())
        {
            g.fillAll(MelissaUISettings::getAccentColour(0.6f));
        }
        else
        {
            g.fillAll(MelissaUISettings::getSubColour());
        }
        
        g.setColour(MelissaUISettings::getTextColour());
        g.drawText(text_, 0, 0, getWidth(), getHeight(), juce::Justification::centred);
    }
    
    String text_;
};

MelissaMobileMenuComponent::MelissaMobileMenuComponent()
{
    const String categoryNames[] =
    {
        TRANS("category_file"), TRANS("category_playlist"), TRANS("category_shortcut"), TRANS("category_settings"), TRANS("category_iap"), TRANS("category_about")
    };
    
    for (int categoryIndex = 0; categoryIndex < kNumMenuCategories; ++categoryIndex)
    {
        auto b = std::make_unique<MenuCategoryButton>();
        b->setToggleable(true);
        b->setToggleState(categoryIndex == 0, dontSendNotification);
        b->setText(categoryNames[categoryIndex]);
        b->setRadioGroupId(kMenuCategoryGroupId);
        b->onClick = [&, categoryIndex]()
        {
            categoryButtons_[categoryIndex]->setToggleState(true, dontSendNotification);
            updateCategory(static_cast<MenuCategory>(categoryIndex));
        };
        addAndMakeVisible(b.get());
        categoryButtons_[categoryIndex] = std::move(b);
    }
    
    fileListBox_ = std::make_unique<MelissaMobileFileListBox>();
    addAndMakeVisible(fileListBox_.get());
}

MelissaMobileMenuComponent::~MelissaMobileMenuComponent()
{
    
}

void MelissaMobileMenuComponent::resized()
{
    constexpr int categoryButtonWidth = 200;
    const int categoryButtonHeight = 55;
    constexpr int categoryButtonMargin = 2;
    constexpr int categoryX0 = 60;
    constexpr int xMargin = 10;
    constexpr int yMargin = 10;
    for (int categoryIndex = 0; categoryIndex < kNumMenuCategories; ++categoryIndex)
    {
        categoryButtons_[categoryIndex]->setBounds(categoryX0, yMargin + categoryIndex * (categoryButtonHeight + categoryButtonMargin), categoryButtonWidth, categoryButtonHeight);
    }
    
    
    const int contentX0 = categoryX0 + categoryButtonWidth + xMargin * 2 + kBorderWidth;
    const int contentWidth = getWidth() - xMargin - contentX0;
    fileListBox_->setBounds(contentX0, yMargin, contentWidth, getHeight() - yMargin * 2);
}

void MelissaMobileMenuComponent::paint(juce::Graphics& g)
{
    g.fillAll(MelissaUISettings::getMainColour());
    
    g.setColour(MelissaUISettings::getSubColour());
    g.fillRoundedRectangle(categoryButtons_[0]->getRight() + 10, 10, kBorderWidth, getHeight() - 20, kBorderWidth / 2);
}

void MelissaMobileMenuComponent::updateCategory(MenuCategory menuCategory)
{
    
}
