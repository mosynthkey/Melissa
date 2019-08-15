#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

using std::make_unique;

class MelissaDebugComponent : public Component
{
public:
    MelissaDebugComponent()
    {
        fileBrowserComponent_ = make_unique<FileBrowserComponent>(
            FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
            File::getSpecialLocation(File::userHomeDirectory),
            new WildcardFileFilter("*.mp3;*.wav", "*", "Music Files"),
            nullptr);
        fileBrowserComponent_->setBounds(0, 0, 300, 360);
        addAndMakeVisible(fileBrowserComponent_.get());
        
        playButton_ = make_unique<TextButton>();
        playButton_->setButtonText("Play");
        playButton_->setBounds(340, 40, 100, 24);
        addAndMakeVisible(playButton_.get());
        
        stopButton_ = make_unique<TextButton>();
        stopButton_->setButtonText("Stop");
        stopButton_->setBounds(460, 40, 100, 24);
        addAndMakeVisible(stopButton_.get());
        
        pauseButton_ = make_unique<TextButton>();
        pauseButton_->setButtonText("Pause");
        pauseButton_->setBounds(580, 40, 100, 24);
        addAndMakeVisible(pauseButton_.get());
        
        posLabel_ = make_unique<Label>();
        posLabel_->setText("Position", dontSendNotification);
        posLabel_->setBounds(340, 80, 460, 24);
        addAndMakeVisible(posLabel_.get());
        
        posSlider_ = make_unique<Slider>();
        posSlider_->setRange(0, 100, 0.01);
        posSlider_->setSliderStyle(Slider::LinearHorizontal);
        posSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        posSlider_->setBounds(500, 80, 300, 24);
        addAndMakeVisible(posSlider_.get());
        
        aLabel_ = make_unique<Label>();
        aLabel_->setText("A:", dontSendNotification);
        aLabel_->setBounds(340, 120, 150, 24);
        addAndMakeVisible(aLabel_.get());
        
        aSlider_ = make_unique<Slider>();
        aSlider_->setRange(0, 100, 0.01);
        aSlider_->setSliderStyle(Slider::LinearHorizontal);
        aSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        aSlider_->setBounds(500, 120, 300, 24);
        addAndMakeVisible(aSlider_.get());
        
        bLabel_ = make_unique<Label>();
        bLabel_->setText("B:", dontSendNotification);
        bLabel_->setBounds(340, 160, 150, 24);
        addAndMakeVisible(bLabel_.get());
        
        bSlider_ = make_unique<Slider>();
        bSlider_->setRange (0, 100, 0.01);
        bSlider_->setValue(100);
        bSlider_->setSliderStyle(Slider::LinearHorizontal);
        bSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        bSlider_->setBounds(500, 160, 300, 24);
        addAndMakeVisible(bSlider_.get());
        
        rateLabel_ = make_unique<Label>();
        rateLabel_->setText("Rate:", dontSendNotification);
        rateLabel_->setBounds(340, 200, 150, 24);
        addAndMakeVisible(rateLabel_.get());
        
        rateSlider_ = make_unique<Slider>();
        rateSlider_->setRange (200, 1500, 0.01);
        rateSlider_->setValue(1000);
        rateSlider_->setSliderStyle (Slider::LinearHorizontal);
        rateSlider_->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
        rateSlider_->setBounds (500, 200, 300, 24);
        addAndMakeVisible(rateSlider_.get());
        
        statusLabel_ = make_unique<Label>();
        statusLabel_->setBounds(340, 240, 460, 24);
        addAndMakeVisible(statusLabel_.get());
        
        debugLabel_ = make_unique<Label>();
        debugLabel_->setBounds(340, 280, 360, 80);
        debugLabel_->setFont(Font(Font::getDefaultMonospacedFontName(), 10, 10));
        addAndMakeVisible(debugLabel_.get());
    }
    
    void paint(Graphics& g) override
    {
        g.fillAll(Colour::fromRGBA(0, 0, 0, 255 * 0.4f));
    }
    
    std::unique_ptr<FileBrowserComponent> fileBrowserComponent_;
    
    std::unique_ptr<TextButton> playButton_;
    std::unique_ptr<TextButton> stopButton_;
    std::unique_ptr<TextButton> pauseButton_;
    
    std::unique_ptr<Label>      posLabel_;
    std::unique_ptr<Slider>     posSlider_;
    
    std::unique_ptr<Label>      aLabel_;
    std::unique_ptr<Slider>     aSlider_;
    
    std::unique_ptr<Label>      bLabel_;
    std::unique_ptr<Slider>     bSlider_;
    
    std::unique_ptr<Label>      rateLabel_;
    std::unique_ptr<Slider>     rateSlider_;
    
    std::unique_ptr<Label>      statusLabel_;
    std::unique_ptr<Label>      debugLabel_;
};
