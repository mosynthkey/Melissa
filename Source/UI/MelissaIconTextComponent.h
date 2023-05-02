//
//  MelissaIconTextComponent.h
//  Melissa
//
//  Copyright(c) 2023 Masaki Ono
//

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaDataSource.h"

class MelissaIconTextComponent : public juce::Component
{
public:
    MelissaIconTextComponent(const void* imageData, const size_t imageNumBytes, const juce::Colour& colour, const juce::String& text)
    {
        image_ = juce::Drawable::createFromImageData(imageData, imageNumBytes);
        image_->replaceColour(juce::Colours::black, colour);
        
        text_ = text;
    }
    
    void paint(juce::Graphics& g) override
    {
        constexpr int xSize = 24;
        image_->drawAt(g, (getWidth() - xSize) / 2, 0, 1.f);
        
        g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Small));
        g.setColour(MelissaUISettings::getTextColour());
        g.drawText(text_, 0, getHeight() - 16, getWidth(), 16, juce::Justification::centred);
    }
    
private:
    std::unique_ptr<juce::Drawable> image_;
    juce::String text_;
};
