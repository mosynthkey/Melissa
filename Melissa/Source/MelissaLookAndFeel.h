#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MelissaLookAndFeel : public LookAndFeel_V4
{
public:
    virtual ~MelissaLookAndFeel() {};
    
    void drawButtonBackground(Graphics& g, Button& b, const Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto& c = b.getLocalBounds();
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, shouldDrawButtonAsHighlighted ? 0.8f : 0.4f));
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void drawButtonText(Graphics& g, TextButton& tb, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
        g.drawText(tb.getButtonText(), 0, 0, tb.getWidth(), tb.getHeight(), Justification::centred);
    }
    
    Font getTextButtonFont(TextButton& tb, int buttonHeight) override
    {
        return Font(14);
    }
    
    Font getLabelFont(Label& l) override
    {
        return Font(14);
    }
    
    void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& te) override
    {

    }
    
    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& te) override
    {
        const auto& c = te.getLocalBounds();
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void drawComboBox(Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& cb) override
    {
        const auto& c = cb.getLocalBounds();
        g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
        g.drawRoundedRectangle(lineThickNess_ / 2, lineThickNess_ / 2, c.getWidth() - lineThickNess_ - 1, c.getHeight() - lineThickNess_ - 1, (c.getHeight() - lineThickNess_) / 2, lineThickNess_);
    }
    
    void layoutFileBrowserComponent(FileBrowserComponent &browserComp, DirectoryContentsDisplayComponent *fileListComponent, FilePreviewComponent *previewComp, ComboBox *currentPathBox, TextEditor *filenameBox, Button *goUpButton) override
    {
    }
    
private:
    const float lineThickNess_ = 1.4;
};
