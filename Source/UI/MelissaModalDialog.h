//
//  MelissaModalDialog.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MelissaButtons.h"
#include "MelissaUISettings.h"
#include "MelissaHost.h"

class MelissaModalDialogListener
{
public:
    virtual ~MelissaModalDialogListener() {}
    virtual void onClose() = 0;
};

class MelissaDialog : public juce::Component
{
public:
    MelissaDialog(std::shared_ptr<Component> contentComponent, const juce::String& title, bool closeOnClickingOutside);
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    std::unique_ptr<BackgroundButton> backgroundButton_;
    std::unique_ptr<CloseButton> closeButton_;
    std::unique_ptr<juce::Label> titleLabel_;
    std::shared_ptr<Component> contentComponent_;
    bool closeOnClickingOutside_;
};

class MelissaModalDialog
{
public:
    MelissaModalDialog() {}
    
    static void setParentComponent(juce::Component* parentComponent) { parentComponent_ = parentComponent; }
    static void show(std::shared_ptr<juce::Component> component, const juce::String& title, bool closeOnClickingOutside = true);
    static void close();
    static void resize();
    
private:
    static juce::Component* parentComponent_;
    static std::unique_ptr<MelissaDialog> dialog_;
};
