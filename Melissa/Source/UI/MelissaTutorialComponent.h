//
//  MelissaTutorialComponent.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaHost.h"

class MelissaTutorialComponent : public Component
{
public:
    struct Page
    {
        Component* targetComponent_;
        String explanation;
    };
    
    MelissaTutorialComponent(MelissaHost* host);
    void setPages(const std::vector<Page>& pages);
    void update();
    
    // Component
    void paint(Graphics& g) override;
    void resized() override;
    
private:
    MelissaHost* host_;
    std::unique_ptr<TextButton> skipButton_;
    std::unique_ptr<TextButton> nextButton_;
    std::unique_ptr<Label> explanationLabel_;
    
    Rectangle<int> targetRectangle_;
    
    size_t currentPage_;
    std::vector<Page> pages_;
};
