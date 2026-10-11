#include "Internal.hpp"

#include <set>
#include <stack>

#ifdef MessageBox
#    undef MessageBox // <windows.h> (through Internal.hpp) maps it to MessageBoxA / MessageBoxW
#endif

namespace AppCUI
{
using namespace OS;
using namespace Graphics;
using namespace Controls;
using namespace Dialogs;
using namespace std::literals;

constexpr uint32 ALL_FILES_INDEX = 0xFFFFFFFFU;
// Upload / Download (frontends with host file transfer only) - outside of the Dialogs::Result range used by Ok/Cancel
constexpr int BUTTON_ID_UPLOAD   = 100;
constexpr int BUTTON_ID_DOWNLOAD = 101;

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
    Reference<Controls::Button> btnUpload, btnDownload; // only when the frontend supports host file transfer
    // TODO: Future back and forward option
    // Controls::Button btnBack, btnForward;
    vector<FSLocationData> locations;
    vector<std::set<uint32>> extensions;
    vector<std::string> acceptFilters; // per extensions[] entry: ".ext1,.ext2" (for the file picker of the host)
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

    bool SelectFileByName(const std::filesystem::path& name);
    void UploadFiles();
    void DownloadCurrentFile();
    void UpdateDownloadButton();
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
          "l:1,t:0,r:0,b:0",
          { "n:&Name,w:fill,a:l", "n:&Size,w:14,a:r", "n:&Modified,w:19,a:c" },
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

    // A sandboxed frontend (the browser) has a file system of its own: a second column of buttons moves files between
    // it and the machine of the user. Without host file transfer the layout is the classic one.
    const bool hostTransfer = OS::IsHostFileTransferAvailable();

    lbName = Factory::Label::Create(this, "File &Name", "b:3,l:1,w:11");
    txName = Factory::TextField::Create(
          this, fileName, hostTransfer ? "b:3,l:12,r:23" : "b:3,l:12,r:11", TextFieldFlags::ProcessEnter);
    txName->SetHotKey('N');
    if (hostTransfer)
        btnUpload = Factory::Button::Create(this, "&Upload", "b:2,r:12,w:10", BUTTON_ID_UPLOAD);
    btnOK = Factory::Button::Create(
          this, "&Ok", hostTransfer ? "b:2,r:1,w:10" : "b:2,r:1,w:8", (int) Dialogs::Result::Ok);

    lbExt = Factory::Label::Create(this, "File &Type", "b:1,l:1,w:11");
    if (hostTransfer)
    {
        btnDownload = Factory::Button::Create(this, "&Download", "b:0,r:12,w:10", BUTTON_ID_DOWNLOAD);
        btnDownload->SetEnabled(false); // enabled while the current item of the files list is a file
    }
    btnCancel = Factory::Button::Create(
          this, "&Cancel", hostTransfer ? "b:0,r:1,w:10" : "b:0,r:1,w:8", (int) Dialogs::Result::Cancel);

    comboType = Factory::ComboBox::Create(this, hostTransfer ? "b:1,l:12,r:23" : "b:1,l:12,r:11");
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
        std::string accept;
        for (const auto& extension : filterExtensions)
        {
            requiredExtensions.insert(__compute_hash__(extension.data(), extension.data() + extension.size()));
            // the picker of the host only gets printable ASCII extensions (a hint: the dialog still filters by hash)
            bool printable = true;
            for (const auto ch : extension)
            {
                if ((ch <= u' ') || (ch >= 127) || (ch == u','))
                {
                    printable = false;
                    break;
                }
            }
            if (printable)
            {
                if (!accept.empty())
                    accept += ',';
                accept += '.';
                for (const auto ch : extension)
                    accept += static_cast<char>(ch);
            }
        }
        CHECK(comboType->AddItem(filterName, this->extensions.size()), false, "Failed to add item to combo-box ");
        this->extensions.push_back(requiredExtensions);
        this->acceptFilters.push_back(std::move(accept));
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
    UpdateDownloadButton();
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

void FileDialogWindow::UpdateDownloadButton()
{
    if (!btnDownload.IsValid())
        return;
    auto current = files->GetCurrentItem();
    btnDownload->SetEnabled(current.IsValid() && (current.GetData(0) == 2));
}

bool FileDialogWindow::SelectFileByName(const std::filesystem::path& name)
{
    const auto target = name.u16string();
    std::u16string text;
    const auto count = files->GetItemsCount();
    for (uint32 idx = 0; idx < count; idx++)
    {
        auto item = files->GetItem(idx);
        if ((item.GetData(0) != 2) || (!item.GetText(0).ToString(text)) || (text != target))
            continue;
        files->SetCurrentItem(item);
        txName->SetText(target);
        UpdateDownloadButton();
        return true;
    }
    return false;
}

void FileDialogWindow::UploadFiles()
{
    auto app = Application::GetApplication();
    if ((app == nullptr) || (app->terminal == nullptr) || currentPath.empty())
        return;

    std::string_view accept;
    const auto idx = static_cast<uint32>(comboType->GetCurrentItemUserData(ALL_FILES_INDEX));
    if (idx < acceptFilters.size())
        accept = acceptFilters[idx];

    vector<std::filesystem::path> uploaded;
    std::string error;
    const bool ok = app->terminal->UploadFromHost(currentPath, accept, true, uploaded, error);
    if (!uploaded.empty())
    {
        // the picker of the host only *suggests* the extensions -> never hide what the user has just uploaded
        ReloadCurrentPath();
        if ((!SelectFileByName(uploaded[0])) && (extFilter != nullptr))
        {
            comboType->SetCurentItemIndex(comboType->GetItemsCount() - 1); // "All files" is always the last item
            UpdateCurrentExtensionFilter();
            ReloadCurrentPath();
            SelectFileByName(uploaded[0]);
        }
        btnOK->SetFocus(); // a single Enter opens the uploaded file
    }
    if (!ok)
        MessageBox::ShowError("Upload", error.empty() ? std::string_view("Unable to upload the file(s)") : error);
}

void FileDialogWindow::DownloadCurrentFile()
{
    auto current = files->GetCurrentItem();
    if ((!current.IsValid()) || (current.GetData(0) != 2) || currentPath.empty())
        return;
    std::filesystem::path name;
    if (!current.GetText(0).ToPath(name))
        return;
    if (!OS::DownloadFile(currentPath / name))
        MessageBox::ShowError("Download", u"Unable to download: "s + name.u16string());
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
    UpdateDownloadButton();
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
        else if (controlID == BUTTON_ID_UPLOAD)
        {
            UploadFiles();
        }
        else if (controlID == BUTTON_ID_DOWNLOAD)
        {
            DownloadCurrentFile();
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
    if (dlg.Show() != Dialogs::Result::Ok)
        return std::nullopt;
    auto result = dlg.GetResultedPath();
    // a sandboxed frontend (the browser) exports the file to the user once the application has written it
    auto app = Application::GetApplication();
    if ((app != nullptr) && (app->terminal != nullptr))
        app->terminal->OnSaveFileSelected(result);
    return result;
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