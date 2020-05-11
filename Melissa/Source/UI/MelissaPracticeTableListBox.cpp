//
//  MelissaPracticeTableListBox.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <numeric>
#include "MelissaPracticeTableListBox.h"

class LoopRangeComponent : public Component
{
public:
    LoopRangeComponent(float aRatio, float bRatio) : aRatio_(aRatio), bRatio_(bRatio)
    {
        setInterceptsMouseClicks(false, false);
    };
    
    void paint(Graphics& g)
    {
        constexpr float lineWidth = 8.f;
        constexpr float xMargin = 10.f;
        const auto w = getWidth();
        const auto h = getHeight();
        const float aX = (w - lineWidth - xMargin * 2) * aRatio_ + xMargin;
        const float bX = (w - lineWidth - xMargin * 2) * bRatio_ + xMargin + lineWidth;
        
        g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(0.1f));
        g.fillRoundedRectangle(xMargin, (h - lineWidth) / 2.f, w - xMargin * 2, lineWidth, lineWidth / 2);
        g.setColour(Colour(MelissaUISettings::getMainColour()).withAlpha(0.4f));
        g.fillRoundedRectangle(aX,      (h - lineWidth) / 2.f, bX - aX,         lineWidth, lineWidth / 2);
    }
    
    void setLoopRangeRatio(float aRatio, float bRatio)
    {
        if (0 <= aRatio && aRatio < bRatio && bRatio <= 1.f)
        {
            aRatio_ = aRatio;
            bRatio_ = bRatio;
        }
        repaint();
    }
    
private:
    float aRatio_, bRatio_;
};

MelissaPracticeTableListBox::MelissaPracticeTableListBox(const String& componentName) :
TableListBox(componentName, this),
dataSource_(MelissaDataSource::getInstance())
{
    dataSource_->addListener(this);
    
    String headerTitles[kNumOfColumn] = { "Name", "Loop range", "Speed" };
    for (int i = 0; i < kNumOfColumn; ++i)
    {
        getHeader().addColumn(headerTitles[i], i + 1, 50);
    }
    
    popupMenu_ = std::make_shared<PopupMenu>();
    popupMenu_->setLookAndFeel(&laf_);
}

MelissaPracticeTableListBox::~MelissaPracticeTableListBox()
{
    popupMenu_->setLookAndFeel(nullptr);
}

void MelissaPracticeTableListBox::updatePracticeList()
{
    dataSource_->getPracticeList(practiceList_);
    updateContent();
    repaint();
}

void MelissaPracticeTableListBox::resized()
{
    TableListBox::resized();
    autoSizeAllColumns();
}

int MelissaPracticeTableListBox::getNumRows()
{
    return static_cast<int>(practiceList_.size());
}

void MelissaPracticeTableListBox::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    const auto colour = Colour(MelissaUISettings::getMainColour()).withAlpha(rowIsSelected ? 0.06f : 0.f);
    g.fillAll(colour);
}

void MelissaPracticeTableListBox::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    String text = "";
    if (rowNumber < practiceList_.size())
    {
        auto prac = practiceList_[rowNumber];
        switch (columnId)
        {
            case kColumn_Name + 1:
            {
                text = prac.name_;
                break;
            }
            case kColumn_Speed + 1:
            {
                text = String(prac.speed_) + " %";
                break;
            }
            default:
            {
                return;
            }
        }
    }
    
    g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.8f));
    g.setFont(MelissaUISettings::getFontSizeMain());
    constexpr int xMargin = 10;
    g.drawText(text, xMargin, 0, width - xMargin * 2, height, Justification::left);
}

Component* MelissaPracticeTableListBox::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    if (rowNumber < practiceList_.size() && columnId == kColumn_LoopRange + 1)
    {
        auto prac = practiceList_[rowNumber];
        if (existingComponentToUpdate == nullptr)
        {
            return dynamic_cast<Component*>(new LoopRangeComponent(prac.aRatio_, prac.bRatio_));
        }
        else
        {
            auto component = dynamic_cast<LoopRangeComponent*>(existingComponentToUpdate);
            component->setLoopRangeRatio(prac.aRatio_, prac.bRatio_);
            return existingComponentToUpdate;
        }
    }
    
    return nullptr;
}

int MelissaPracticeTableListBox::getColumnAutoSizeWidth(int columnId)
{
    const std::vector<int> widthRatio = { 3, 5, 2 };
    const float sum = std::accumulate(widthRatio.begin(), widthRatio.end(), 0);
    return widthRatio[columnId - 1] / sum * getWidth();
}

void MelissaPracticeTableListBox::cellClicked(int rowNumber, int columnId, const MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        enum
        {
            kMenuId_Remove = 1,
            kMenuId_Overwrite,
        };
        popupMenu_->clear();
        popupMenu_->addItem(kMenuId_Remove, TRANS("remove"), true);
        popupMenu_->addItem(kMenuId_Overwrite, TRANS("overwrite"), true);
        auto result = popupMenu_->show();

        if (result == kMenuId_Remove)
        {
            dataSource_->removePracticeList(rowNumber);
        }
        else if (result == kMenuId_Overwrite)
        {
            auto model = MelissaModel::getInstance();
            const float a = model->getLoopAPosRatio();
            const float b = model->getLoopBPosRatio();
            const int   speed = model->getSpeed();
            const OutputMode output = model->getOutputMode();
            dataSource_->overwritePracticeList(rowNumber, practiceList_[rowNumber].name_);
        }
    }
}

void MelissaPracticeTableListBox::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e)
{
    auto prac = practiceList_[rowNumber];
    auto model = MelissaModel::getInstance();
    model->setLoopPosRatio(prac.aRatio_, prac.bRatio_);
    model->setSpeed(prac.speed_);
    model->setOutputMode(prac.outputMode_);
}

void MelissaPracticeTableListBox::songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate)
{
    totalLengthMSec_ = static_cast<float>(bufferLength) / sampleRate * 1000.f;
    updatePracticeList();
}

void MelissaPracticeTableListBox::practiceListUpdated()
{
    updatePracticeList();
}
