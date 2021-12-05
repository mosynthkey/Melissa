//
//  MelissaPracticeTableListBox.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <numeric>
#include "MelissaDoubleClickEditLabel.h"
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
        
        g.setColour(MelissaUISettings::getAccentColour(0.6f));
        g.fillRoundedRectangle(xMargin, (h - lineWidth) / 2.f, w - xMargin * 2, lineWidth, lineWidth / 2);
        g.setColour(MelissaUISettings::getAccentColour());
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
    setOutlineThickness(1);
    
    setLookAndFeel(&laf_);
    
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
    if (rowIsSelected)
    {
        g.fillAll(MelissaUISettings::getSubColour());
    }
}

void MelissaPracticeTableListBox::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
}

Component* MelissaPracticeTableListBox::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    if (practiceList_.size() <= rowNumber) return nullptr;
    
    if (columnId == kColumn_Name + 1)
    {
        auto prac = practiceList_[rowNumber];
        if (existingComponentToUpdate == nullptr)
        {
            auto l = new MelissaDoubleClickEditLabel(this, rowNumber, columnId);
            l->setName("name");
            l->setComponentID(String(rowNumber));
            l->setText(prac.name_, dontSendNotification);
            l->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
            l->setColour(Label::textColourId, MelissaUISettings::getTextColour());
            l->addListener(this);
            return dynamic_cast<Component*>(l);
        }
        else
        {
            auto l = dynamic_cast<MelissaDoubleClickEditLabel*>(existingComponentToUpdate);
            l->setText(prac.name_, dontSendNotification);
            return existingComponentToUpdate;
        }
    }
    else if (columnId == kColumn_LoopRange + 1)
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
    else if (columnId == kColumn_Speed + 1)
    {
        auto prac = practiceList_[rowNumber];
        if (existingComponentToUpdate == nullptr)
        {
            auto l = new MelissaDoubleClickEditLabel(this, rowNumber, columnId);
            l->setName("speed");
            l->setComponentID(String(rowNumber));
            l->setText(String(prac.speed_) + " %",  dontSendNotification);
            l->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
            l->setColour(Label::textColourId, MelissaUISettings::getTextColour());
            l->addListener(this);
            return dynamic_cast<Component*>(l);
        }
        else
        {
            auto l = dynamic_cast<MelissaDoubleClickEditLabel*>(existingComponentToUpdate);
            l->setText(String(prac.speed_) + "%", dontSendNotification);
            return existingComponentToUpdate;
        }
    }
        
    
    return nullptr;
}

int MelissaPracticeTableListBox::getColumnAutoSizeWidth(int columnId)
{
    const std::vector<int> widthRatio = { 3, 5, 3 };
    const float sum = std::accumulate(widthRatio.begin(), widthRatio.end(), 0);
    return widthRatio[columnId - 1] / sum * (getWidth() - 2);
}

void MelissaPracticeTableListBox::cellClicked(int rowNumber, int columnId, const MouseEvent& e)
{
    selectRow(rowNumber);
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
        
        popupMenu_->showMenuAsync(PopupMenu::Options(), [&, rowNumber](int result) {
            if (result == kMenuId_Remove)
            {
                dataSource_->removePracticeList(rowNumber);
            }
            else if (result == kMenuId_Overwrite)
            {
                dataSource_->overwritePracticeList(rowNumber, practiceList_[rowNumber].name_);
            }
        });


    }
}

void MelissaPracticeTableListBox::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e)
{
    auto prac = practiceList_[rowNumber];
    auto model = MelissaModel::getInstance();
    
    model->setLoopPosRatio(prac.aRatio_, prac.bRatio_);
    
    model->setSpeed(prac.speed_);
#if defined(ENABLE_SPEED_TRAINING)
    model->setSpeedIncStart(prac.speedIncStart_);
    model->setSpeedIncValue(prac.speedIncValue_);
    model->setSpeedIncPer(prac.speedIncPer_);
    model->setSpeedIncGoal(prac.speedIncGoal_);
    model->setSpeedMode(prac.speedMode_);
#endif
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

void MelissaPracticeTableListBox::labelTextChanged(Label* label)
{
    const int rowIndex = label->getComponentID().getIntValue();
    if (practiceList_.size() <= rowIndex) return;
    
    if (label->getName() == "name")
    {
        practiceList_[rowIndex].name_ = label->getText();
    }
    else if (label->getName() == "speed")
    {
        const auto speed = label->getText().getIntValue();
        if (kSpeedMin <= speed && speed <= kSpeedMax) practiceList_[rowIndex].speed_ = speed;
    }
    dataSource_->overwritePracticeList(rowIndex, practiceList_[rowIndex]);
}
