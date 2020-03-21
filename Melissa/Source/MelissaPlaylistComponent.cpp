#include "MelissaInputDialog.h"
#include "MelissaPlaylistComponent.h"

MelissaPlaylistComponent::MelissaPlaylistComponent(MelissaHost* host) :
host_(host)
{
    createUI();
}

void MelissaPlaylistComponent::createUI()
{
    setListComboBox_ = std::make_unique<ComboBox>();
    setListComboBox_->onChange = [&]()
    {
        auto list = getCurrentSongList();
        if (list != nullptr) listBox_->setList(*list);
    };
    addAndMakeVisible(setListComboBox_.get());
    
    newPlaylistButton_ = std::make_unique<TextButton>();
    newPlaylistButton_->setButtonText("New");
    newPlaylistButton_->setTooltip(TRANS("tooltip_new_playlist"));
    newPlaylistButton_->onClick = [&]()
    {
        auto inputDialog = std::make_shared<MelissaInputDialog>(host_, TRANS("enter_playlist_name"), "new", [&](const String& text) {
            if (text == "") return;
            host_->createPlaylist(text);
            host_->closeModalDialog();
        });
        host_->showModalDialog(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("new_playlist"));
    };
    addAndMakeVisible(newPlaylistButton_.get());
    
    removePlaylistButton_ = std::make_unique<TextButton>();
    removePlaylistButton_->setButtonText("Remove");
    removePlaylistButton_->setTooltip(TRANS("tooltip_remove_playlist"));
    removePlaylistButton_->onClick = [&]()
    {
        if (NativeMessageBox::showYesNoBox(AlertWindow::WarningIcon, TRANS("remove_playlist"), TRANS("are_you_sure")) == 0) return;
        
        const int selectedIndex = setListComboBox_->getSelectedId() - 1;
        data_.remove(selectedIndex);
        update();
        
        int indexToSelect = selectedIndex - 1;
        if (indexToSelect < 0) indexToSelect = 0;
        select(indexToSelect);
    };
    addAndMakeVisible(removePlaylistButton_.get());
    
    addToPlaylistButton_ = std::make_unique<TextButton>();
    addToPlaylistButton_->setButtonText("Add");
    addToPlaylistButton_->setTooltip(TRANS("tooltip_addto_playlist"));
    addToPlaylistButton_->onClick = [&]()
    {
        fileChooser_ = std::make_unique<FileChooser>(TRANS("choose_file_playlist"), File::getCurrentWorkingDirectory(), "*.mp3;*.wav;*.m4a", true);
        fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [&] (const FileChooser& chooser)
        {
            auto fileUrl = chooser.getURLResult();
            if (fileUrl.isLocalFile())
            {
                auto filePath = fileUrl.getLocalFile().getFullPathName();
                if (getCurrentSongList() == nullptr)
                {
                    auto inputDialog = std::make_shared<MelissaInputDialog>(host_, TRANS("enter_playlist_name"), "new", [&, filePath](const String& text) {
                        if (text == "") return;
                        host_->createPlaylist(text);
                        getCurrentSongList()->add(filePath);
                        host_->closeModalDialog();
                    });
                    host_->showModalDialog(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("new_playlist"));
                }
                else
                {
                    getCurrentSongList()->add(filePath);
                    update();
                }

                chooser.getResult().getParentDirectory().setAsCurrentWorkingDirectory();
            }
        });
    };
    addAndMakeVisible(addToPlaylistButton_.get());
    
    listBox_ = std::make_unique<MelissaFileListBox>(host_);
    addAndMakeVisible(listBox_.get());
}

const Array<var>& MelissaPlaylistComponent::getData()
{
    return data_;
}

void MelissaPlaylistComponent::setData(const Array<var>& data)
{
    data_ = data;
    update();
    select(0);
}

void MelissaPlaylistComponent::update()
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

void MelissaPlaylistComponent::select(int index)
{
    if (index < 0 ||  data_.size() < index) return;
    
    setListComboBox_->setSelectedId(index + 1);
    
    auto list = getCurrentSongList();
    if (list != nullptr) listBox_->setList(*list);
}

void MelissaPlaylistComponent::createPlaylist(const String& name, bool shouldSelect)
{
    auto object = new DynamicObject();
    object->setProperty("name", String(name));
    object->setProperty("songs", Array<var>());
    data_.add(object);
    update();

    if (shouldSelect) select(data_.size() - 1);
}

void MelissaPlaylistComponent::addToPlaylist(const String& filePath, int index)
{
    if (index == kIndex_Current)
    {
        getCurrentSongList()->add(filePath);
    }
    else if (0 <= index && index < data_.size())
    {
        data_[index].getDynamicObject()->getProperty("songs").getArray()->add(filePath);
        
    }
    update();
}


void MelissaPlaylistComponent::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int margin = 10;
    const int controlWidth = 80;
    const int controlHeight = 30;
    
    setListComboBox_->setBounds(0, 0, w - (controlWidth + margin) * 2, controlHeight);
    newPlaylistButton_->setBounds(setListComboBox_->getRight() + margin, 0, controlWidth, controlHeight);
    removePlaylistButton_->setBounds(newPlaylistButton_->getRight() + margin, 0, controlWidth, controlHeight);
    
    listBox_->setBounds(0, 40, w, h - (controlHeight + margin) * 2);
    addToPlaylistButton_->setBounds(w - controlWidth, h - controlHeight, controlWidth, controlHeight);
}

Array<var>* MelissaPlaylistComponent::getCurrentSongList()
{
    const int selectedIndex = setListComboBox_->getSelectedId() - 1;
    
    if (0 <= selectedIndex && selectedIndex < data_.size())
    {
        return data_[selectedIndex].getDynamicObject()->getProperty("songs").getArray();
    }
    
    return nullptr;
}
