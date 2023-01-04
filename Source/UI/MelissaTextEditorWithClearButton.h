//
//  MelissaTextEditorWithClearButton.h
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BinaryData.h"

class MelissaTextEditorWithClearButton : public TextEditor
{
public:
    MelissaTextEditorWithClearButton();
    void resized() override;
    
private:
    std::unique_ptr<DrawableButton> clearButton_;
    std::unique_ptr<Drawable> clearButtonDrawable_;
    std::unique_ptr<Drawable> clearButtonHighlightedDrawable_;
};
