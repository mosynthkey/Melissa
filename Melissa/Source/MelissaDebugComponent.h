#pragma once

#include <utility>
#include "../JuceLibraryCode/JuceHeader.h"

using std::make_shared;

class MelissaDebugComponent : public Component
{
public:
    MelissaDebugComponent()
    {
        fileBrowserComponent_ = make_shared<FileBrowserComponent>(
            FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles | FileBrowserComponent::filenameBoxIsReadOnly,
            File::getSpecialLocation(File::userHomeDirectory),
                                                                  new WildcardFileFilter("*.mp3;*.wav;*.m4a", "*", "Music Files"),
            nullptr);
        fileBrowserComponent_->setBounds(0, 0, 300, 360);
        fileBrowserComponent_->setFilenameBoxLabel("");
        addAndMakeVisible(fileBrowserComponent_.get());
        
        auto makeLabelAndSlider = [this](int x, int y)
        {
            auto l = make_shared<Label>();
            addAndMakeVisible(l.get());
            l->setBounds(x, y, 150, 24);
            
            auto s = make_shared<Slider>();
            addAndMakeVisible(s.get());
            s->setBounds(x + 160, y, 300, 24);
            
            std::pair<std::shared_ptr<Label>, std::shared_ptr<Slider>> result(l, s);
            return result;//std::make_pair<Label*, TextButton*>(l, b);
        };
        
        playButton_ = make_shared<TextButton>();
        playButton_->setButtonText("Play");
        playButton_->setBounds(340, 40, 80, 34);
        addAndMakeVisible(playButton_.get());
        
        stopButton_ = make_shared<TextButton>();
        stopButton_->setButtonText("Stop");
        stopButton_->setBounds(440, 40, 80, 34);
        addAndMakeVisible(stopButton_.get());
        
        pauseButton_ = make_shared<TextButton>();
        pauseButton_->setButtonText("Pause");
        pauseButton_->setBounds(540, 40, 80, 34);
        addAndMakeVisible(pauseButton_.get());
        
        resetLoopButton_ = make_shared<TextButton>();
        resetLoopButton_->setButtonText("Reset Loop");
        resetLoopButton_->setBounds(640, 40, 80, 34);
        addAndMakeVisible(resetLoopButton_.get());
        
        posLabel_ = make_shared<Label>();
        posLabel_->setText("Position", dontSendNotification);
        posLabel_->setBounds(340, 80, 460, 24);
        addAndMakeVisible(posLabel_.get());
        
        posSlider_ = make_shared<Slider>();
        posSlider_->setRange(0, 100, 0.01);
        posSlider_->setSliderStyle(Slider::LinearHorizontal);
        posSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        posSlider_->setBounds(500, 80, 300, 24);
        addAndMakeVisible(posSlider_.get());
        
        {
            auto lAndS = makeLabelAndSlider(340, 120);
            aLabel_ = lAndS.first;
            aLabel_->setText("A:", dontSendNotification);
            aSlider_ = lAndS.second;
            aSlider_->setRange(0, 100, 0.01);
            aSlider_->setSliderStyle(Slider::LinearHorizontal);
            aSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        }
        
        {
            auto lAndS = makeLabelAndSlider(340, 160);
            bLabel_ = lAndS.first;
            bLabel_->setText("B:", dontSendNotification);
            bSlider_ = lAndS.second;
            bSlider_->setRange (0, 100, 0.01);
            bSlider_->setValue(100);
            bSlider_->setSliderStyle(Slider::LinearHorizontal);
            bSlider_->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
        }
        
        {
            auto lAndS = makeLabelAndSlider(340, 200);
            rateLabel_ = lAndS.first;
            rateLabel_->setText("Rate:", dontSendNotification);
            rateSlider_ = lAndS.second;
            rateSlider_->setRange (200, 1500, 0.01);
            rateSlider_->setValue(1000);
            rateSlider_->setSliderStyle (Slider::LinearHorizontal);
            rateSlider_->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
        }
        
        {
            auto lAndS = makeLabelAndSlider(340, 240);
            pitchLabel_ = lAndS.first;
            pitchLabel_->setText("Semitone:", dontSendNotification);
            pitchSlider_ = lAndS.second;
            pitchSlider_->setRange (-24, 24, 1);
            pitchSlider_->setValue(0);
            pitchSlider_->setSliderStyle (Slider::LinearHorizontal);
            pitchSlider_->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
        }
        
        {
            auto lAndS = makeLabelAndSlider(340, 280);
            volumeLabel_ = lAndS.first;
            volumeLabel_->setText("Volume:", dontSendNotification);
            volumeSlider_ = lAndS.second;
            volumeSlider_->setRange (0, 2, 0.01);
            volumeSlider_->setValue(1);
            volumeSlider_->setSliderStyle (Slider::LinearHorizontal);
            volumeSlider_->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
        }
        
        statusLabel_ = make_shared<Label>();
        statusLabel_->setBounds(840, 40, 460, 24);
        addAndMakeVisible(statusLabel_.get());
        
        debugLabel_ = make_shared<Label>();
        debugLabel_->setBounds(840, 60, 360, 80);
        debugLabel_->setFont(Font(Font::getDefaultMonospacedFontName(), 10, 10));
        addAndMakeVisible(debugLabel_.get());
    }
    
    void paint(Graphics& g) override
    {
        g.fillAll(Colour::fromRGBA(0, 0, 0, 255 * 0.4f));
    }
    
    std::shared_ptr<FileBrowserComponent> fileBrowserComponent_;
    
    std::shared_ptr<TextButton> playButton_;
    std::shared_ptr<TextButton> stopButton_;
    std::shared_ptr<TextButton> pauseButton_;
    std::shared_ptr<TextButton> resetLoopButton_;
    
    std::shared_ptr<Label>      posLabel_;
    std::shared_ptr<Slider>     posSlider_;
    
    std::shared_ptr<Label>      aLabel_;
    std::shared_ptr<Slider>     aSlider_;
    
    std::shared_ptr<Label>      bLabel_;
    std::shared_ptr<Slider>     bSlider_;
    
    std::shared_ptr<Label>      rateLabel_;
    std::shared_ptr<Slider>     rateSlider_;
    
    std::shared_ptr<Label>      pitchLabel_;
    std::shared_ptr<Slider>     pitchSlider_;
    
    std::shared_ptr<Label>      volumeLabel_;
    std::shared_ptr<Slider>     volumeSlider_;
    
    std::shared_ptr<Label>      statusLabel_;
    std::shared_ptr<Label>      debugLabel_;
};
