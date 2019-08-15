#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"

class MelissaWaveformControlComponent::TimeLineBar : public Component
{
public:
    TimeLineBar(MelissaWaveformControlComponent* parent) :
    parent_(parent)
    {};
    
    void paint(Graphics& g) override
    {
        g.fillAll(Colour(0x50ffffff));
    }
    
private:
    MelissaWaveformControlComponent* parent_;
};

class MelissaWaveformControlComponent::WaveformView : public Component
{
public:
    WaveformView(MelissaWaveformControlComponent* parent) :
    parent_(parent),
    numOfStrip_(1),
    isMouseDown_(false),
    clickedStripIndex_(-1), loopAStripIndex_(-1), loopBStripIndex_(-1),
    listener_(nullptr)
    {
        current_ = std::make_unique<Label>();
        current_->setColour(Label::backgroundColourId, Colour(0x80ffffff));
        current_->setInterceptsMouseClicks(false, false);
        addAndMakeVisible(current_.get());
    };
    
    void resized() override
    {
        numOfStrip_ = static_cast<float>(getWidth() / (waveformStripWidth_ + waveformStripInterval_));
        previewBuffer_.resize(numOfStrip_);
    }
    
    void paint(Graphics& g) override
    {
        g.setColour(Colour(0x50ffffff));
        for (size_t iStrip = 0; iStrip < numOfStrip_; ++iStrip)
        {
            const int32_t height = previewBuffer_[iStrip] * getHeight();
            const int32_t x = static_cast<int32_t>((waveformStripWidth_ + waveformStripInterval_) * iStrip);
            
            auto color = Colour(0x40ffffff);
            if (loopAStripIndex_ != -1 && loopAStripIndex_ <= iStrip && iStrip <= loopBStripIndex_)
            {
                color = Colour(0xbb50E3C2);
            }
            g.setColour(color);
            g.fillRect(x, getHeight() - height, waveformStripWidth_, height);
        }
    }
    
    void mouseMoveOrDrag(const MouseEvent &event)
    {
        auto pos = event.getPosition();
        parent_->showTimeTooltip(static_cast<float>(pos.getX()) / getWidth());
        
        current_->setVisible(true);
        size_t strip = static_cast<float>(pos.getX() / (waveformStripWidth_ + waveformStripInterval_));
        const int32_t height = previewBuffer_[strip] * getHeight();
        const int32_t x = static_cast<int32_t>((waveformStripWidth_ + waveformStripInterval_) * strip);
        current_->setBounds(x, getHeight() - height, waveformStripWidth_, height);
        
        repaint();
    }
    
    void mouseMove(const MouseEvent &event) override
    {
        mouseMoveOrDrag(event);
    }
    
    void mouseDrag(const MouseEvent &event) override
    {
        mouseMoveOrDrag(event);
    }
    
    void mouseDown(const MouseEvent &event) override
    {
        isMouseDown_ = true;
        clickedStripIndex_ = getStripIndexOnX(static_cast<float>(event.getPosition().getX()));
    }
    
    void mouseUp(const MouseEvent &event) override
    {
        auto distX = abs(event.getMouseDownX() - event.x);
        auto distY = abs(event.getMouseDownY() - event.y);
        if (isMouseDown_)
        {
            if (distX < 4 && distY < 4)
            {
                std::cout << "Click" << std::endl;
                clickedStripIndex_ = -1;
                if (listener_ != nullptr) listener_->setPlayPosition(parent_, static_cast<float>(event.getPosition().getX()) / getWidth());
            }
            else
            {
                std::cout << "Drag" << std::endl;
                loopAStripIndex_ = clickedStripIndex_;
                auto stripIndex = getStripIndexOnX(static_cast<float>(event.getPosition().getX()));
                if (stripIndex < loopAStripIndex_)
                {
                    loopBStripIndex_ = loopAStripIndex_;
                    loopAStripIndex_ = stripIndex;
                }
                else
                {
                    loopBStripIndex_ = stripIndex;
                }
                if (listener_ != nullptr)
                {
                    listener_->setAPosition(parent_, static_cast<float>(loopAStripIndex_) / numOfStrip_);
                    listener_->setBPosition(parent_, static_cast<float>(loopBStripIndex_) / numOfStrip_);
                }
            }
        }
        isMouseDown_ = false;
    }
    
    void setBuffer(const float* buffer[], size_t bufferLength)
    {
        if (numOfStrip_ <= 0) return;
        
        float preview, previewMax = 0.f;
        for (int32_t iStrip = 0; iStrip < numOfStrip_; ++iStrip)
        {
            preview = 0.f;
            for (int32_t iBuffer = 0; iBuffer < bufferLength / numOfStrip_; ++iBuffer)
            {
                const size_t bufIndex = iStrip * (bufferLength / numOfStrip_) + iBuffer;
                if (bufIndex >= bufferLength) break;
                preview += (abs(buffer[0][bufIndex]) + abs(buffer[1][bufIndex])) / 2.f;
            }
            preview /= (bufferLength / numOfStrip_);
            if (preview >= 1.f) preview = 1.f;
            if (previewMax < preview) previewMax = preview;
            previewBuffer_[iStrip] = preview;
        }
        
        // normalize
        for (int iPreviewBuffer = 0; iPreviewBuffer < previewBuffer_.size(); ++iPreviewBuffer)
        {
            previewBuffer_[iPreviewBuffer] /= previewMax;
        }
        loopAStripIndex_ = -1;
        
        repaint();
    }
    
    void setListener(MelissaWaveformControlListener* listener)
    {
        listener_ = listener;
    }
    
private:
    int32_t getStripIndexOnX(float x)
    {
        return x / (waveformStripWidth_ + waveformStripInterval_);
    }
    
    MelissaWaveformControlComponent* parent_;
    std::shared_ptr<Label> current_;
    const int32_t waveformStripWidth_ = 2, waveformStripInterval_ = 1;
    size_t numOfStrip_;
    bool isMouseDown_;
    int32_t clickedStripIndex_, loopAStripIndex_, loopBStripIndex_;
    MelissaWaveformControlListener* listener_;
    std::vector<float> previewBuffer_;
};

MelissaWaveformControlComponent::MelissaWaveformControlComponent() :
timeSec_(0)
{
    timeLineBar_ = std::make_unique<TimeLineBar>(this);
    addAndMakeVisible(timeLineBar_.get());
    
    waveformView_ = std::make_unique<WaveformView>(this);
    addAndMakeVisible(waveformView_.get());
    
    aLabel_ = std::make_unique<Label>();
    aLabel_->setSize(60, 20);
    aLabel_->setText("0:00", dontSendNotification);
    aLabel_->setJustificationType(Justification::centredLeft);
    aLabel_->setFont(Font(18));
    aLabel_->setColour(Label::textColourId, Colour(0x80ffffff));
    addAndMakeVisible(aLabel_.get());
    
    bLabel_ = std::make_unique<Label>();
    bLabel_->setSize(60, 18);
    bLabel_->setText("-:--", dontSendNotification);
    bLabel_->setJustificationType(Justification::centredRight);
    bLabel_->setFont(Font(18));
    bLabel_->setColour(Label::textColourId, Colour(0x80ffffff));
    addAndMakeVisible(bLabel_.get());
    
    posTooltip_ = std::make_unique<MelissaLabel>();
    posTooltip_->setSize(100, 28);
    posTooltip_->setText("2:48");
    addAndMakeVisible(posTooltip_.get());
    
    startTimer(100);
}

MelissaWaveformControlComponent::~MelissaWaveformControlComponent()
{
    
}

void MelissaWaveformControlComponent::resized()
{
    timeLineBar_->setBounds(50, getHeight() - 18, getWidth() - 50 * 2, 2);
    waveformView_->setBounds(50, 20, getWidth() - 50 * 2, getHeight() - 20 - 30);
    aLabel_->setTopLeftPosition(0, getHeight() - 20);
    bLabel_->setTopRightPosition(getWidth(), getHeight() - 20);
    
    posTooltip_->setTopLeftPosition(0, 0);
}

void MelissaWaveformControlComponent::setListener(MelissaWaveformControlListener* listener)
{
    listener_ = listener;
    waveformView_->setListener(listener);
}

void MelissaWaveformControlComponent::timerCallback()
{
    posTooltip_->setVisible(false);
}

void MelissaWaveformControlComponent::setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate)
{
    waveformView_->setBuffer(buffer, bufferLength);
    
    timeSec_ = static_cast<float>(bufferLength) / sampleRate;
    bLabel_->setText(MelissaUtility::getFormattedTimeSec(timeSec_), dontSendNotification);
}

void MelissaWaveformControlComponent::showTimeTooltip(float posRatio)
{
    posTooltip_->setText(timeSec_ != 0 ? MelissaUtility::getFormattedTimeMSec(timeSec_ * posRatio * 1000) : "-:--.----");
    
    const int32_t x = waveformView_->getX() + waveformView_->getWidth() * posRatio;
    if (x != posTooltip_->getX())
    {
        posTooltip_->setCentrePosition(x, posTooltip_->getY() + posTooltip_->getHeight() / 2);
    
        startTimer(2000);
        posTooltip_->setVisible(true);
    }
}
