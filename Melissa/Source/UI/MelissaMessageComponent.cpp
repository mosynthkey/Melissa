//
//  MelissaMessageComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaMessageComponent.h"
#include "MelissaUISettings.h"

class MelissaMessageComponent::MessageLabel : public Component
{
public:
    MessageLabel()
    {
        label_.setFont(MelissaUISettings::getFontSizeMain());
        label_.setJustificationType(Justification::centredBottom);
        addAndMakeVisible(&label_);
    }
    
    void resized() override
    {
        label_.setSize(getWidth(), getHeight() - 4);
    }
    
    void paint(Graphics& g) override
    {
        g.setColour(colour_);
        g.fillRoundedRectangle(0, getHeight() - 2, getWidth(), 2, 1);
    }
    
    void setMessage(const String& message)
    {
        label_.setText(message, dontSendNotification);
    }
    
    void setColour(const Colour& colour)
    {
        colour_ = colour;
        repaint();
    }
    
private:
    Label label_;
    Colour colour_;
};

MelissaMessageComponent MelissaMessageComponent::instance_;

MelissaMessageComponent::MelissaMessageComponent() : assignCounter_(0)
{

}

MelissaMessageComponent::~MelissaMessageComponent()
{
    
}

void MelissaMessageComponent::resized()
{
            /*
    if (label_ == nullptr)
    {
        label_ = std::make_unique<Label>();
        label_->setFont(MelissaUISettings::getFontSizeMain());
        label_->setJustificationType(Justification::centredBottom);
        addAndMakeVisible(label_.get());
    }
    
    if (messageLabels_.size() == 0)
    {
        auto l = std::make_unique<MessageLabel>();
        l->setMessage("Melissa v2.0.0");
        l->setColour(Colour::fromRGB(255, 160, 160).withHue(0.5));
        l->setBounds(0, 0, 200, 30);
        addAndMakeVisible(l.get());
        messageLabels_.emplace_back(std::move(l));
    }
    label_->setBounds(0, 0, getWidth(), getHeight());
                      */
}

void MelissaMessageComponent::timerCallback()
{
    
}

int MelissaMessageComponent::showMessage(const String& message)
{
    /*
    const auto assignedId = assignCounter_;
    label_->setText(message, dontSendNotification);
    messageList_.emplace_back(std::make_pair(assignedId, message));
    
    ++assignCounter_;
    
    return assignedId;
     */
}

void MelissaMessageComponent::hideMessage(int messageId)
{
    bool found = false;
    for (int i = 0; i < messageList_.size(); ++i)
    {
        if (messageList_[i].first == messageId)
        {
            messageList_.erase(messageList_.begin() + i);
            found = true;
            break;
        }
    }
    
    if (!found) return;
    
}
