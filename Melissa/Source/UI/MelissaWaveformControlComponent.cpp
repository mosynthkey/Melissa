//
//  MelissaWaveformControlComponent.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include "MelissaMessageComponent.h"
#include "MelissaUISettings.h"
#include "MelissaUtility.h"
#include "MelissaWaveformControlComponent.h"

class MelissaWaveformControlComponent::WaveformView : public Component,
                                                      public MelissaModelListener,
                                                      public MelissaWaveformMouseEventListener,
                                                      public Timer
{
public:
    WaveformView(MelissaWaveformControlComponent* parent) :
    parent_(parent),
    numOfStrip_(0),
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
        Colour colour(MelissaUISettings::getMainColour());
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
    }
    
    void mouseExit(float xRatio) override
    {
        current_->setVisible(false);
        parent_->hideTimeTooltip();
    }
    
    void mouseMove(float xRatio) override
    {
        mouseMoveOrDrag(xRatio);
    }
    
    void mouseDrag(float xRatio) override
    {
        mouseMoveOrDrag(xRatio);
    }
    
    void mouseMoveOrDrag(float xRatio)
    {
        parent_->showTimeTooltip(xRatio);
        
        current_->setVisible(true);
        size_t strip = static_cast<float>(xRatio * getWidth() / (waveformStripWidth_ + waveformStripInterval_));
        const int32_t height = previewBuffer_[strip] * getHeight();
        const int32_t x = static_cast<int32_t>((waveformStripWidth_ + waveformStripInterval_) * strip);
        current_->setBounds(x, getHeight() - height, waveformStripWidth_, height);
    }
    
    void timerCallback() override
    {
        stopTimer();
        update_();
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
        auto dataSource = MelissaDataSource::getInstance();
        const size_t bufferLength = dataSource->getBufferLength();
        
        if (numOfStrip_ <= 0 || bufferLength == 0 || previewBuffer_.size() == 0) return;
        
        float preview, previewMax = 0.f;
        for (int32_t iStrip = 0; iStrip < numOfStrip_; ++iStrip)
        {
            preview = 0.f;
            for (int32_t iBuffer = 0; iBuffer < bufferLength / numOfStrip_; ++iBuffer)
            {
                const size_t bufIndex = iStrip * (bufferLength / numOfStrip_) + iBuffer;
                if (bufIndex >= bufferLength) break;
                const auto l = dataSource->readBuffer(0, bufIndex);
                const auto r = dataSource->readBuffer(1, bufIndex);
                preview += (l * l + r * r);
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
    int32_t clickedStripIndex_, loopAStripIndex_, loopBStripIndex_;
    int32_t currentMouseOnStripIndex_;
    float playingPosRatio_, loopAPosRatio_, loopBPosRatio_;
    std::vector<float> previewBuffer_;
};

class MelissaWaveformControlComponent::Marker : public Button
{
public:
    Marker() : Button(""), isMouseOn_(false) {}
    
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        constexpr float headHeight = 20;
        constexpr float bodyWidth = 3;
        
        g.setColour(colour_);
        g.fillRoundedRectangle(0.f, 0.f, static_cast<float>(getWidth()), headHeight, 4);
        g.fillRect((getWidth() - bodyWidth) / 2.f, headHeight - 2, bodyWidth, getHeight() - headHeight + 2);
        
        if (isMouseOn_)
        {
            g.setColour(Colours::black.withAlpha(0.3f));
            g.fillRoundedRectangle(0.f, 0.f, static_cast<float>(getWidth()), headHeight, 4);
            g.fillRect((getWidth() - bodyWidth) / 2.f, headHeight, bodyWidth, getHeight() - headHeight);
        }
    }
    
    std::function<void(bool)> onClick_;
    
    void clicked(const ModifierKeys& modifiers) override
    {
        if (onClick_ != nullptr) onClick_(modifiers.isShiftDown());
    }
    
    void mouseEnter(const MouseEvent& e) override
    {
        isMouseOn_ = true;
        MelissaMessageComponent::getInstance()->showMessage(memo_);
        repaint();
    }
    
    void mouseExit(const MouseEvent& e) override
    {
        isMouseOn_ = false;
        MelissaMessageComponent::getInstance()->showMessage("");
        repaint();
    }
    
    void setPosition(float position)
    {
        position_ = position;
    }
    
    float getPosition() const
    {
        return position_;
    }
    
    void setColour(const Colour& colour)
    {
        colour_ = colour;
        repaint();
    }
    
    void setMemo(const String& memo)
    {
        memo_ = memo;
    }
    
private:
    float position_;
    Colour colour_;
    String memo_;
    bool isMouseOn_;
};

MelissaWaveformControlComponent::MelissaWaveformControlComponent() :
timeSec_(0)
{
    MelissaDataSource::getInstance()->addListener(this);
    
    waveformView_ = std::make_unique<WaveformView>(this);
    addAndMakeVisible(waveformView_.get());
    
    markerBaseComponent_ = std::make_unique<Component>();
    markerBaseComponent_->setInterceptsMouseClicks(false, true);
    addAndMakeVisible(markerBaseComponent_.get());
    
    loopRangeComponent_ = std::make_unique<MelissaLoopRangeComponent>();
    addAndMakeVisible(loopRangeComponent_.get());
    
    mouseEventComponent_ = std::make_unique<MelissaWaveformMouseEventComponent>();
    mouseEventComponent_->addListener(waveformView_.get());
    mouseEventComponent_->addListener(loopRangeComponent_.get());
    addAndMakeVisible(mouseEventComponent_.get());
    
    posTooltip_ = std::make_unique<Label>();
    posTooltip_->setSize(100, 30);
    posTooltip_->setJustificationType(Justification::centred);
    posTooltip_->setFont(MelissaUISettings::getFontSizeSmall());
    addChildComponent(posTooltip_.get());
}

MelissaWaveformControlComponent::~MelissaWaveformControlComponent() {}

void MelissaWaveformControlComponent::resized()
{
    waveformView_->setBounds(20, 20, getWidth() - 20 * 2, getHeight() - 40);
    markerBaseComponent_->setBounds(0, 0, getWidth(), getHeight());
    
    loopRangeComponent_->setBounds(waveformView_->getBounds());
    mouseEventComponent_->setBounds(waveformView_->getBounds());
    
    posTooltip_->setTopLeftPosition(0, 0);
    
    arrangeMarkers();
    arrangeTimeLabels();
}

void MelissaWaveformControlComponent::setPlayPosition(float ratio)
{
    waveformView_->setPlayPosition(ratio);
}

void MelissaWaveformControlComponent::showTimeTooltip(float posRatio)
{
    posTooltip_->setText(timeSec_ != 0 ? MelissaUtility::getFormattedTimeMSec(timeSec_ * posRatio * 1000) : "-:--.-", dontSendNotification);
    posTooltip_->setSize(MelissaUtility::getStringSize(posTooltip_->getFont(), posTooltip_->getText()).first, 20);
    
    const int32_t x = waveformView_->getX() + waveformView_->getWidth() * posRatio  - posTooltip_->getWidth() / 2;
    if (x != posTooltip_->getX())
    {
        posTooltip_->setTopLeftPosition(x, getHeight() - posTooltip_->getHeight());
        
        for (auto&& l : timeLabels_)
        {
            const auto x0 = posTooltip_->getX();
            const auto x1 = posTooltip_->getRight();
            l->setVisible(x1 < l->getX() || l->getRight() < x0);
        }
        posTooltip_->setVisible(true);
    }
}

void MelissaWaveformControlComponent::hideTimeTooltip()
{
    posTooltip_->setVisible(false);
    for (auto&& l : timeLabels_) l->setVisible(true);
}

void MelissaWaveformControlComponent::songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate)
{
    timeSec_ = static_cast<float>(bufferLength) / sampleRate;
    waveformView_->update(true);
    
    timeLabels_.clear();
    auto createLabel = [this](const String& text)
    {
        auto l = std::make_unique<Label>();
        l->setJustificationType(Justification::centred);
        l->setFont(MelissaUISettings::getFontSizeSmall());
        l->setText(text, dontSendNotification);
        l->setSize(MelissaUtility::getStringSize(l->getFont(), l->getText()).first, 20);
        addAndMakeVisible(l.get());
        return l;
    };
    
    for (size_t minuteIndex = 0; minuteIndex * 60 < timeSec_; ++minuteIndex)
    {
        auto l = createLabel(String::formatted("%d:00", minuteIndex));
        timeLabels_.emplace_back(std::move(l));
    }
    /*
    const int minute = timeSec_ / 60;
    const int second = timeSec_ - minute * 60;
    timeLabels_.emplace_back(createLabel(String::formatted("%d:%02d", minute, second)));
     */
    
    arrangeTimeLabels();
}

void MelissaWaveformControlComponent::markerUpdated()
{
    std::vector<MelissaDataSource::Song::Marker> markers;
    MelissaDataSource::getInstance()->getMarkers(markers);
    markers_.clear();
    
    size_t markerIndex = 0;
    for (auto&& m : markers)
    {
        auto marker = std::make_unique<Marker>();
        marker->setPosition(m.position_);
        marker->setColour(Colour::fromRGB(m.colourR_, m.colourG_, m.colourB_));
        marker->setMemo(m.memo_);
        marker->onClick_ = [&, markerIndex](bool isShiftKeyDown)
        {
            auto model = MelissaModel::getInstance();
            if (isShiftKeyDown)
            {
                const auto markerPosSec  = markers_[markerIndex]->getPosition() * timeSec_;
                const auto startPosRatio = std::clamp<float>((markerPosSec - 3) / timeSec_, 0, 1.f);
                const auto endPosSec     = std::clamp<float>((markerPosSec + 3) / timeSec_, 0, 1.f);
                model->setLoopPosRatio(startPosRatio, endPosSec);
            }
            else
            {
                model->setPlayingPosRatio(markers_[markerIndex]->getPosition());
            }
        };
        
        markerBaseComponent_->addAndMakeVisible(marker.get());
        markers_.emplace_back(std::move(marker));
        ++markerIndex;
    }
    
    arrangeMarkers();
}

void MelissaWaveformControlComponent::arrangeMarkers() const
{
    for (auto&& m : markers_)
    {
        const int x = waveformView_->getX() + waveformView_->getWidth() * m->getPosition();
        m->setBounds(x - 4, 0, 8, getHeight() - 20);
    }
}

void MelissaWaveformControlComponent::arrangeTimeLabels() const
{
    int minuteIndex = 0;
    for (auto&& l : timeLabels_)
    {
        int x = waveformView_->getX() + waveformView_->getWidth() * (minuteIndex * 60.f / timeSec_)  - l->getWidth() / 2;
        /*
        if (minuteIndex == timeLabels_.size() - 1)
        {
            x = waveformView_->getX() + waveformView_->getWidth() - l->getWidth() / 2;
        }
         */
        
        l->setTopLeftPosition(x, getHeight() - l->getHeight());
        ++minuteIndex;
    }
}
