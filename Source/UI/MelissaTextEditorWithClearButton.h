//
//  MelissaTextEditorWithClearButton.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BinaryData.h"

class MelissaTextEditorWithClearButton : public juce::TextEditor
{
public:
    MelissaTextEditorWithClearButton();
    void resized() override;
    
private:
    std::unique_ptr<juce::DrawableButton> clearButton_;
    std::unique_ptr<juce::Drawable> clearButtonDrawable_;
    std::unique_ptr<juce::Drawable> clearButtonHighlightedDrawable_;
};
