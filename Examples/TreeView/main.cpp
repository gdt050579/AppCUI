#include "AppCUI.hpp"
#include <cstring>
#include <string>
#include <locale>
#include <codecvt>
#include <charconv>

#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#endif

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

class ExampleMainWindow : public AppCUI::Controls::Window
{
    enum class ControlIds : unsigned int
    {
        ButtonShowOpen = 1
    };

    Button open;
    TextField currentFolder;
    Splitter vertical;
    Splitter horizontal;
    Tree tree;

  public:
    ExampleMainWindow()
    {
        Create("Tree view example", "d:c, w:80%, h:60%");
        open.Create(this, "&Open", "x:1%, y:6%, w:10%", static_cast<unsigned int>(ControlIds::ButtonShowOpen));
        vertical.Create(this, "x:6%, y:0, w:11%, h:15%", true);
        horizontal.Create(this, "x:1%, y:15%, w:99%, h:5%", false);
        currentFolder.Create(this, std::filesystem::current_path().u8string(), "x:12%, y:1%, h:15%, w:87%");
        tree.Create(
              this,
              "x:1%, y:20%, w:99%, h:85%",
              static_cast<unsigned int>(TreeFlags::DynamicallyPopulateNodeChildren) |
                    static_cast<unsigned int>(TreeFlags::HideScrollBar),
              { u"Path", u"Last Write Time", u"Size" });
        tree.SetToggleItemHandle(PopulateTree);

        tree.ClearItems();
        const auto path              = std::filesystem::current_path().u16string();
        const auto filename          = std::filesystem::current_path().filename().u16string();
        const auto pathLastWriteTime = GetLastFileWriteText(std::filesystem::current_path());
        unsigned long long pathSize  = 0;
        std::u16string pathSizeText  = u"0";
        try
        {
            pathSize     = std::filesystem::file_size(path);
            pathSizeText = GetTextFromNumber(pathSize);
        }
        catch (...)
        {
        }
        const auto root = tree.AddItem(
              InvalidItemHandle,
              { filename, pathLastWriteTime, pathSizeText },
              nullptr,
              false,
              std::filesystem::current_path().u16string(),
              std::filesystem::is_directory(path));
        PopulateTree(tree, root, &path);
    }

    bool OnEvent(Control*, Event eventType, int controlID) override
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
                const auto res = FileDialog::ShowOpenFileWindow("", "", currentFolder.GetText());
                if (res.has_value())
                {
                    currentFolder.SetText(res->u8string());
                    tree.ClearItems();

                    const auto path              = std::filesystem::path(res->u16string());
                    const auto filename          = path.filename().u16string();
                    const auto pathLastWriteTime = GetLastFileWriteText(path);

                    unsigned long long pathSize = 0;
                    std::u16string pathSizeText = u"0";
                    try
                    {
                        pathSize     = std::filesystem::file_size(path);
                        pathSizeText = GetTextFromNumber(pathSize);
                    }
                    catch (...)
                    {
                    }

                    const auto root = tree.AddItem(
                          InvalidItemHandle,
                          { filename, pathLastWriteTime, pathSizeText },
                          nullptr,
                          false,
                          path.u16string(),
                          std::filesystem::is_directory(path));
                    PopulateTree(tree, root, &path);
                }

                return true;
            }
            }
        }

        return false;
    }

    static bool PopulateTree(Tree& tree, const ItemHandle handle, const void* context)
    {
        const auto fsPath = std::filesystem::path(*reinterpret_cast<std::u16string*>(const_cast<void*>(context)));
        try
        {
            const auto rdi = std::filesystem::directory_iterator(fsPath);
            for (const auto& p : rdi)
            {
                const auto filename          = p.path().filename().u16string();
                const auto pathLastWriteTime = GetLastFileWriteText(p.path());
                unsigned long long pathSize  = p.file_size();
                std::u16string pathSizeText  = u"0";
                try
                {
                    pathSizeText = GetTextFromNumber(pathSize);
                }
                catch (...)
                {
                }
                tree.AddItem(
                      handle,
                      { filename, pathLastWriteTime, pathSizeText },
                      nullptr,
                      false,
                      p.path().u16string(),
                      p.is_directory());
            }
        }
        catch (std::exception e)
        {
            LOG_ERROR("%s", e.what());
        }

        return true;
    }

    static const std::u16string GetLastFileWriteText(const std::filesystem::path& path)
    {
        try
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
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
            const auto size = strlen(dateBuffer);
            return convert.from_bytes(dateBuffer, dateBuffer + size);
        }
        catch(...)
        {
            return u"ERROR";
        }
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

    static const std::u16string GetTextFromNumber(const unsigned long long value)
    {
        constexpr auto SIZE = 20;
        char cValue[SIZE]   = { 0 };

        if (auto [ptr, ec] = std::to_chars(cValue, cValue + SIZE, value); ec == std::errc())
        {
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
            return convert.from_bytes(cValue, cValue + SIZE);
        }
        else
        {
            return u"0";
        }
    }
};

int main()
{
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(std::make_unique<ExampleMainWindow>());
    Application::Run();

    return 0;
}