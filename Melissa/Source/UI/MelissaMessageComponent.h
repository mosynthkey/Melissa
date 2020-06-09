//
//  MelissaUISettings.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"

class MelissaMessageComponent : public Component,
                                public Timer
{
public:
    // Singleton
    static MelissaMessageComponent* getInstance() { return &instance_; }
    MelissaMessageComponent(const MelissaMessageComponent&) = delete;
    MelissaMessageComponent& operator=(const MelissaMessageComponent&) = delete;
    MelissaMessageComponent(MelissaMessageComponent&&) = delete;
    MelissaMessageComponent& operator=(MelissaMessageComponent&&) = delete;
    
    // Component
    void resized() override;
    
    // Timer
    void timerCallback() override;
    
    int showMessage(const String& text);
    void hideMessage(int messageId);
    
private:
    // Singleton
    MelissaMessageComponent();
    ~MelissaMessageComponent();
    static  MelissaMessageComponent instance_;
    
    std::unique_ptr<Label> label_;
    
    int assignCounter_;
    std::vector<std::pair<int, String>> messageList_;
    
    class MessageLabel;
    std::vector<std::unique_ptr<MessageLabel>> messageLabels_;
};
