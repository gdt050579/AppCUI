#include "AppCUI.hpp"

#include <set>

#define ALL_FILES_INDEX 0xFFFFFFFF


using namespace AppCUI;
using namespace AppCUI::OS;
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#endif

// Currently not all compilers support clock_cast (including gcc)
// AppleClang supports std::chrono::file_clock::to_time_t, but gcc or VS doesn't
// Have this frankenstein's monster while the compilers update
//
// Normally, we should be able to convert using clock_cast or a to_time_t kind of function
// to say we have full support
std::time_t getLastModifiedTime(const std::filesystem::directory_entry& entry)
{
#if BUILD_FOR_WINDOWS
    auto lastTime = entry.last_write_time();
    return std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(lastTime));
#elif BUILD_FOR_OSX
    struct stat attr;
    stat(entry.path().string().c_str(), &attr);
    return attr.st_mtimespec.tv_sec;
#elif BUILD_FOR_UNIX
    struct stat attr;
    stat(entry.path().string().c_str(), &attr);
    return attr.st_mtime;
#endif
}

struct FileDialogClass
{
    Controls::Window wnd;
    Controls::Label lbPath, lbDrive, lbName, lbExt;
    Controls::ComboBox comboDrive;
    Controls::ListView files;
    Controls::TextField txName;
    Controls::ComboBox comboType;
    Controls::Button btnOK, btnCancel;
    std::vector<std::pair<std::string, std::filesystem::path>> specialFolders;
    std::vector<std::set<unsigned int>> extensions;
    std::set<unsigned int>* extFilter;
    std::filesystem::path resultedPath;
    bool openDialog;

    bool ProcessExtensionFilter(const char* start, const char *end);

    int Show(
          bool open,
          const AppCUI::Utils::ConstString& fileName,
          std::string_view extensionFilter,
          const std::filesystem::path & _path);
    void UpdateCurrentFolder();
    void UpdateCurrentExtensionFilter();
    void UpdateFileList();
    bool OnEventHandler(const void* sender, AppCUI::Controls::Event eventType, int controlID);
    void Validate();
    void OnClickedOnItem();
    void OnCurrentItemChanged();
};

const char * SkipSpaces(const char * start, const char * end)
{
    while ((start < end) && (((*start) == ' ') || ((*start) == '\t')))
        start++;
    return start;
}
const char* SkipUntilChar(const char* start, const char* end, char ch)
{
    while ((start < end) && ((*start) != ch))
        start++;
    return start;
}
void ConvertSizeToString(unsigned long long size, char result[32])
{
    result[31] = 0;
    int poz    = 30;
    int cnt    = 0;
    do
    {
        result[poz--] = (size % 10) + '0';
        cnt++;
        size = size / 10;
        if ((cnt == 3) && (poz > 0) && (size > 0))
        {
            result[poz--] = ',';
            cnt           = 0;
        }

    } while ((size > 0) && (poz > 0));
    while (poz >= 0)
    {
        result[poz--] = ' ';
    }
}
unsigned int __compute_hash__(const char * start, const char * end)
{
    // use FNV algorithm ==> https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    unsigned int hash = 0x811c9dc5;
    const unsigned char* p_start = (const unsigned char*) start;
    const unsigned char* p_end   = (const unsigned char*) end;
    while (p_start < p_end)
    {
        unsigned int val = *p_start;
        if ((val >= 'A') && (val <= 'Z'))
            val |= 32;
        hash = hash ^ val;
        hash = hash * 0x01000193;
        p_start++;
    }
    return hash;
}
int FileDialog_ListViewItemComparer(
      ListView* control, ItemHandle item1, ItemHandle item2, unsigned int columnIndex, void* Context)
{
    unsigned long long v1 = control->GetItemData(item1)->UInt64Value;
    unsigned long long v2 = control->GetItemData(item2)->UInt64Value;
    if (v1 < v2)
        return -1;
    if (v1 > v2)
        return 1;
    auto& s1 = control->GetItemText(item1, columnIndex);
    auto& s2 = control->GetItemText(item2, columnIndex);
    return s1.CompareWith(s2, true);
}
bool FileDialog_EventHandler(
      Control* control, const void* sender, AppCUI::Controls::Event eventType, int controlID, void* Context)
{
    return ((FileDialogClass*) control)->OnEventHandler(sender, eventType, controlID);
}
bool FileDialogClass::ProcessExtensionFilter(const char* start, const char* end)
{
    // format is: <Name>:ext,<Name>:ext, ...
    //        or: <Name>:[ext1,ext2,ext3],<Name>:ext,....
    if (start >= end)
        return true;
    LocalString<256> temp;
    std::set<unsigned int> tempSet;
    const char* p = start;
    const char* n;
    const char* end_array;
    while (p<end)
    {
        p = SkipSpaces(p, end);
        n = SkipUntilChar(p, end, ':');
        CHECK(n > p, false, "Expecting a valid format (name:ext) or (name:[ext1,ext2,...extn]) !");
        CHECK(temp.Set(p, (unsigned int) (n - p)), false, "Fail to copy name into a string !");
        p = SkipSpaces(n + 1, end);
        CHECK(p < end, false, "Premature end of formated string (an extension must follow ':' delimiter)");
        if ((*p) =='[')
        {
            // we have a list of extensions
            tempSet.clear();
            p++;
            end_array = SkipUntilChar(p, end, ']');
            CHECK(end_array < end, false, "Premature end of an array list (missing ']' delimiter)");

            // parse the array
            while (p < end_array)
            {
                n = SkipUntilChar(p, end_array, ',');
                tempSet.insert(__compute_hash__(p, n));
                p = n + 1;
            }
            p = end_array + 1;
            p = SkipSpaces(p, end); // skip any possible spaces
            if ((p < end) && ((*p) == ','))
                p++; // for a comma after the end of the array
            // add
            CHECK(comboType.AddItem(temp.GetText(), ItemData{ this->extensions.size() }),
                  false,
                  "Fail to add item to combo-box ");
            this->extensions.push_back(tempSet);
        }
        else
        {
            // we have one extension
            n = SkipUntilChar(p,end,',');
            CHECK(n > p, false, "Expecting a valid format (name:ext): an extension must have at least one character");
            // add extension to set
            CHECK(comboType.AddItem(temp.GetText(), ItemData{ this->extensions.size() }),
                  false,
                  "Fail to add item to combo-box ");
            tempSet.clear();
            tempSet.insert(__compute_hash__(p,n));
            this->extensions.push_back(tempSet);
            p = n + 1;
        }
    }
    return true;
}

void FileDialogClass::OnClickedOnItem()
{
    int index = files.GetCurrentItem();
    if (index < 0)
        return;
    unsigned int value = (int) files.GetItemData(index)->UInt32Value;
    std::filesystem::path p = lbPath.GetText();
    if (value == 0)
    {
        lbPath.SetText(p.parent_path().u8string());
        UpdateFileList();
        return;
    }
    if (value == 1)
    {
        p /= files.GetItemText(index, 0);
        lbPath.SetText(p.u8string());
        UpdateFileList();
        return;
    }
    if (value == 2)
        Validate();
}
void FileDialogClass::OnCurrentItemChanged()
{
    int index = files.GetCurrentItem();
    if (index < 0)
        return;
    unsigned int value = files.GetItemData(index)->UInt32Value;
    if (value == 2)
        txName.SetText(files.GetItemText(index, 0));
    else
        txName.SetText(""); // default value
}
void FileDialogClass::Validate()
{
    if (txName.GetText().Len() == 0)
        return;
    if (lbPath.GetText().Len() == 0)
        return;

    this->resultedPath = (std::filesystem::path)lbPath.GetText();
    this->resultedPath /= txName.GetText();
    
    bool exists = std::filesystem::exists(this->resultedPath);
    if (openDialog)
    {
        if (exists == false)
        {
            MessageBox::ShowError("Error", "Selected file does not exists !");
            return;
        }
    }
    else
    {
        if (exists)
        {
            if (MessageBox::ShowOkCancel("Overwrite", "Current file already exists. Overwrite ?") !=
                Dialogs::Result::Ok)
                return;
        }
    }
    // all is ok
    wnd.Exit(Dialogs::Result::Ok);
}
void FileDialogClass::UpdateCurrentFolder()
{
    unsigned int idx = comboDrive.GetCurrentItemUserData().UInt32Value;
    // update lbPath with the name of the selected special folder
    lbPath.SetText((const char*) specialFolders[idx].second.u8string().c_str());
    UpdateFileList();
}
void FileDialogClass::UpdateCurrentExtensionFilter()
{
    unsigned int idx = comboType.GetCurrentItemUserData().UInt32Value;
    if (idx == ALL_FILES_INDEX)
        this->extFilter = nullptr; // no filter
    else
    {
        if (idx < comboType.GetItemsCount())
            this->extFilter = &this->extensions[idx];
        else
            this->extFilter = nullptr; // something went wrong -> disable filtering
    }
}
void FileDialogClass::UpdateFileList()
{
    files.DeleteAllItems();
    std::string s_p;
    if (lbPath.GetText().ToString(s_p))
    {
        std::filesystem::path p = s_p;
        if (p != p.root_path())
        {
            files.AddItem("..", "UP-DIR");
            files.SetItemData(0, 0);
        }
        char size[32];
        char time_rep[64];
        ItemHandle itemHandle;
        try
        {
            for (const auto& fileEntry : std::filesystem::directory_iterator(p))
            {
                if (fileEntry.is_directory())
                    Utils::String::Set(size, "Folder", 32, 6);
                else
                {
                    // check filter first
                    if (extFilter)
                    {
                        // a filter is set - let's check the extention
                        auto ext_u8         = fileEntry.path().extension().u8string();
                        const char* ext     = (const char*) ext_u8.c_str();
                        const char* ext_end = ext + ext_u8.size();
                        if ((ext != nullptr) && ((*ext) == '.'))
                            ext++;
                        unsigned int hash = __compute_hash__(ext, ext_end);
                        if (!extFilter->contains(hash))
                            continue; // extension is filtered
                    }
                    ConvertSizeToString((unsigned long long) fileEntry.file_size(), size);
                }
                    

                auto lastModifiedTime = getLastModifiedTime(fileEntry);
                std::strftime(time_rep, sizeof(time_rep), "%Y-%m-%d  %H:%M:%S", std::localtime(&lastModifiedTime));

                itemHandle =
                      this->files.AddItem((const char*) fileEntry.path().filename().u8string().c_str(), size, time_rep);
                if (fileEntry.is_directory())
                {
                    this->files.SetItemColor(itemHandle, ColorPair{ Color::White, Color::Transparent });
                    this->files.SetItemData(itemHandle, ItemData{ 1 });
                }
                else
                {
                    this->files.SetItemColor(itemHandle, ColorPair{ Color::Gray, Color::Transparent });
                    this->files.SetItemData(itemHandle, ItemData{ 2 });
                }
            }
        }
        catch (...)
        {
            // for the moment skip
        }
        files.Sort();
    }
}
bool FileDialogClass::OnEventHandler(const void* sender, AppCUI::Controls::Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::EVENT_BUTTON_CLICKED:
        if (controlID == (int) Dialogs::Result::Ok)
            Validate();
        else
            wnd.Exit(controlID);
        return true;
    case Event::EVENT_WINDOW_CLOSE:
        wnd.Exit(Dialogs::Result::Cancel);
        return true;
    case Event::EVENT_WINDOW_ACCEPT:
        Validate();
        return true;
    case Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED:
        if (sender == &comboDrive)
        {
            UpdateCurrentFolder();
            UpdateFileList();
        }
        else if (sender == &comboType)
        {
            UpdateCurrentExtensionFilter();
            UpdateFileList();
        }
        return true;
    case Event::EVENT_TEXTFIELD_VALIDATE:
        UpdateFileList();
        files.SetFocus();
        return true;
    case Event::EVENT_LISTVIEW_CURRENTITEM_CHANGED:
        OnCurrentItemChanged();
        return true;
    case Event::EVENT_LISTVIEW_ITEM_CLICKED:
        OnClickedOnItem();
        return true;
    }
    return true;
}
int FileDialogClass::Show(
      bool open,
      const AppCUI::Utils::ConstString& fileName,
      std::string_view extensionFilter,
      const std::filesystem::path & _path)
{

    extFilter       = nullptr;
    //defaultFileName = fileName;
    openDialog      = open;
    if (open)
        wnd.Create("Open", "w:78,h:23,a:c");
    else
        wnd.Create("Save", "w:78,h:23,a:c");
    wnd.SetEventHandler(FileDialog_EventHandler, this);
    lbPath.Create(&wnd, "", "x:2,y:2,w:63");
    lbDrive.Create(&wnd, "&Location", "x:2,y:1,w:8");
    comboDrive.Create(&wnd, "x:12,y:1,w:61");
    comboDrive.SetHotKey('L');
    // populate combo box with special folders and available drivers
    this->specialFolders.clear();
    AppCUI::OS::GetSpecialFolders(this->specialFolders,SpecialFoldersType::Drives,false);
    if (this->specialFolders.size() > 0)
    {
        comboDrive.AddSeparator("Drives");
        for (unsigned int index = 0; index < this->specialFolders.size(); index++)
            comboDrive.AddItem(this->specialFolders[index].first.c_str(), ItemData{ index });
    }
    auto lastSize = this->specialFolders.size();
    AppCUI::OS::GetSpecialFolders(this->specialFolders, SpecialFoldersType::SpecialLocations, false);
    if (this->specialFolders.size() > lastSize)
    {
        comboDrive.AddSeparator("Locations");
        for (unsigned int index = lastSize; index < this->specialFolders.size(); index++)
            comboDrive.AddItem(this->specialFolders[index].first.c_str(), ItemData{ index });
    }
    files.Create(&wnd, "x:2,y:3,w:72,h:13", ListViewFlags::Sortable);
    files.AddColumn("&Name", TextAlignament::Left, 31);
    files.AddColumn("&Size", TextAlignament::Right, 16);
    files.AddColumn("&Modified", TextAlignament::Center, 20);
    files.SetItemCompareFunction(FileDialog_ListViewItemComparer, this);
    files.Sort(0, true); // sort after the first column, ascendent

    lbName.Create(&wnd, "File &Name", "x:2,y:17,w:10");
    txName.Create(&wnd, fileName, "x:13,y:17,w:47");
    txName.SetHotKey('N');
    lbExt.Create(&wnd, "File &Type", "x:2,y:19,w:10");
    comboType.Create(&wnd, "x:13,y:19,w:47");
    comboType.SetHotKey('T');

    btnOK.Create(&wnd, "&Ok", "x:62,y:17,w:13", (int) Dialogs::Result::Ok);
    btnCancel.Create(&wnd, "&Cancel", "x:62,y:19,w:13", (int) Dialogs::Result::Cancel);
    try
    {
        if (_path.empty())
            lbPath.SetText(std::filesystem::absolute(".").u8string());
        else
            lbPath.SetText(std::filesystem::absolute(_path).u8string());        
    }
    catch (...)
    {
        lbPath.SetText(std::filesystem::absolute(".").u8string());
    }

    this->ProcessExtensionFilter(extensionFilter.data(), extensionFilter.data() + extensionFilter.size());
    if (this->comboType.GetItemsCount() > 0)
        this->comboType.AddSeparator();
    this->comboType.AddItem("All files", ItemData{ ALL_FILES_INDEX });
    this->comboType.SetCurentItemIndex(0);
    UpdateCurrentExtensionFilter();
    UpdateFileList();
    txName.SetFocus();
    return wnd.Show();
}

std::optional<std::filesystem::path> FileDialog::ShowSaveFileWindow(
      const AppCUI::Utils::ConstString& fileName, std::string_view extensionFilter, const std::filesystem::path & path)
{
    FileDialogClass dlg;
    int res = dlg.Show(false, fileName, extensionFilter, path);
    if (res == (int) Dialogs::Result::Ok)
        return dlg.resultedPath;
    return std::nullopt;
}
std::optional<std::filesystem::path> FileDialog::ShowOpenFileWindow(
      const AppCUI::Utils::ConstString& fileName, std::string_view extensionFilter, const std::filesystem::path & path)
{
    FileDialogClass dlg;
    int res = dlg.Show(true, fileName, extensionFilter, path);
    if (res == (int) Dialogs::Result::Ok)
        return dlg.resultedPath;
    return std::nullopt;
}
