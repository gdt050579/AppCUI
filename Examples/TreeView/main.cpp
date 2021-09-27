#include "AppCUI.hpp"
#include <chrono>
#include <string>
#include <locale>
#include <codecvt>
#include <charconv>

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
        const auto path                  = std::filesystem::current_path().u16string();
        const auto filename              = std::filesystem::current_path().filename().u16string();
        const auto pathLastWriteTime     = std::filesystem::last_write_time(std::filesystem::current_path());
        const auto pathLastWriteTimeText = GetLastFileWriteText(pathLastWriteTime);
        unsigned long long pathSize      = 0;
        std::u16string pathSizeText      = u"0";
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
              { filename, pathLastWriteTimeText, pathSizeText },
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

                    const auto path                  = std::filesystem::path(res->u16string());
                    const auto filename              = path.filename().u16string();
                    const auto pathLastWriteTime     = std::filesystem::last_write_time(path);
                    const auto pathLastWriteTimeText = GetLastFileWriteText(pathLastWriteTime);

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
                          { filename, pathLastWriteTimeText, pathSizeText },
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
                const auto filename              = p.path().filename().u16string();
                const auto pathLastWriteTimeText = GetLastFileWriteText(p.last_write_time());
                unsigned long long pathSize      = p.file_size();
                std::u16string pathSizeText      = u"0";
                try
                {
                    pathSizeText = GetTextFromNumber(pathSize);
                }
                catch (...)
                {
                }
                tree.AddItem(
                      handle,
                      { filename, pathLastWriteTimeText, pathSizeText },
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

    static const std::u16string GetLastFileWriteText(const std::filesystem::file_time_type& ftime)
    {
        std::time_t cftime =
              std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(ftime));

        const auto local = std::localtime(&cftime);
        if (local == nullptr)
        {
            return u"";
        }

        const auto asc = std::asctime(local);
        if (asc == nullptr)
        {
            return u"";
        }

        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        const auto size = strlen(asc);
        return convert.from_bytes(asc, asc + (size - 1)); // ignore new line
    };

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