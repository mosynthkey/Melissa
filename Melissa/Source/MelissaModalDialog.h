#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaColourScheme.h"
#include "MelissaHost.h"

class MelissaModalDialog : public Component
{
public:
    MelissaModalDialog(MelissaHost* host, std::shared_ptr<Component>& component, const std::string& title);
    
    // Component
    void paint(Graphics& g) override;
    void resized() override;
    
private:
    MelissaHost* host_;
    std::shared_ptr<Component> component_;
    
    class CloseButton : public Button
    {
    public:
        CloseButton() : Button("")  { }
        ~CloseButton() { }
        
        void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            const bool highlighted = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;
            
            const int size = getWidth();
            g.setColour(Colours::black.withAlpha(0.6f));
            g.fillRoundedRectangle(0, 0, size, size, size / 5);
            
            if (highlighted)
            {
                g.setColour(Colour(MelissaColourScheme::MainColour()).withAlpha(0.2f));
                g.fillRoundedRectangle(0, 0, size, size, size / 6);
            }
            
            const int margin = 4;
            g.setColour(Colours::white.withAlpha(0.8f));
            g.drawLine(margin, margin, size - margin, size - margin);
            g.drawLine(margin, size - margin, size - margin, margin);
            
        }
    };
    
    class BackgroundButton : public Button
    {
    public:
        BackgroundButton() : Button("")  { }
        ~BackgroundButton() { }
        void paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override { }
    };
    
    std::unique_ptr<BackgroundButton> backgroundButton_;
    std::unique_ptr<CloseButton> closeButton_;
    std::unique_ptr<Label> titleLabel_;
};
