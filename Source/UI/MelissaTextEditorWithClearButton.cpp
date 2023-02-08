//
//  MelissaTextEditorWithClearButton.cpp
//  Melissa
//
//  Copyright(c) 2022 Masaki Ono
//

#include "../JuceLibraryCode/JuceHeader.h"
#include "BinaryData.h"
#include "MelissaTextEditorWithClearButton.h"
#include "MelissaUISettings.h"

using namespace juce;

MelissaTextEditorWithClearButton::MelissaTextEditorWithClearButton()
{
    clearButtonDrawable_ = Drawable::createFromImageData(BinaryData::clear_svg, BinaryData::clear_svgSize);
    clearButtonHighlightedDrawable_ = Drawable::createFromImageData(BinaryData::clear_svg, BinaryData::clear_svgSize);
    clearButtonDrawable_->replaceColour(Colours::white, MelissaUISettings::getTextColour(0.4f));
    clearButtonHighlightedDrawable_->replaceColour(Colours::white, MelissaUISettings::getTextColour(0.8f));
    
    clearButton_ = std::make_unique<DrawableButton>("", DrawableButton::ImageRaw);
    clearButton_->setTooltip(TRANS("create_playlist"));
    clearButton_->setImages(clearButtonDrawable_.get(), clearButtonHighlightedDrawable_.get());
    clearButton_->onClick = [&]()
    {
        setText("");
    };
    addAndMakeVisible(clearButton_.get());
}

void MelissaTextEditorWithClearButton::resized()
{
    TextEditor::resized();
    
    constexpr int clearButtonSize = 18;
    clearButton_->setBounds(getWidth() - 10 - clearButtonSize, (getHeight() - clearButtonSize) / 2, clearButtonSize, clearButtonSize);
}

