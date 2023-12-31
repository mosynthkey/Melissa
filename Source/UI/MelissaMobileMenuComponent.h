//
//  MelissaMobileMenuComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaMobileFileListBox.h"

class MelissaMobileMenuComponent : public juce::Component
{
public:
    
    class MenuCategoryButton;
    enum MenuCategory
    {
        kMenuCategory_File,
        kMenuCategory_Playlist,
        kMenuCategory_Shortcut,
        kMenuCategory_Settings,
        kMenuCategory_IAP,
        kMenuCategory_About,
        kNumMenuCategories
    };
    
    class FileSelectButton;
    
    MelissaMobileMenuComponent();
    ~MelissaMobileMenuComponent();
    
private:
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void updateCategory(MenuCategory menuCategory);
    
    std::unique_ptr<MenuCategoryButton> categoryButtons_[kNumMenuCategories];
    std::unique_ptr<MelissaMobileFileListBox> fileListBox_;
};
