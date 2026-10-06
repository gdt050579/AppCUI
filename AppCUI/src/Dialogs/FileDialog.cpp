#include "AppCUI.hpp"

#include <set>
#include <stack>

namespace AppCUI
{
using namespace OS;
using namespace Graphics;
using namespace Controls;
using namespace Dialogs;
using namespace std::literals;

constexpr uint32 ALL_FILES_INDEX = 0xFFFFFFFFU;

static void ConvertSizeToString(uint64 size, char result[32])
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

static uint32 __compute_hash__(const char16* start, const char16* end)
{
    // use FNV algorithm ==> https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    uint32 hash           = 0x811c9dc5;
    const char16* p_start = (const char16*) start;
    const char16* p_end   = (const char16*) end;

    while (p_start < p_end)
    {
        uint32 val = *p_start;
        if ((val >= 'A') && (val <= 'Z'))
            val |= 32;
        hash = hash ^ val;
        hash = hash * 0x01000193;
        p_start++;
    }
    return hash;
}

// https://www.cppstories.com/2018/07/string-view-perf-followup/
static vector<u16string_view> splitSV(u16string_view strv, u16string_view delims = u" ")
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
    vector<std::set<uint32>> extensions;
    const std::set<uint32>* extFilter;
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
    : Window(open ? "Open" : "Save", "w:60%,h:80%,d:c", WindowFlags::Sizeable), extFilter(nullptr), openDialog(open)
{
    const std::filesystem::path initialPath = CanonizePath(specifiedPath.empty() ? "." : specifiedPath);

    lbLocation = Factory::Label::Create(this, "Location: ", "t:0,l:1,w:11");
    lbPath     = Factory::Label::Create(this, "", "t:0,l:11,r:2");

    splitListView = Factory::Splitter::Create(this, "t:1,l:2,r:2,b:4", SplitterFlags::Vertical);
    splitListView->SetSecondPanelSize(static_cast<uint32>(0.75 * this->GetWidth()));
    splitPanelLeft  = Factory::Panel::Create(splitListView, "x:1,y:0,w:100%,h:100%");
    splitPanelRight = Factory::Panel::Create(splitListView, "x:1,y:0,w:100%,h:100%");

    ListViewFlags specialPathsFlags = ListViewFlags::HideColumnsSeparator |
                                      ListViewFlags::HideCurrentItemWhenNotFocused | ListViewFlags::HideBorder |
                                      ListViewFlags::PopupSearchBar | ListViewFlags::HideScrollBar;
    lSpecialPaths = Factory::ListView::Create(
          splitPanelLeft, "x:1,y:0,w:100%,h:100%", { "n:Special,a:l,w:20" }, specialPathsFlags);

    // TODO: Future option for back and front
    // btnBack.Create(&wnd, "<", "x:1,y:0,w:3", 1, ButtonFlags::Flat);
    // btnForward.Create(&wnd, ">", "x:5,y:0,w:3", 2, ButtonFlags::Flat);

    locations.push_back({ "Initial", initialPath });
    LoadAllSpecialLocations();
    uint64 idx = 0;
    for (const auto& locationInfo : locations)
    {
        auto item = lSpecialPaths->AddItem(locationInfo.locationName);
        item.SetData(idx++);
    }

    files = Factory::ListView::Create(
          splitPanelRight,
          "x:1,y:0,w:100%,h:100%",
          { "n:&Name,w:60%,a:l", "n:&Size,w:10%,a:r", "n:&Modified,w:30%,a:c" },
          ListViewFlags::Sortable | ListViewFlags::HideBorder | ListViewFlags::PopupSearchBar |
                ListViewFlags::HideScrollBar);
    files->Handlers()->ComparereItem =
          [](Reference<ListView> control, const ListViewItem& item1, const ListViewItem& item2) -> int
    {
        const auto& v1 = item1.GetData(0);
        const auto& v2 = item2.GetData(0);
        if (v1 < v2)
            return -1;
        if (v1 > v2)
            return 1;
        auto cindex = control->GetSortColumnIndex();
        if (cindex.has_value())
        {
            const auto& s1 = item1.GetText(cindex.value());
            const auto& s2 = item2.GetText(cindex.value());
            return s1.CompareWith(s2, true);
        }
        // else --> no sortable column
        return 0;
    };
    files->Sort(0, SortDirection::Ascendent); // sort after the first column, ascendent

    lbName = Factory::Label::Create(this, "File &Name", "b:3,l:1,w:11");
    txName = Factory::TextField::Create(this, fileName, "b:3,l:12,r:11", TextFieldFlags::ProcessEnter);
    txName->SetHotKey('N');
    btnOK = Factory::Button::Create(this, "&Ok", "b:2,r:1,w:8", (int) Dialogs::Result::Ok);

    lbExt     = Factory::Label::Create(this, "File &Type", "b:1,l:1,w:11");
    btnCancel = Factory::Button::Create(this, "&Cancel", "b:0,r:1,w:8", (int) Dialogs::Result::Cancel);

    comboType = Factory::ComboBox::Create(this, "b:1,l:12,r:11");
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

        std::set<uint32> requiredExtensions;
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
    auto current = files->GetCurrentItem();
    if (!current.IsValid())
        return;
    uint32 value = (int) current.GetData(0);
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
            UpdateCurrentPath(currentPath / current.GetText(0));
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
    auto current = files->GetCurrentItem();
    if (!current.IsValid())
        return;
    uint32 value = (int) current.GetData(0);
    if (value == 1)
    {
        txName->SetText(current.GetText(0));
    }
    else if (value == 2)
    {
        txName->SetText(current.GetText(0));
    }
    else
    {
        txName->SetText(""); // default value
    }
}

void FileDialogWindow::ProcessTextFieldInput()
{
    if (currentPath.empty())
    {
        return;
    }

    std::filesystem::path candidateResultedPath = currentPath;
    try
    {
        if (!txName->GetText().IsEmpty())
        {
            candidateResultedPath /= txName->GetText();
        }
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
    // const bool isDir = std::filesystem::is_directory(candidateResultedPath);
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
    UpdateCurrentPath(locations[lSpecialPaths->GetCurrentItem().GetData(0)].locationPath);
}

void FileDialogWindow::UpdateCurrentExtensionFilter()
{
    uint32 idx = (uint32) comboType->GetCurrentItemUserData(0);
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
        files->AddItem({ "..", "UP-DIR" }).SetData(0);
    }

    char size[32];
    AppCUI::OS::DateTime dt;
    try
    {
        for (const auto& fileEntry : std::filesystem::directory_iterator(currentPath))
        {
            bool isDirectory = false;
            bool failed      = false;
            try
            {
                isDirectory = fileEntry.is_directory();
                if (isDirectory)
                {
                    Utils::String::Set(size, "Folder", 32, 6);
                }
            }
            catch (...)
            {
                MessageBox::ShowError("Error", u"Unable to read location: "s + fileEntry.path().u16string());
                Utils::String::Set(size, "Unknown", 32, 7);
                failed = true; // not really - maybe reparse point
            }

            if (isDirectory == false)
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

                if (failed == false)
                {
                    ConvertSizeToString((uint64) fileEntry.file_size(), size);
                }
            }

            dt.CreateFrom(fileEntry);
            auto item =
                  this->files->AddItem({ fileEntry.path().filename().u16string(), size, dt.GetStringRepresentation() });

            if (failed)
            {
                item.SetType(ListViewItem::Type::ErrorInformation);
                item.SetData(2);
            }
            else if (isDirectory)
            {
                item.SetType(ListViewItem::Type::Highlighted);
                item.SetData(1);
            }
            else
            {
                item.SetType(ListViewItem::Type::GrayedOut);
                item.SetData(2);
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
            Exit(Dialogs::Result::Cancel);
        }
        return true;
    case Event::ComboBoxSelectedItemChanged:
        if (sender == comboType.ToBase<Control>())
        {
            UpdateCurrentExtensionFilter();
            ReloadCurrentPath();
        }
        return true;
    case Event::TextFieldValidate:
        if (sender == txName.ToBase<Control>())
        {
            ProcessTextFieldInput();
        }
        files->SetFocus();
        return true;
    case Event::ListViewCurrentItemChanged:
        if (sender == lSpecialPaths.ToBase<Control>())
        {
            SpecialFoldersUpdatePath();
        }
        else if (sender == files.ToBase<Control>())
        {
            FileListItemChanged();
        }
        return true;
    case Event::ListViewItemPressed:
        if (sender == lSpecialPaths.ToBase<Control>())
        {
            SpecialFoldersUpdatePath();
        }
        else if (sender == files.ToBase<Control>())
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
    if (dlg.Show() == Dialogs::Result::Ok)
        return dlg.GetResultedPath();
    return std::nullopt;
}

optional<std::filesystem::path> FileDialog::ShowOpenFileWindow(
      const ConstString& fileName, const ConstString& extensionsFilter, const std::filesystem::path& path)
{
    FileDialogWindow dlg(true, fileName, extensionsFilter, path);
    if (dlg.Show() == Dialogs::Result::Ok)
        return dlg.GetResultedPath();
    return std::nullopt;
}
} // namespace AppCUI