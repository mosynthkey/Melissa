#include "MelissaInputDialog.h"
#include "MelissaSetListComponent.h"

MelissaSetListComponent::MelissaSetListComponent(MelissaHost* host) :
host_(host)
{
    createUI();
}

void MelissaSetListComponent::createUI()
{
    setListComboBox_ = std::make_unique<ComboBox>();
    setListComboBox_->onChange = [&]()
    {
        auto list = getCurrentSongList();
        if (list != nullptr) listBox_->setList(*list);
    };
    addAndMakeVisible(setListComboBox_.get());
    
    newSetListButton_ = std::make_unique<TextButton>();
    newSetListButton_->setButtonText("New");
    newSetListButton_->onClick = [&]()
    {
        auto inputDialog = std::make_shared<MelissaInputDialog>(host_, "Enter a name of a new setlist:", "New setlist", [&](const std::string& text) {
            if (text == "") return;
            host_->createSetlist(text);
            host_->closeModalDialog();
        });
        host_->showModalDialog(std::dynamic_pointer_cast<Component>(inputDialog), "New setlist");
    };
    addAndMakeVisible(newSetListButton_.get());
    
    removeSetListButton_ = std::make_unique<TextButton>();
    removeSetListButton_->setButtonText("Remove");
    removeSetListButton_->onClick = [&]()
    {
        if (NativeMessageBox::showYesNoBox(AlertWindow::WarningIcon, "Remove setlist", "Are you sure?") == 0) return;
        
        const int selectedIndex = setListComboBox_->getSelectedId() - 1;
        data_.remove(selectedIndex);
        update();
        
        int indexToSelect = selectedIndex - 1;
        if (indexToSelect < 0) indexToSelect = 0;
        select(indexToSelect);
    };
    addAndMakeVisible(removeSetListButton_.get());
    
    addToSetListButton_ = std::make_unique<TextButton>();
    addToSetListButton_->setButtonText("Add");
    addToSetListButton_->onClick = [&]()
    {
        fileChooser_ = std::make_unique<FileChooser>("Choose a file to add to this set list...", File::getCurrentWorkingDirectory(), "*.mp3;*.wav;*.m4a", true);
        fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [&] (const FileChooser& chooser)
        {
            auto fileUrl = chooser.getURLResult();
            if (fileUrl.isLocalFile())
            {
                auto filePath = fileUrl.getLocalFile().getFullPathName();
                getCurrentSongList()->add(filePath);
                update();
                
                chooser.getResult().getParentDirectory().setAsCurrentWorkingDirectory();
            }
        });
    };
    addAndMakeVisible(addToSetListButton_.get());
    
    listBox_ = std::make_unique<MelissaFileListBox>(host_);
    addAndMakeVisible(listBox_.get());
}

const Array<var>& MelissaSetListComponent::getData()
{
    return data_;
}

void MelissaSetListComponent::setData(const Array<var>& data)
{
    data_ = data;
    update();
    select(0);
}

void MelissaSetListComponent::update()
{
    // update combobox
    
    const int selectedId = setListComboBox_->getSelectedId();
    
    setListComboBox_->clear();
    for (int iItemId = 0; iItemId < data_.size(); ++iItemId)
    {
        setListComboBox_->addItem(data_[iItemId].getDynamicObject()->getProperty("name").toString(), iItemId + 1);
    }
    
    select(selectedId - 1);
}

void MelissaSetListComponent::select(int index)
{
    if (index < 0 ||  data_.size() < index) return;
    
    setListComboBox_->setSelectedId(index + 1);
    
    auto list = getCurrentSongList();
    if (list != nullptr) listBox_->setList(*list);
}

void MelissaSetListComponent::add(const std::string& name, bool sholdSelect)
{
    auto object = new DynamicObject();
    object->setProperty("name", String(name));
    object->setProperty("songs", Array<var>());
    data_.add(object);
    update();

    if (sholdSelect) select(data_.size() - 1);
}


void MelissaSetListComponent::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int margin = 10;
    const int controlWidth = 80;
    const int controlHeight = 30;
    
    setListComboBox_->setBounds(0, 0, w - (controlWidth + margin) * 2, controlHeight);
    newSetListButton_->setBounds(setListComboBox_->getRight() + margin, 0, controlWidth, controlHeight);
    removeSetListButton_->setBounds(newSetListButton_->getRight() + margin, 0, controlWidth, controlHeight);
    
    listBox_->setBounds(0, 40, w, h - (controlHeight + margin) * 2);
    addToSetListButton_->setBounds(w - controlWidth, h - controlHeight, controlWidth, controlHeight);
}

Array<var>* MelissaSetListComponent::getCurrentSongList()
{
    const int selectedIndex = setListComboBox_->getSelectedId() - 1;
    
    if (0 <= selectedIndex && selectedIndex < data_.size())
    {
        return data_[selectedIndex].getDynamicObject()->getProperty("songs").getArray();
    }
    
    return nullptr;
}
