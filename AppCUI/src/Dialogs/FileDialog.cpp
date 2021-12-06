#include "AppCUI.hpp"
#include <set>
#include <stack>
#include <vector>



namespace AppCUI
{
using namespace OS;
using namespace Graphics;
using namespace Controls;
using namespace Dialogs;
using namespace std::literals;

#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#endif

constexpr unsigned int ALL_FILES_INDEX = 0xFFFFFFFFU;

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

void ConvertSizeToString(uint64 size, char result[32])
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

// https://www.cppstories.com/2018/07/string-view-perf-followup/
vector<u16string_view> splitSV(u16string_view strv, u16string_view delims = u" ")
{
    vector<u16string_view> output;
    size_t first = 0;

    while (first < strv.size())
    {
        const auto second = strv.find_first_of(delims, first);

        if (first != second)
            output.emplace_back(strv.substr(first, second - first));

        if (second == u16string_view::npos)
            break;

        first = second + 1;
    }
    return output;
}

class FileDialogWindow : public Window
{
  public:
    FileDialogWindow(
          bool open,
          const ConstString& fileName,
          const ConstString& extensionsFilter,
          const std::filesystem::path& _path);

    bool OnEvent(Reference<Control> sender, Event eventType, int controlID) override;
    std::filesystem::path GetResultedPath() const;

  protected:
    Reference<Controls::Label> lbPath, lbLocation;
    Reference<Controls::Label> lbName, lbExt;
    Reference<Controls::Splitter> splitListView;
    Reference<Controls::Panel> splitPanelLeft, splitPanelRight;
    Reference<Controls::ListView> lSpecialPaths;
    Reference<Controls::ListView> files;
    Reference<Controls::TextField> txName;
    Reference<Controls::ComboBox> comboType;
    Reference<Controls::Button> btnOK, btnCancel;
    // TODO: Future back and forward option
    // Controls::Button btnBack, btnForward;
    vector<FSLocationData> locations;
    vector<std::set<unsigned int>> extensions;
    const std::set<unsigned int>* extFilter;
    std::filesystem::path resultedPath;
    std::filesystem::path currentPath;

    bool openDialog;

    void LoadAllSpecialLocations();
    bool ProcessExtensionFilter(const ConstString& extensionsFilter);

    void SpecialFoldersUpdatePath();
    void UpdateCurrentExtensionFilter();

    void UpdateCurrentPath(const std::filesystem::path& newPath);
    void ReloadCurrentPath();

    void ProcessTextFieldInput();
    void FileListItemClicked();
    void FileListItemChanged();
};

std::filesystem::path CanonizePath(std::filesystem::path p)
{
    std::filesystem::path result;
    std::error_code err;

    result = std::filesystem::canonical(p, err);

    if (err)
    {
        result = std::filesystem::absolute(".", err);
        if (err)
        {
            return p;
        }
    }
    return result;
}

FileDialogWindow::FileDialogWindow(
      bool open,
      const ConstString& fileName,
      const ConstString& extensionsFilter,
      const std::filesystem::path& specifiedPath)
    : Window(open ? "Open" : "Save", "w:78,h:23,d:c", WindowFlags::None), extFilter(nullptr), openDialog(open)
{
    const std::filesystem::path initialPath = CanonizePath(specifiedPath.empty() ? "." : specifiedPath);

    lbLocation = Factory::Label::Create(this, "Location: ", "x:1,y:0,w:10");
    lbPath     = Factory::Label::Create(this, "", "x:11,y:0,w:62");

    splitListView = Factory::Splitter::Create(this, "x:0,y:1,w:76,h:15", true);
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

    locations.push_back({ "Initial", initialPath });
    LoadAllSpecialLocations();
    for (const auto& locationInfo : locations)
    {
        lSpecialPaths->AddItem(locationInfo.locationName);
    }

    files = Factory::ListView::Create(splitPanelRight, "x:0,y:0,w:100%,h:100%", ListViewFlags::Sortable);
    files->AddColumn("&Name", TextAlignament::Left, 31);
    files->AddColumn("&Size", TextAlignament::Right, 16);
    files->AddColumn("&Modified", TextAlignament::Center, 20);
    files->SetItemCompareFunction(
          [](Controls::ListView* control, ItemHandle item1, ItemHandle item2, unsigned int columnIndex, void*) -> int
          {
              const auto& v1 = control->GetItemData(item1, 0);
              const auto& v2 = control->GetItemData(item2, 0);
              if (v1 < v2)
                  return -1;
              if (v1 > v2)
                  return 1;
              const auto& s1 = control->GetItemText(item1, columnIndex);
              const auto& s2 = control->GetItemText(item2, columnIndex);
              return s1.CompareWith(s2, true);
          },
          this);
    files->Sort(0, true); // sort after the first column, ascendent

    lbName = Factory::Label::Create(this, "File &Name", "x:2,y:17,w:11");
    txName = Factory::TextField::Create(this, fileName, "x:15,y:17,w:45", TextFieldFlags::ProcessEnter);
    txName->SetHotKey('N');
    lbExt = Factory::Label::Create(this, "File &Type", "x:2,y:19,w:11");

    btnOK     = Factory::Button::Create(this, "&Ok", "x:62,y:17,w:13", (int) Dialogs::Result::Ok);
    btnCancel = Factory::Button::Create(this, "&Cancel", "x:62,y:19,w:13", (int) Dialogs::Result::Cancel);

    comboType = Factory::ComboBox::Create(this, "x:15,y:19,w:45");
    comboType->SetHotKey('T');
    ProcessExtensionFilter(extensionsFilter);
    if (comboType->GetItemsCount() > 0)
        comboType->AddSeparator();
    comboType->AddItem("All files", ALL_FILES_INDEX);
    comboType->SetCurentItemIndex(0);
    UpdateCurrentExtensionFilter();

    UpdateCurrentPath(initialPath);
    txName->SetFocus();
}

void FileDialogWindow::LoadAllSpecialLocations()
{
    SpecialFolderMap specialFoldersMap;
    RootsVector rootsVector;
    OS::GetSpecialFolders(specialFoldersMap, rootsVector);

    for (const auto& root : rootsVector)
    {
        locations.push_back(root);
    }

    std::set<std::filesystem::path> locationDeDuplicator;

    for (const auto& specialFolder : specialFoldersMap)
    {
        if (!locationDeDuplicator.contains(specialFolder.second.locationPath))
        {
            locations.push_back(specialFolder.second);
            locationDeDuplicator.insert(specialFolder.second.locationPath);
        }
    }
}

std::filesystem::path FileDialogWindow::GetResultedPath() const
{
    return resultedPath;
}

bool FileDialogWindow::ProcessExtensionFilter(const ConstString& extensiosFilter)
{
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
        CHECK(comboType->AddItem(filterName, this->extensions.size()), false, "Failed to add item to combo-box ");
        this->extensions.push_back(requiredExtensions);
    }
    return true;
}

void FileDialogWindow::FileListItemClicked()
{
    int index = files->GetCurrentItem();
    if (index < 0)
        return;
    unsigned int value = (int) files->GetItemData(index, 0);
    if (value == 0)
    {
        try
        {
            UpdateCurrentPath(currentPath.parent_path());
        }
        catch (...)
        {
            MessageBox::ShowError("Error", u"Unable to get parent path of: "s + currentPath.u16string());
        }
        return;
    }
    else if (value == 1)
    {
        try
        {
            UpdateCurrentPath(currentPath / files->GetItemText(index, 0));
        }
        catch (...)
        {
            MessageBox::ShowError("Error", u"Unable to open path: "s + currentPath.u16string());
        }

        return;
    }
    else if (value == 2)
    {
        ProcessTextFieldInput();
    }
}
void FileDialogWindow::FileListItemChanged()
{
    const int index = files->GetCurrentItem();
    if (index < 0)
    {
        return;
    }

    const auto value = files->GetItemData(index, 0);
    if (value == 1)
    {
        txName->SetText(files->GetItemText(index, 0));
    }
    else if (value == 2)
    {
        txName->SetText(files->GetItemText(index, 0));
    }
    else
    {
        txName->SetText(""); // default value
    }
}

void FileDialogWindow::ProcessTextFieldInput()
{
    if (currentPath.empty() || txName->GetText().Len() == 0)
    {
        return;
    }

    std::filesystem::path candidateResultedPath = currentPath;
    try
    {
        candidateResultedPath /= txName->GetText();
    }
    catch (...)
    {
        MessageBox::ShowError("Error", "Unable to construct path");
        return;
    }

    std::error_code err;
    err.clear();
    // remove becuase of invalid paths keeps returing an error code, even if the path does not exists
    // const bool isDir = std::filesystem::is_directory(candidateResultedPath, err);
    const bool isDir = std::filesystem::is_directory(candidateResultedPath);
    // if (err)
    //{
    //    MessageBox::ShowError(
    //          "Error", u"Unable to check path for being a directory: "s + candidateResultedPath.u16string());
    //    return;
    //}

    const bool exists = std::filesystem::exists(candidateResultedPath, err);
    if (err)
    {
        MessageBox::ShowError("Error", u"Unable to check path for existence: "s + candidateResultedPath.u16string());
        return;
    }

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
    resultedPath = candidateResultedPath;
    Exit(Dialogs::Result::Ok);
}

void FileDialogWindow::SpecialFoldersUpdatePath()
{
    const auto idx = lSpecialPaths->GetCurrentItem();
    UpdateCurrentPath(locations[idx].locationPath);
}

void FileDialogWindow::UpdateCurrentExtensionFilter()
{
    unsigned int idx = (unsigned int) comboType->GetCurrentItemUserData(0);
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

void FileDialogWindow::ReloadCurrentPath()
{
    files->DeleteAllItems();

    if (currentPath != currentPath.root_path())
    {
        files->AddItem("..", "UP-DIR");
        files->SetItemData(0, 0);
    }

    char size[32];
    char dateBuffer[64]{ 0 };
    ItemHandle itemHandle;
    try
    {
        for (const auto& fileEntry : std::filesystem::directory_iterator(currentPath))
        {
            if (fileEntry.is_directory())
            {
                Utils::String::Set(size, "Folder", 32, 6);
            }
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
                    {
                        continue; // extension is filtered
                    }
                }
                ConvertSizeToString((uint64) fileEntry.file_size(), size);
            }

            const time_t date{ getLastModifiedTime(fileEntry) };
            struct tm t;
#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
            localtime_r(&date, &t); // TODO: errno not treated
            strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d  %H:%M:%S", &t);
#else
            localtime_s(&t, &date); // TODO: errno not treated
            std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d  %H:%M:%S", &t);
#endif

            itemHandle = this->files->AddItem(fileEntry.path().filename().u16string(), size, dateBuffer);
            if (fileEntry.is_directory())
            {
                this->files->SetItemColor(itemHandle, ColorPair{ Color::White, Color::Transparent });
                this->files->SetItemData(itemHandle, 1);
            }
            else
            {
                this->files->SetItemColor(itemHandle, ColorPair{ Color::Gray, Color::Transparent });
                this->files->SetItemData(itemHandle, 2);
            }
        }
    }
    catch (...)
    {
        MessageBox::ShowError("Error", u"Unable to read location: "s + currentPath.u16string());
    }

    files->Sort();
}

void FileDialogWindow::UpdateCurrentPath(const std::filesystem::path& newPath)
{
    std::error_code err;
    const bool exists = std::filesystem::exists(newPath, err);
    if (err)
    {
        MessageBox::ShowError("Error", u"Unable to check path for existence: "s + newPath.u16string());
        return;
    }
    if (!exists)
    {
        MessageBox::ShowError("Error", u"Path doesn't exist: "s + newPath.u16string());
        return;
    }
    const bool isDir = std::filesystem::is_directory(newPath, err);
    if (err)
    {
        MessageBox::ShowError("Error", u"Cannot check if the path is directory: "s + newPath.u16string());
        return;
    }
    if (!isDir)
    {
        MessageBox::ShowError("Error", u"Path is not a directory: "s + newPath.u16string());
        return;
    }

    std::filesystem::directory_iterator(newPath, err);
    if (err)
    {
        MessageBox::ShowError("Error", u"Unable to read location: "s + newPath.u16string());
        return;
    }

    currentPath = CanonizePath(newPath);
    lbPath->SetText(currentPath.u16string());
    ReloadCurrentPath();
}

bool FileDialogWindow::OnEvent(Reference<Control> sender, Controls::Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::WindowClose:
        Exit(Dialogs::Result::Cancel);
        return true;
    case Event::WindowAccept:
        ProcessTextFieldInput();
        return true;
    case Event::ButtonClicked:
        if (controlID == (int) Dialogs::Result::Ok)
        {
            ProcessTextFieldInput();
        }
        else
        {
            Exit(controlID);
        }
        return true;
    case Event::ComboBoxSelectedItemChanged:
        if (sender == comboType.To<Control>())
        {
            UpdateCurrentExtensionFilter();
            ReloadCurrentPath();
        }
        return true;
    case Event::TextFieldValidate:
        if (sender == txName.To<Control>())
        {
            ProcessTextFieldInput();
        }
        files->SetFocus();
        return true;
    case Event::ListViewCurrentItemChanged:
        if (sender == lSpecialPaths.To<Control>())
        {
            SpecialFoldersUpdatePath();
        }
        else if (sender == files.To<Control>())
        {
            FileListItemChanged();
        }
        return true;
    case Event::ListViewItemClicked:
        if (sender == lSpecialPaths.To<Control>())
        {
            SpecialFoldersUpdatePath();
        }
        else if (sender == files.To<Control>())
        {
            FileListItemClicked();
        }
        return true;
    }
    return true;
}

optional<std::filesystem::path> FileDialog::ShowSaveFileWindow(
      const ConstString& fileName, const ConstString& extensionsFilter, const std::filesystem::path& path)
{
    FileDialogWindow dlg(false, fileName, extensionsFilter, path);
    const int res = dlg.Show();
    if (res == (int) Dialogs::Result::Ok)
        return dlg.GetResultedPath();
    return std::nullopt;
}
optional<std::filesystem::path> FileDialog::ShowOpenFileWindow(
      const ConstString& fileName, const ConstString& extensionsFilter, const std::filesystem::path& path)
{
    FileDialogWindow dlg(true, fileName, extensionsFilter, path);
    const int res = dlg.Show();
    if (res == (int) Dialogs::Result::Ok)
        return dlg.GetResultedPath();
    return std::nullopt;
}
} // namespace AppCUI