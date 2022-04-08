#include "AppCUI.hpp"
#include <cstring>
#include <string>
#include <charconv>

#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#endif

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Dialogs;
using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;

class TreeExample : public Window, public Handlers::OnTreeItemToggleInterface
{
    enum class ControlIds : uint32
    {
        ButtonShowOpen = 1
    };

    Reference<Button> open;
    Reference<TextField> currentFolder;
    Reference<Splitter> vertical;
    Reference<Splitter> horizontal;
    Reference<TreeView> tree;

  public:
    TreeExample() : Window("Tree view example", "d:c, w:100%, h:100%", WindowFlags::Sizeable)
    {
        open = Factory::Button::Create(
              this, "&Open", "x:1%, y:6%, w:10%", static_cast<uint32>(ControlIds::ButtonShowOpen));
        vertical   = Factory::Splitter::Create(this, "x:1%, y:0, w:11%, h:15%", SplitterFlags::Vertical);
        horizontal = Factory::Splitter::Create(this, "x:1%, y:15%, w:99%, h:5%");
        currentFolder =
              Factory::TextField::Create(this, std::filesystem::current_path().u8string(), "x:12%, y:1%, h:15%, w:87%");
        tree = Factory::TreeView::Create(
              this,
              "x:1%, y:20%, w:99%, h:80%",
              (TreeViewFlags::DynamicallyPopulateNodeChildren | TreeViewFlags::Sortable),
              3);

        tree->Handlers()->OnTreeItemToggle = this;

        tree->AddColumnData(0, u"&Path", TextAlignament::Left, TextAlignament::Left, 200);
        tree->AddColumnData(1, u"&Last Write Time", TextAlignament::Right, TextAlignament::Right, 25);
        tree->AddColumnData(2, u"&Size (bytes)", TextAlignament::Right, TextAlignament::Right, 25);

        tree->ClearItems();
        const auto path              = std::filesystem::current_path().u16string();
        const auto filename          = std::filesystem::current_path().filename().u16string();
        const auto pathLastWriteTime = GetLastFileWriteText(std::filesystem::current_path());
        std::string pathSizeText;
        try
        {
            const auto pathSize = std::filesystem::file_size(path);
            pathSizeText        = GetTextFromNumber(pathSize);
        }
        catch (...)
        {
            pathSizeText = "0";
        }

        const auto cpath = std::filesystem::current_path().u16string();
        const auto root  = tree->AddItem(
              AppCUI::Controls::InvalidItemHandle,
              { filename, pathLastWriteTime, pathSizeText },
              cpath,
              false,
              std::filesystem::is_directory(path));
    }

    bool OnEvent(Reference<Control>, Event eventType, int controlID) override
    {
        switch (eventType)
        {
        case Event::WindowClose:
            Application::Close();
            return true;
        case Event::ButtonClicked:
            switch (static_cast<ControlIds>(controlID))
            {
            case ControlIds::ButtonShowOpen:
            {
                const auto res = FileDialog::ShowOpenFileWindow("", "", currentFolder->GetText());
                if (res.has_value())
                {
                    currentFolder->SetText(res->u8string());
                    tree->ClearItems();

                    const auto path              = std::filesystem::path(res->u16string());
                    const auto filename          = path.filename().u16string();
                    const auto pathLastWriteTime = GetLastFileWriteText(path);

                    std::string pathSizeText;
                    try
                    {
                        const auto pathSize = std::filesystem::file_size(path);
                        pathSizeText        = GetTextFromNumber(pathSize);
                    }
                    catch (...)
                    {
                        pathSizeText = "0";
                    }

                    const auto localPath = path.u16string();
                    const auto root      = tree->AddItem(
                          TreeView::RootItemHandle,
                          { filename, pathLastWriteTime, pathSizeText },
                          localPath,
                          false,
                          std::filesystem::is_directory(path));

                    auto& metadata = tree->GetItemMetadata(root);
                    UnicodeStringBuilder usb;
                    usb.Add(ConstString{ metadata });
                    usb.Add(res->u16string());
                    tree->SetItemMetadata(root, usb);
                    OnTreeItemToggle(tree, root);
                }

                return true;
            }
            }
        }

        return false;
    }

    bool OnTreeItemToggle(Reference<TreeView> ctrl, ItemHandle handle) override
    {
        const auto& usb = ctrl->GetItemMetadata(handle);
        std::u16string u16Path;
        usb.ToString(u16Path);
        const auto fsPath = std::filesystem::path(u16Path);
        try
        {
            const auto rdi = std::filesystem::directory_iterator(fsPath);
            for (const auto& p : rdi)
            {
                const auto filename          = p.path().filename().u16string();
                const auto pathLastWriteTime = GetLastFileWriteText(p.path());
                uint64 pathSize              = p.file_size();
                const auto pathSizeText      = GetTextFromNumber(pathSize);
                const auto cpath             = p.path().u16string();

                ctrl->AddItem(handle, { filename, pathLastWriteTime, pathSizeText }, cpath, false, p.is_directory());
            }
        }
        catch (std::exception& e)
        {
            LOG_ERROR("%s", e.what());
        }

        return true;
    }

    static const std::string GetLastFileWriteText(const std::filesystem::path& path)
    {
        char dateBuffer[64]{ 0 };
        const time_t date{ GetLastModifiedTime(path) };
        struct tm t;
#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
        localtime_r(&date, &t); // TODO: errno not treated
        strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d  %H:%M:%S", &t);
#else
        localtime_s(&t, &date); // TODO: errno not treated
        std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d  %H:%M:%S", &t);
#endif
        return dateBuffer;
    };

    static std::time_t GetLastModifiedTime(const std::filesystem::path& path)
    {
#if BUILD_FOR_WINDOWS
        auto lastTime = std::filesystem::last_write_time(path);
        return std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(lastTime));
#elif BUILD_FOR_OSX
        struct stat attr;
        stat(path.string().c_str(), &attr);
        return attr.st_mtimespec.tv_sec;
#elif BUILD_FOR_UNIX
        struct stat attr;
        stat(path.string().c_str(), &attr);
        return attr.st_mtime;
#endif
    }

    static const std::string GetTextFromNumber(const uint64 value)
    {
        constexpr auto SIZE = 20;
        char cValue[SIZE]   = { 0 };

        if (auto [ptr, ec] = std::to_chars(cValue, cValue + SIZE, value); ec == std::errc())
        {
            return cValue;
        }

        return "0";
    }
};

int main()
{
    CHECK(Init(), 1, "");

    AddWindow(std::make_unique<TreeExample>());
    Run();

    return 0;
}