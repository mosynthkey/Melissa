#include "MelissaUISettings.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"

class MelissaWaveformControlComponent::WaveformView : public Component,
                                                      public MelissaModelListener,
                                                      public Timer
{
public:
    WaveformView(MelissaWaveformControlComponent* parent) :
    parent_(parent),
    numOfStrip_(0),
    isMouseDown_(false),
    clickedStripIndex_(-1), loopAStripIndex_(-1), loopBStripIndex_(-1),
    currentMouseOnStripIndex_(-1),
    playingPosRatio_(-1.f), loopAPosRatio_(0.f), loopBPosRatio_(1.f)
    {
        MelissaModel::getInstance()->addListener(this);
        
        current_ = std::make_unique<Label>();
        current_->setColour(Label::backgroundColourId, Colour(0x80ffffff));
        current_->setInterceptsMouseClicks(false, false);
        addAndMakeVisible(current_.get());
    };
    
    void resized() override
    {
        update();
    }
    
    void paint(Graphics& g) override
    {
        Colour colour(MelissaUISettings::MainColour());
        for (size_t iStrip = 0; iStrip < numOfStrip_; ++iStrip)
        {
            const int32_t height = previewBuffer_[iStrip] * getHeight();
            const int32_t x = static_cast<int32_t>((waveformStripWidth_ + waveformStripInterval_) * iStrip);
            
            if (iStrip == static_cast<size_t>(playingPosRatio_ * numOfStrip_))
            {
                g.setColour(colour.withAlpha(1.f));
            }
            else if (loopAStripIndex_ != -1 && loopAStripIndex_ <= iStrip && iStrip <= loopBStripIndex_)
            {
                g.setColour(colour.withAlpha(0.6f));
            }
            else
            {
                g.setColour(colour.withAlpha(0.2f));
            }
            g.fillRect(x, getHeight() - height, waveformStripWidth_, height);
        }
        
        if (isMouseDown_)
        {
            const int32_t highlightAStripIndex = std::min(currentMouseOnStripIndex_, clickedStripIndex_);
            const int32_t highlightBStripIndex = std::max(currentMouseOnStripIndex_, clickedStripIndex_);
            for (size_t iStrip = highlightAStripIndex; iStrip < highlightBStripIndex && iStrip < previewBuffer_.size(); ++iStrip)
            {
                const int32_t height = previewBuffer_[iStrip] * getHeight();
                const int32_t x = static_cast<int32_t>((waveformStripWidth_ + waveformStripInterval_) * iStrip);
                
                g.setColour(colour.withAlpha(0.7f));
                g.fillRect(x, getHeight() - height, waveformStripWidth_, height);
            }
        }
    }
    
    void mouseMoveOrDrag(const MouseEvent& event)
    {
        auto pos = event.getPosition();
        parent_->showTimeTooltip(static_cast<float>(pos.getX()) / getWidth());
        
        current_->setVisible(true);
        size_t strip = static_cast<float>(pos.getX() / (waveformStripWidth_ + waveformStripInterval_));
        const int32_t height = previewBuffer_[strip] * getHeight();
        const int32_t x = static_cast<int32_t>((waveformStripWidth_ + waveformStripInterval_) * strip);
        current_->setBounds(x, getHeight() - height, waveformStripWidth_, height);
    }
    
    void mouseMove(const MouseEvent& event) override
    {
        mouseMoveOrDrag(event);
    }
    
    void mouseDrag(const MouseEvent& event) override
    {
        mouseMoveOrDrag(event);
        currentMouseOnStripIndex_ = getStripIndexOnX(static_cast<float>(event.getPosition().getX()));
        repaint();
    }
    
    void mouseDown(const MouseEvent& event) override
    {
        isMouseDown_ = true;
        clickedStripIndex_ = currentMouseOnStripIndex_ = getStripIndexOnX(static_cast<float>(event.getPosition().getX()));
    }
    
    void mouseUp(const MouseEvent& event) override
    {
        auto distX = abs(event.getMouseDownX() - event.x);
        auto distY = abs(event.getMouseDownY() - event.y);
        if (isMouseDown_)
        {
            if (distX < 4 && distY < 4)
            {
                // Click
                clickedStripIndex_ = -1;
                
                MelissaModel::getInstance()->setPlayingPosRatio(static_cast<float>(event.getPosition().getX()) / getWidth());
            }
            else
            {
                // Drag
                loopAStripIndex_ = clickedStripIndex_;
                auto stripIndex = getStripIndexOnX(static_cast<float>(event.getPosition().getX()));
                if (stripIndex < 0) stripIndex = 0;
                if (stripIndex < loopAStripIndex_)
                {
                    loopBStripIndex_ = loopAStripIndex_;
                    loopAStripIndex_ = stripIndex;
                }
                else
                {
                    loopBStripIndex_ = stripIndex;
                }
                
                float aRatio = static_cast<float>(loopAStripIndex_) / numOfStrip_;
                if (1.f < aRatio) aRatio = 1.f;
                float bRatio = static_cast<float>(loopBStripIndex_) / numOfStrip_;
                if (1.f < bRatio) bRatio = 1.f;
                MelissaModel::getInstance()->setLoopPosRatio(aRatio, bRatio);
            }
        }
        isMouseDown_ = false;
        currentMouseOnStripIndex_ = -1;
    }
    
    void mouseExit(const MouseEvent& event) override
    {
        current_->setVisible(false);
    }
    
    void timerCallback() override
    {
        stopTimer();
        update_();
    }
    
    void setBuffer(const float* buffer[], size_t bufferLength)
    {
        originalMonoralBuffer_.resize(bufferLength);
        for (size_t iBufIndex = 0; iBufIndex < bufferLength; ++iBufIndex)
        {
            originalMonoralBuffer_[iBufIndex] = ((buffer[0][iBufIndex] * buffer[0][iBufIndex]) + (buffer[1][iBufIndex] * buffer[1][iBufIndex])) / 2.f;
        }
        
        update(true);
    }
    
    void update(bool immediately = false)
    {
        stopTimer();
        
        numOfStrip_ = static_cast<float>(getWidth() / (waveformStripWidth_ + waveformStripInterval_));
        previewBuffer_.resize(numOfStrip_);
        loopAStripIndex_ = loopAPosRatio_ * numOfStrip_;
        loopBStripIndex_ = loopBPosRatio_ * numOfStrip_;
        
        if (immediately)
        {
            stopTimer();
            update_();
        }
        else
        {
            startTimer(1000); // 1 sec delay
        }
    }
    
    void update_()
    {
        if (numOfStrip_ <= 0 || originalMonoralBuffer_.size() == 0 || previewBuffer_.size() == 0) return;
        
        const size_t bufferLength = originalMonoralBuffer_.size();
        
        float preview, previewMax = 0.f;
        for (int32_t iStrip = 0; iStrip < numOfStrip_; ++iStrip)
        {
            preview = 0.f;
            for (int32_t iBuffer = 0; iBuffer < bufferLength / numOfStrip_; ++iBuffer)
            {
                const size_t bufIndex = iStrip * (bufferLength / numOfStrip_) + iBuffer;
                if (bufIndex >= bufferLength) break;
                preview += originalMonoralBuffer_[bufIndex];
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
        
        repaint();
    }
    
    void setPlayPosition(float ratio)
    {
        if (playingPosRatio_ == ratio) return;
        playingPosRatio_ = ratio;
        repaint();
    }
    
    void setAPosition(float ratio)
    {
        loopAPosRatio_ = ratio;
        loopAStripIndex_ = ratio * numOfStrip_;
        repaint();
    }
    
    void setBPosition(float ratio)
    {
        loopBPosRatio_ = ratio;
        loopBStripIndex_ = ratio * numOfStrip_;
        repaint();
    }
    
private:
    int32_t getStripIndexOnX(float x)
    {
        return x / (waveformStripWidth_ + waveformStripInterval_);
    }
    
    // MelissaModelListener
    void loopPosChanged(float aTimeMSec, float aRatio, float bTimeMSec, float bRatio) override
    {
        setAPosition(aRatio);
        setBPosition(bRatio);
    }
    
    MelissaWaveformControlComponent* parent_;
    std::shared_ptr<Label> current_;
    const int32_t waveformStripWidth_ = 3, waveformStripInterval_ = 1;
    size_t numOfStrip_;
    bool isMouseDown_;
    int32_t clickedStripIndex_, loopAStripIndex_, loopBStripIndex_;
    int32_t currentMouseOnStripIndex_;
    float playingPosRatio_, loopAPosRatio_, loopBPosRatio_;
    std::vector<float> previewBuffer_;
    std::vector<float> originalMonoralBuffer_;
};

MelissaWaveformControlComponent::MelissaWaveformControlComponent() :
timeSec_(0)
{
    waveformView_ = std::make_unique<WaveformView>(this);
    addAndMakeVisible(waveformView_.get());
    
    posTooltip_ = std::make_unique<MelissaLabel>();
    posTooltip_->setSize(100, 28);
    addChildComponent(posTooltip_.get());
    
    startTimer(100);
}

MelissaWaveformControlComponent::~MelissaWaveformControlComponent()
{
    
}

void MelissaWaveformControlComponent::resized()
{
    waveformView_->setBounds(60, 20, getWidth() - 60 * 2, getHeight() - 40);
    
    posTooltip_->setTopLeftPosition(0, 0);
}

void MelissaWaveformControlComponent::timerCallback()
{
    posTooltip_->setVisible(false);
}

void MelissaWaveformControlComponent::setBuffer(const float* buffer[], size_t bufferLength, int32_t sampleRate)
{
    waveformView_->setBuffer(buffer, bufferLength);
    timeSec_ = static_cast<float>(bufferLength) / sampleRate;
}

void MelissaWaveformControlComponent::setPlayPosition(float ratio)
{
    waveformView_->setPlayPosition(ratio);
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
