//
//  MelissaMarkerListBox.cpp
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#include <numeric>
#include "MelissaDoubleClickEditLabel.h"
#include "MelissaMarkerListBox.h"

using namespace juce;

class MarkerColourLabel : public Component
{
public:
    MarkerColourLabel() : colour_(Colours::white) {}
    
    void setColour(Colour colour)
    {
        colour_ = colour;
        repaint();
    }
    
    void paint(Graphics& g) override
    {
        /*
        const auto size = jmin(getWidth(), getHeight()) - 4;
        
        g.setColour(colour_);
        auto rect = Rectangle<int>((getWidth() - size) / 2, (getHeight() - size) / 2, size, size);
        g.fillRect(rect);
        
        g.setColour(Colours::black.withAlpha(0.2f));
        g.drawRect(rect);
         */
        
        const auto rect = getLocalBounds().reduced(10, 4).toFloat();
        g.setColour(colour_);
        g.fillRoundedRectangle(rect, rect.getHeight() / 2);
    }
    
private:
    Colour colour_;
};

MelissaMarkerListBox::MelissaMarkerListBox(const String& componentName) :
TableListBox(componentName, this),
dataSource_(MelissaDataSource::getInstance())
{
    dataSource_->addListener(this);
    
    String headerTitles[kNumOfColumn] = { "Time", "Color", "Memo" };
    for (int i = 0; i < kNumOfColumn; ++i)
    {
        getHeader().addColumn(headerTitles[i], i + 1, 50);
    }
    setOutlineThickness(1);
    
    setLookAndFeel(&laf_);
    
    popupMenu_ = std::make_shared<PopupMenu>();
    popupMenu_->setLookAndFeel(&laf_);
}

MelissaMarkerListBox::~MelissaMarkerListBox()
{
    popupMenu_->setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void MelissaMarkerListBox::updateMarkers()
{
    dataSource_->getMarkers(markers_);
    updateContent();
    repaint();
}

void MelissaMarkerListBox::resized()
{
    TableListBox::resized();
    autoSizeAllColumns();
}

int MelissaMarkerListBox::getNumRows()
{
    return static_cast<int>(markers_.size());
}

void MelissaMarkerListBox::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
    {
        g.fillAll(MelissaUISettings::getAccentColour(0.4f));
    }
}

void MelissaMarkerListBox::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    String text = "";
    if (rowNumber < markers_.size())
    {
        auto marker = markers_[rowNumber];
        switch (columnId)
        {
            case kColumn_Time + 1:
            {
                text = MelissaUtility::getFormattedTimeSec(marker.position_ * totalLengthMSec_ / 1000.f);
                break;
            }
            case kColumn_Colour + 1:
            {
                text = "Colour";
                break;
            }
            default:
            {
                return;
            }
        }
    }
    
    g.setColour(MelissaUISettings::getTextColour());
    g.setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Main));
    constexpr int xMargin = 10;
    g.drawText(text, xMargin, 0, width - xMargin * 2, height, Justification::left);
}

Component* MelissaMarkerListBox::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    if (markers_.size() <= rowNumber) return nullptr;
    
    if (columnId == kColumn_Colour + 1)
    {
        auto marker = markers_[rowNumber];
        auto colour = Colour::fromRGB(marker.colourR_, marker.colourG_, marker.colourB_);
        if (existingComponentToUpdate == nullptr)
        {
            auto l = new MarkerColourLabel();
            l->setColour(colour);
            l->setInterceptsMouseClicks(false, true);
            return dynamic_cast<Component*>(l);
        }
        else
        {
            auto component = dynamic_cast<MarkerColourLabel*>(existingComponentToUpdate);
            component->setColour(colour);
            return existingComponentToUpdate;
        }
    }
    else if (columnId == kColumn_Memo + 1)
    {
        auto marker = markers_[rowNumber];
        if (existingComponentToUpdate == nullptr)
        {
            auto l = new MelissaDoubleClickEditLabel(this, rowNumber, columnId);
            l->setText(marker.memo_, dontSendNotification);
            l->setColour(Label::textColourId, MelissaUISettings::getTextColour());
            l->setFont(MelissaDataSource::getInstance()->getFont(MelissaDataSource::Global::kFontSize_Sub));
            l->setRowNumber(rowNumber);
            l->addListener(this);
            return dynamic_cast<Component*>(l);
        }
        else
        {
            auto l = dynamic_cast<MelissaDoubleClickEditLabel*>(existingComponentToUpdate);
            l->setText(marker.memo_, dontSendNotification);
            l->setRowNumber(rowNumber);
            return existingComponentToUpdate;
        }
    }
    
    return nullptr;
}

int MelissaMarkerListBox::getColumnAutoSizeWidth(int columnId)
{
    if (columnId == kColumn_Time + 1)
    {
        return 80;
    }
    else if (columnId == kColumn_Colour + 1)
    {
        return 100;
    }
    else
    {
        return getWidth() - 100 - 80 - 2;
    }
}

void MelissaMarkerListBox::cellClicked(int rowNumber, int columnId, const MouseEvent& e)
{
    selectRow(rowNumber);
    if (e.mods.isRightButtonDown())
    {
        enum
        {
            kMenuId_Remove = 1,
        };
        popupMenu_->clear();
        popupMenu_->addItem(kMenuId_Remove, TRANS("remove"), true);
        
        popupMenu_->showMenuAsync(PopupMenu::Options(), [&, rowNumber](int result) {
            if (result == kMenuId_Remove)
            {
                dataSource_->removeMarker(rowNumber);
            }
        });
    }
}

void MelissaMarkerListBox::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent& e)
{
}

void MelissaMarkerListBox::labelTextChanged(Label* label)
{
    auto l = dynamic_cast<MelissaDoubleClickEditLabel*>(label);
    const int rowIndex = l->getRowNumber();
    if (markers_.size() <= rowIndex) return;
    markers_[rowIndex].memo_ = label->getText();
    dataSource_->overwriteMarker(rowIndex, markers_[rowIndex]);
}

void MelissaMarkerListBox::songChanged(const String& filePath, size_t bufferLength, int32_t sampleRate)
{
    totalLengthMSec_ = static_cast<float>(bufferLength) / sampleRate * 1000.f;
    updateMarkers();
}

void MelissaMarkerListBox::markerUpdated()
{
    updateMarkers();
}
