#include "MelissaInputDialog.h"
#include "MelissaModalDialog.h"
#include "MelissaModel.h"
#include "MelissaPlaylistComponent.h"

MelissaPlaylistComponent::MelissaPlaylistComponent(MelissaHost* host) :
host_(host)
{
    createUI();
}

enum
{
    kMenuID_New = 1,
    kMenuID_Rename,
    kMenuID_Remove
};

enum
{
    kMenuIDAddToList_Select = 1,
    kMenuIDAddToList_Current
};

void MelissaPlaylistComponent::createUI()
{
    playlistComboBox_ = std::make_unique<ComboBox>();
    playlistComboBox_->onChange = [&]()
    {
        auto list = getCurrentSongList();
        if (list != nullptr) listBox_->setList(*list);
    };
    addAndMakeVisible(playlistComboBox_.get());
    
    menuButton_ = std::make_unique<MelissaMenuButton>();
    menuButton_->onClick = [&]()
    {
        PopupMenu menu;
        menu.setLookAndFeel(&lookAndFeel_);
        menu.addItem(kMenuID_New, TRANS("create_playlist"));
        menu.addItem(kMenuID_Rename, TRANS("rename_playlist"));
        menu.addItem(kMenuID_Remove, TRANS("remove_playlist"));
        
        const auto result = menu.show();
        if (result == kMenuID_New)
        {
            auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), "New playlist", [&](const String& text) {
                if (text == "") return;
                host_->createPlaylist(text);
                MelissaModalDialog::close();
            });
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("create_playlist"));
        }
        else if (result == kMenuID_Rename)
        {
            const int selectedIndex = playlistComboBox_->getSelectedId() - 1;
            if (selectedIndex < 0) return;
            const String currentName = data_[selectedIndex].getDynamicObject()->getProperty("name").toString();
            auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), currentName, [&, selectedIndex](const String& text) {
                if (text == "") return;
                data_[selectedIndex].getDynamicObject()->setProperty("name", text);
                MelissaModalDialog::close();
                update();
            });
            MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("rename_playlist"));
        }
        else if (result == kMenuID_Remove)
        {
            if (NativeMessageBox::showYesNoBox(AlertWindow::WarningIcon, TRANS("remove_playlist"), TRANS("are_you_sure")) == 0) return;
            
            const int selectedIndex = playlistComboBox_->getSelectedId() - 1;
            data_.remove(selectedIndex);
            update();
            
            int indexToSelect = selectedIndex - 1;
            if (indexToSelect < 0) indexToSelect = 0;
            select(indexToSelect);
        }
    };
    addAndMakeVisible(menuButton_.get());
    
    addToPlaylistButton_ = std::make_unique<MelissaAddButton>();
    addToPlaylistButton_->setTooltip(TRANS("tooltip_addto_playlist"));
    addToPlaylistButton_->onClick = [&]()
    {
        PopupMenu menu;
        menu.setLookAndFeel(&lookAndFeel_);
        menu.addItem(kMenuID_New, TRANS("addtolist_select"));
        menu.addItem(kMenuID_Rename, TRANS("addtolist_current"));
        
        const auto result = menu.show();
        if (result == kMenuIDAddToList_Select)
        {
            fileChooser_ = std::make_unique<FileChooser>(TRANS("choose_file_playlist"), File::getCurrentWorkingDirectory(), "*.mp3;*.wav;*.m4a", true);
            fileChooser_->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [&] (const FileChooser& chooser)
            {
                auto fileUrl = chooser.getURLResult();
                if (fileUrl.isLocalFile())
                {
                    auto filePath = fileUrl.getLocalFile().getFullPathName();
                    addToCurrentPlaylist(filePath);
                    chooser.getResult().getParentDirectory().setAsCurrentWorkingDirectory();
                }
            });
        }
        else if (result == kMenuIDAddToList_Current)
        {
            //addToCurrentPlaylist();
        }
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
    
    const int selectedId = playlistComboBox_->getSelectedId();
    
    playlistComboBox_->clear();
    for (int iItemId = 0; iItemId < data_.size(); ++iItemId)
    {
        playlistComboBox_->addItem(data_[iItemId].getDynamicObject()->getProperty("name").toString(), iItemId + 1);
    }
    
    select(selectedId - 1);
}

void MelissaPlaylistComponent::select(int index)
{
    if (index < 0 ||  data_.size() < index) return;
    
    playlistComboBox_->setSelectedId(index + 1);
    
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
    const int controlWidth = 30;
    const int controlHeight = 30;
    
    playlistComboBox_->setBounds(0, 0, w - (26 + margin + 10), controlHeight);
    menuButton_->setBounds(playlistComboBox_->getRight() + margin + 5, (playlistComboBox_->getHeight() - 14) / 2, 26, 14);
    
    listBox_->setBounds(0, 40, w, h - (controlHeight + margin) * 2);
    addToPlaylistButton_->setBounds(w - controlWidth, h - controlHeight, controlWidth, controlHeight);
}

Array<var>* MelissaPlaylistComponent::getCurrentSongList()
{
    const int selectedIndex = playlistComboBox_->getSelectedId() - 1;
    
    if (0 <= selectedIndex && selectedIndex < data_.size())
    {
        auto array = data_[selectedIndex].getDynamicObject()->getProperty("songs").getArray();
        if (array == nullptr)
        {
            data_[selectedIndex].getDynamicObject()->setProperty("songs", Array<var>());
        }
        return data_[selectedIndex].getDynamicObject()->getProperty("songs").getArray();
    }
    
    return nullptr;
}

void MelissaPlaylistComponent::addToCurrentPlaylist(const String& filePath)
{
    if (filePath.isEmpty()) return;
    
    const int selectedIndex = playlistComboBox_->getSelectedId() - 1;
    if (selectedIndex < 0)
    {
        auto inputDialog = std::make_shared<MelissaInputDialog>(TRANS("enter_playlist_name"), "new", [&, filePath](const String& text) {
            if (text == "") return;
            createPlaylist(text, true);
            getCurrentSongList()->add(filePath);
            MelissaModalDialog::close();
        });
        MelissaModalDialog::show(std::dynamic_pointer_cast<Component>(inputDialog), TRANS("new_playlist"));
    }
    else
    {
        auto list = getCurrentSongList();
        if (list == nullptr) list->add(Array<var>());
        list->add(filePath);
        update();
    }
}
