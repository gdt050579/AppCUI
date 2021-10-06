#include "AppCUI.hpp"
#include <set>
#include <stack>
#include <vector>

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
    FileDialogClass() : wnd(nullptr), extFilter(nullptr)
    {
    }

    Pointer<Controls::Window> wnd;
    Reference<Controls::Label> lbPath, lbLocation;
    Reference<Controls::Label> lbName, lbExt;
    Reference<Controls::Splitter> splitListView;
    Reference<AppCUI::Controls::Panel> splitPanelLeft, splitPanelRight;
    Reference<Controls::ListView> lSpecialPaths;
    Reference<Controls::ListView> files;
    Reference<Controls::TextField> txName;
    Reference<Controls::ComboBox> comboType;
    std::vector<FSLocationData> locations;
    Reference<Controls::Button> btnOK, btnCancel;
    // TODO: Future back and forward option
    // Controls::Button btnBack, btnForward;
    std::vector<std::set<unsigned int>> extensions;
    std::set<unsigned int>* extFilter;
    std::filesystem::path resultedPath;
    std::filesystem::path currentPath;

    bool openDialog;

    bool ProcessExtensionFilter(const AppCUI::Utils::ConstString& extensionsFilter);

    int Show(
          bool open,
          const AppCUI::Utils::ConstString& fileName,
          const AppCUI::Utils::ConstString& extensionsFilter,
          const std::filesystem::path& _path);
    void UpdateCurrentFolder();
    void UpdateCurrentExtensionFilter();
    void UpdateFileList();
    bool OnEventHandler(const void* sender, AppCUI::Controls::Event eventType, int controlID);

    void Validate();
    void OnClickedOnItem();
    void OnCurrentItemChanged();
};

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

unsigned int __compute_hash__(const char16_t* start, const char16_t* end)
{
    // use FNV algorithm ==> https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    unsigned int hash       = 0x811c9dc5;
    const char16_t* p_start = (const char16_t*) start;
    const char16_t* p_end   = (const char16_t*) end;

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
      ListView* control, ItemHandle item1, ItemHandle item2, unsigned int columnIndex, void*)
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
      Control* control, const void* sender, AppCUI::Controls::Event eventType, int controlID, void* context)
{    
    return ((FileDialogClass*) context)->OnEventHandler(sender, eventType, controlID);
}

// https://www.cppstories.com/2018/07/string-view-perf-followup/
std::vector<std::u16string_view> splitSV(std::u16string_view strv, std::u16string_view delims = u" ")
{
    std::vector<std::u16string_view> output;
    size_t first = 0;

    while (first < strv.size())
    {
        const auto second = strv.find_first_of(delims, first);

        if (first != second)
            output.emplace_back(strv.substr(first, second - first));

        if (second == std::u16string_view::npos)
            break;

        first = second + 1;
    }
    return output;
}

bool FileDialogClass::ProcessExtensionFilter(const AppCUI::Utils::ConstString& extensiosFilter)
{
    using namespace std::literals;
    // format is: <Name>:ext|<Name>:ext| ...
    //        or: <Name>:ext1,ext2,ext3|<Name>:ext|....
    LocalUnicodeStringBuilder<256> filters;
    CHECK(filters.Set(extensiosFilter), false, "Failed to convert extensions filter");
    auto filterSV = filters.ToStringView();

    const auto filterGroups = splitSV(filterSV, u"|");
    for (const auto& nameAndExtensions : filterGroups)
    {
        const auto splitNameAndExtensions = splitSV(nameAndExtensions, u":");
        CHECK(splitNameAndExtensions.size() == 2, false, "The format is <Name>:ext1,ext2,ext3 or <Name>:ext");
        const auto filterName       = splitNameAndExtensions[0];
        const auto filterExtensions = splitSV(splitNameAndExtensions[1], u",");
        CHECK(filterExtensions.size() != 0,
              false,
              "Name should have at least one extension in the list, separated by coma");

        std::set<unsigned int> requiredExtensions;
        for (const auto& extension : filterExtensions)
        {
            requiredExtensions.insert(__compute_hash__(extension.data(), extension.data() + extension.size()));
        }
        CHECK(comboType->AddItem(filterName, ItemData{ this->extensions.size() }),
              false,
              "Failed to add item to combo-box ");
        this->extensions.push_back(requiredExtensions);
    }
    return true;
}

void FileDialogClass::OnClickedOnItem()
{
    int index = files->GetCurrentItem();
    if (index < 0)
        return;
    unsigned int value      = (int) files->GetItemData(index)->UInt32Value;
    std::filesystem::path p = currentPath;
    if (value == 0)
    {
        currentPath = p.parent_path();
        UpdateFileList();
        return;
    }
    if (value == 1)
    {
        p /= files->GetItemText(index, 0);
        currentPath = p;
        UpdateFileList();
        return;
    }
    if (value == 2)
        Validate();
}
void FileDialogClass::OnCurrentItemChanged()
{
    int index = files->GetCurrentItem();
    if (index < 0)
        return;
    unsigned int value = files->GetItemData(index)->UInt32Value;
    if (value == 2)
        txName->SetText(files->GetItemText(index, 0));
    else
        txName->SetText(""); // default value
}

void FileDialogClass::Validate()
{
    if (currentPath.empty())
    {
        return;
    }

    if (txName->GetText().Len() == 0)
        return;

    this->resultedPath = currentPath;
    this->resultedPath /= txName->GetText();

    bool exists = std::filesystem::exists(this->resultedPath);
    if (openDialog)
    {
        if (exists == false)
        {
            MessageBox::ShowError("Error", "Selected file does not exist!");
            return;
        }
    }
    else
    {
        if (exists)
        {
            if (MessageBox::ShowOkCancel("Overwrite", "Current file already exists. Overwrite?") != Dialogs::Result::Ok)
                return;
        }
    }
    // all is ok
    wnd->Exit(Dialogs::Result::Ok);
}

void FileDialogClass::UpdateCurrentFolder()
{
    const auto idx = lSpecialPaths->GetCurrentItem();
    currentPath    = locations[idx].locationPath;
    UpdateFileList();
}

void FileDialogClass::UpdateCurrentExtensionFilter()
{
    unsigned int idx = comboType->GetCurrentItemUserData().UInt32Value;
    if (idx == ALL_FILES_INDEX)
        this->extFilter = nullptr; // no filter
    else
    {
        if (idx < comboType->GetItemsCount())
            this->extFilter = &this->extensions[idx];
        else
            this->extFilter = nullptr; // something went wrong -> disable filtering
    }
}
void FileDialogClass::UpdateFileList()
{
    files->DeleteAllItems();
    if (std::filesystem::exists(currentPath))
    {
        try
        {
            currentPath = std::filesystem::canonical(currentPath);
        }
        catch (...)
        {
            // pass
        }

        std::filesystem::path p = currentPath;
        lbPath->SetText(p.u16string());

        if (p != p.root_path())
        {
            files->AddItem("..", "UP-DIR");
            files->SetItemData(0, ItemData{ nullptr });
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
                        auto ext16          = fileEntry.path().extension().u16string();
                        auto ext16Start     = ext16.data();
                        const auto ext16End = ext16.data() + ext16.size();

                        if (ext16.length() > 1 && ext16[0] == '.')
                        {
                            ext16Start++;
                        }
                        if (!extFilter->contains(__compute_hash__(ext16Start, ext16End)))
                            continue; // extension is filtered
                    }
                    ConvertSizeToString((unsigned long long) fileEntry.file_size(), size);
                }

                auto lastModifiedTime = getLastModifiedTime(fileEntry);
                std::strftime(time_rep, sizeof(time_rep), "%Y-%m-%d  %H:%M:%S", std::localtime(&lastModifiedTime));

                itemHandle = this->files->AddItem(fileEntry.path().filename().u16string(), size, time_rep);
                if (fileEntry.is_directory())
                {
                    this->files->SetItemColor(itemHandle, ColorPair{ Color::White, Color::Transparent });
                    this->files->SetItemData(itemHandle, ItemData{ 1 });
                }
                else
                {
                    this->files->SetItemColor(itemHandle, ColorPair{ Color::Gray, Color::Transparent });
                    this->files->SetItemData(itemHandle, ItemData{ 2 });
                }
            }
        }
        catch (...)
        {
            // for the moment skip
        }
        files->Sort();
    }
}
bool FileDialogClass::OnEventHandler(const void* sender, AppCUI::Controls::Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::ButtonClicked:
        if (controlID == (int) Dialogs::Result::Ok)
            Validate();
        else
            wnd->Exit(controlID);
        return true;
    case Event::WindowClose:
        wnd->Exit(Dialogs::Result::Cancel);
        return true;
    case Event::WindowAccept:
        Validate();
        return true;
    case Event::ComboBoxSelectedItemChanged:
        UpdateCurrentExtensionFilter();
        UpdateFileList();
        return true;
    case Event::TextFieldValidate:
        UpdateFileList();
        files->SetFocus();
        return true;
    case Event::ListViewCurrentItemChanged:
        if (lSpecialPaths == sender)
        {
            UpdateCurrentFolder();
        }
        else
        {
            OnCurrentItemChanged();
        }
        return true;
    case Event::ListViewItemClicked:
        if (sender == &lSpecialPaths)
        {
            UpdateCurrentFolder();
        }
        else
        {
            OnClickedOnItem();
        }
        return true;
    }
    return true;
}

int FileDialogClass::Show(
      bool open,
      const AppCUI::Utils::ConstString& fileName,
      const AppCUI::Utils::ConstString& extensionsFilter,
      const std::filesystem::path& _path)
{
    std::filesystem::path initialPath = std::filesystem::absolute(".");
    try
    {
        if (_path.empty())
        {
            initialPath = std::filesystem::absolute(".");
        }
        else
        {
            initialPath = std::filesystem::absolute(_path);
        }
    }
    catch (...)
    {
        // pass
    }
    currentPath = initialPath;
    openDialog  = open;

    wnd = Factory::Window::Create((open) ? "Open" : "Save", "w:78,h:23,d:c");
    wnd->SetEventHandler(FileDialog_EventHandler, this);

    lbLocation = Factory::Label::Create(wnd, "Location: ", "x:1,y:0,w:10");
    lbPath     = Factory::Label::Create(wnd, "", "x:11,y:0,w:62");

    splitListView = Factory::Splitter::Create(wnd, "x:0,y:1,w:76,h:15", true);
    splitListView->SetSecondPanelSize(60);
    splitPanelLeft  = Factory::Panel::Create(splitListView, "x:0,y:0,w:100%,h:100%");
    splitPanelRight = Factory::Panel::Create(splitListView, "x:0,y:0,w:100%,h:100%");

    ListViewFlags specialPathsFlags =
          ListViewFlags::HideColumnsSeparator | ListViewFlags::HideCurrentItemWhenNotFocused;
    lSpecialPaths = Factory::ListView::Create(splitPanelLeft, "x:0,y:0,w:100%,h:100%", specialPathsFlags);
    lSpecialPaths->AddColumn("Special", TextAlignament::Left, 20);

    // TODO: Future option for back and front
    // btnBack.Create(&wnd, "<", "x:1,y:0,w:3", 1, ButtonFlags::Flat);
    // btnForward.Create(&wnd, ">", "x:5,y:0,w:3", 2, ButtonFlags::Flat);

    SpecialFolderMap specialFoldersMap;
    RootsVector rootsVector;
    AppCUI::OS::GetSpecialFolders(specialFoldersMap, rootsVector);

    this->locations.push_back({ "Initial", initialPath });
    for (const auto& root : rootsVector)
    {
        this->locations.push_back(root);
    }

    for (const auto& specialFolder : specialFoldersMap)
    {
        this->locations.push_back(specialFolder.second);
    }

    for (const auto& locationInfo : locations)
    {
        lSpecialPaths->AddItem(locationInfo.locationName);
    }

    files = Factory::ListView::Create(splitPanelRight, "x:0,y:0,w:100%,h:100%", ListViewFlags::Sortable);
    files->AddColumn("&Name", TextAlignament::Left, 31);
    files->AddColumn("&Size", TextAlignament::Right, 16);
    files->AddColumn("&Modified", TextAlignament::Center, 20);
    files->SetItemCompareFunction(FileDialog_ListViewItemComparer, this);
    files->Sort(0, true); // sort after the first column, ascendent

    lbName = Factory::Label::Create(wnd, "File &Name", "x:2,y:17,w:11");
    txName = Factory::TextField::Create(wnd, fileName, "x:15,y:17,w:45");
    txName->SetHotKey('N');
    lbExt = Factory::Label::Create(wnd, "File &Type", "x:2,y:19,w:11");

    comboType = Factory::ComboBox::Create(wnd, "x:15,y:19,w:45");
    comboType->SetHotKey('T');

    btnOK     = Factory::Button::Create(wnd, "&Ok", "x:62,y:17,w:13", (int) Dialogs::Result::Ok);
    btnCancel = Factory::Button::Create(wnd, "&Cancel", "x:62,y:19,w:13", (int) Dialogs::Result::Cancel);

    this->ProcessExtensionFilter(extensionsFilter);

    if (this->comboType->GetItemsCount() > 0)
        this->comboType->AddSeparator();
    this->comboType->AddItem("All files", ItemData{ ALL_FILES_INDEX });
    this->comboType->SetCurentItemIndex(0);
    UpdateCurrentExtensionFilter();
    UpdateFileList();
    txName->SetFocus();
    return wnd->Show();
}

std::optional<std::filesystem::path> FileDialog::ShowSaveFileWindow(
      const AppCUI::Utils::ConstString& fileName,
      const AppCUI::Utils::ConstString& extensionsFilter,
      const std::filesystem::path& path)
{
    FileDialogClass dlg;
    int res = dlg.Show(false, fileName, extensionsFilter, path);
    if (res == (int) Dialogs::Result::Ok)
        return dlg.resultedPath;
    return std::nullopt;
}
std::optional<std::filesystem::path> FileDialog::ShowOpenFileWindow(
      const AppCUI::Utils::ConstString& fileName,
      const AppCUI::Utils::ConstString& extensionsFilter,
      const std::filesystem::path& path)
{
    FileDialogClass dlg;
    int res = dlg.Show(true, fileName, extensionsFilter, path);
    if (res == (int) Dialogs::Result::Ok)
        return dlg.resultedPath;
    return std::nullopt;
}
