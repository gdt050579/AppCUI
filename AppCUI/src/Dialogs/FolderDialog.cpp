#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::OS;
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#    include <time.h>
#endif

namespace FolderDialogUtils
{
// Currently not all compilers support clock_cast (including gcc)
// AppleClang supports std::chrono::file_clock::to_time_t, but gcc or VS doesn't
// Have this frankenstein's monster while the compilers update
//
// Normally, we should be able to convert using clock_cast or a to_time_t kind of function
// to say we have full support
std::time_t GetLastModifiedTime(const std::filesystem::directory_entry& entry)
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
} // namespace FolderDialogUtils

struct FolderDialogClass
{
    Window window;
    Label path, drive, folderName;
    TextField fName;
    ComboBox comboDrive;
    ListView folders;
    Button ok, cancel;
    std::vector<std::pair<std::string, std::filesystem::path>> specialFolders;
    std::filesystem::path resultedPath;

    int Show(const ConstString& folderName, const std::filesystem::path& _path);
    void UpdateCurrentFolder();
    void UpdateFolderList();
    bool OnEventHandler(const void* sender, Event eventType, int controlID);
    void Validate();
    void OnClickedOnItem();
    void OnCurrentItemChanged();
};

bool FolderDialog_EventHandler(
      Control* control, const void* sender, Event eventType, int controlID, void* /* Context */)
{
    return ((FolderDialogClass*) control)->OnEventHandler(sender, eventType, controlID);
}

void FolderDialogClass::OnClickedOnItem()
{
    const unsigned int index{ static_cast<unsigned int>(folders.GetCurrentItem()) };
    if (index == 0xFFFFFFFF)
    {
        return;
    }

    const unsigned int value{ folders.GetItemData(index)->UInt32Value };
    std::filesystem::path itemPath{ path.GetText() };

    switch (value)
    {
    case 0:
        path.SetText(itemPath.parent_path().u8string());
        fName.SetText("");
        break;
    case 1:
        itemPath /= folders.GetItemText(index, 0);
        path.SetText(itemPath.u8string());
        fName.SetText("");
        break;
    default:
        break;
    }

    UpdateFolderList();
}

void FolderDialogClass::OnCurrentItemChanged()
{
    const unsigned int index{ folders.GetCurrentItem() };
    if (index == 0xFFFFFFFF)
    {
        return;
    }

    const unsigned int value{ folders.GetItemData(index)->UInt32Value };
    switch (value)
    {
    case 1:
        fName.SetText(folders.GetItemText(index, 0));
        break;
    default:
        fName.SetText("");
        break;
    }
}

void FolderDialogClass::Validate()
{
    if (path.GetText().Len() == 0)
    {
        return;
    }

    path.GetText().ToPath(this->resultedPath);

    if (fName.GetText().Len() != 0)
    {
        this->resultedPath /= fName.GetText();
    }

    if (std::filesystem::exists(this->resultedPath) == false)
    {
        MessageBox::ShowError("Error", "Selected file does not exists !");
        return;
    }

    window.Exit(Dialogs::Result::Ok);
}

void FolderDialogClass::UpdateCurrentFolder()
{
    const unsigned int i{ comboDrive.GetCurrentItemUserData().UInt32Value };
    path.SetText(reinterpret_cast<const char*>(specialFolders[i].second.u8string().c_str()));
    UpdateFolderList();
}

void FolderDialogClass::UpdateFolderList()
{
    folders.DeleteAllItems();

    std::string currentPath;
    if (path.GetText().ToString(currentPath) == false)
    {
        return;
    }

    const std::filesystem::path path{ currentPath };
    if (path != path.root_path())
    {
        folders.AddItem("..", "UP-DIR");
        folders.SetItemData(0, ItemData{ 0 });
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory() == false)
            {
                continue;
            }

            char dateBuffer[64]{ 0 };
            const time_t date{ FolderDialogUtils::GetLastModifiedTime(entry) };
            struct tm t;
#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
            localtime_r(&lastModifiedTime, &t); // TODO: errno not treated
            strftime(lastModifiedTimeBuffer, sizeof(lastModifiedTimeBuffer), "%Y-%m-%d  %H:%M:%S", &t);
#else
            localtime_s(&t, &date); // TODO: errno not treated
            std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d  %H:%M:%S", &t);
#endif
            const ItemHandle itemHandle = folders.AddItem(
                  reinterpret_cast<const char*>(entry.path().filename().u8string().c_str()), "Folder", dateBuffer);

            folders.SetItemColor(itemHandle, ColorPair{ Color::White, Color::Transparent });
            folders.SetItemData(itemHandle, ItemData{ 1 });
        }
    }
    catch (...)
    {
        // for the moment skip
    }

    folders.Sort();
}

bool FolderDialogClass::OnEventHandler(const void* sender, Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::ButtonClicked:
        if (controlID == (int) Dialogs::Result::Ok)
            Validate();
        else
            window.Exit(controlID);
        return true;
    case Event::WindowClose:
        window.Exit(Dialogs::Result::Cancel);
        return true;
    case Event::WindowAccept:
        Validate();
        return true;
    case Event::ComboBoxSelectedItemChanged:
        if (sender == &comboDrive)
        {
            UpdateCurrentFolder();
            UpdateFolderList();
        }
        return true;
    case Event::TextFieldValidate:
        UpdateFolderList();
        folders.SetFocus();
        return true;
    case Event::ListViewCurrentItemChanged:
        OnCurrentItemChanged();
        return true;
    case Event::ListViewItemClicked:
        OnClickedOnItem();
        return true;
    }
    return true;
}

int FolderDialogClass::Show(const ConstString& folderName, const std::filesystem::path& _path)
{
    window.Create("Open", "w:78, h:23, d:c");
    window.SetEventHandler(FolderDialog_EventHandler, this);

    path.Create(&window, "", "x:2, y:2, w:63");

    drive.Create(&window, "&Location", "x:2, y:1, w:8");

    comboDrive.Create(&window, "x:12, y:1, w:61");
    comboDrive.SetHotKey('L');

    // populate combo box with special folders and available drivers
    this->specialFolders.clear();
    GetSpecialFolders(this->specialFolders, SpecialFoldersType::Drives, false);
    if (this->specialFolders.size() > 0)
    {
        comboDrive.AddSeparator("Drives");
        for (auto i = 0U; i < this->specialFolders.size(); i++)
        {
            comboDrive.AddItem(this->specialFolders[i].first.c_str(), ItemData{ static_cast<unsigned long long>(i) });
        }
    }

    auto lastSize = this->specialFolders.size();
    GetSpecialFolders(this->specialFolders, SpecialFoldersType::SpecialLocations, false);
    if (this->specialFolders.size() > lastSize)
    {
        comboDrive.AddSeparator("Locations");
        for (auto i = lastSize; i < this->specialFolders.size(); i++)
        {
            comboDrive.AddItem(this->specialFolders[i].first.c_str(), ItemData{ i });
        }
    }

    folders.Create(&window, "x:2, y:3, w:72, h:13", ListViewFlags::Sortable);
    folders.AddColumn("&Name", TextAlignament::Left, 31);
    folders.AddColumn("&Size", TextAlignament::Right, 16);
    folders.AddColumn("&Modified", TextAlignament::Center, 20);
    folders.SetItemCompareFunction(
          [](ListView* lv, ItemHandle i01, ItemHandle i02, unsigned int colIndex, void* /* ctx */)
          {
              const auto v1 = lv->GetItemData(i01)->UInt64Value;
              const auto v2 = lv->GetItemData(i02)->UInt64Value;

              if (v1 < v2)
              {
                  return -1;
              }
              else if (v1 > v2)
              {
                  return 1;
              }
              else
              {
                  auto& s1 = lv->GetItemText(i01, colIndex);
                  auto& s2 = lv->GetItemText(i02, colIndex);
                  return s1.CompareWith(s2, true);
              }
          },
          this);
    folders.Sort(0, true); // sort after the first column, ascendent

    this->folderName.Create(&window, "Folder &Name", "x:2, y:17, w:11");
    fName.Create(&window, folderName, "x:14, y:17, w:80%");
    fName.SetHotKey('N');

    ok.Create(&window, "&Ok", "x:25%, y:19, w:13", (int) Dialogs::Result::Ok);
    cancel.Create(&window, "&Cancel", "x:45%, y:19, w:13", (int) Dialogs::Result::Cancel);
    try
    {
        if (_path.empty())
        {
            path.SetText(std::filesystem::absolute(".").u8string());
        }
        else
        {
            path.SetText(std::filesystem::absolute(_path).u8string());
        }
    }
    catch (...)
    {
        path.SetText(std::filesystem::absolute(".").u8string());
    }

    UpdateFolderList();
    fName.SetFocus();

    return window.Show();
}

std::optional<std::filesystem::path> FolderDialog::ShowOpenFileWindow(
      const ConstString& folderName, const std::filesystem::path& path)
{
    FolderDialogClass dlg;
    if (dlg.Show(folderName, path) == static_cast<int>(Dialogs::Result::Ok))
    {
        return dlg.resultedPath;
    }

    return std::nullopt;
}
