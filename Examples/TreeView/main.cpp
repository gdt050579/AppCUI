#include "AppCUI.hpp"
#include <cstring>
#include <string>
#include <charconv>

#if defined(BUILD_FOR_OSX) || defined(BUILD_FOR_UNIX)
#    include <sys/stat.h>
#endif

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;
using namespace AppCUI::Graphics;

class TreeExample : public AppCUI::Controls::Window
{
    enum class ControlIds : unsigned int
    {
        ButtonShowOpen = 1
    };

    Reference<Button> open;
    Reference<TextField> currentFolder;
    Reference<Splitter> vertical;
    Reference<Splitter> horizontal;
    Reference<Tree> tree;

  public:
    TreeExample() : Window("Tree view example", "d:c, w:100%, h:100%", WindowFlags::Sizeable)
    {
        open = Factory::Button::Create(
              this, "&Open", "x:1%, y:6%, w:10%", static_cast<unsigned int>(ControlIds::ButtonShowOpen));
        vertical   = Factory::Splitter::Create(this, "x:1%, y:0, w:11%, h:15%", true);
        horizontal = Factory::Splitter::Create(this, "x:1%, y:15%, w:99%, h:5%", false);
        currentFolder =
              Factory::TextField::Create(this, std::filesystem::current_path().u8string(), "x:12%, y:1%, h:15%, w:87%");
        tree = Factory::Tree::Create(
              this,
              "x:1%, y:20%, w:99%, h:80%",
              (TreeFlags::DynamicallyPopulateNodeChildren | TreeFlags::SearchMode),
              3);

        // TODO: maybe add % for column sizes as well
        tree->AddColumnData(
              0, u"Path", AppCUI::Graphics::TextAlignament::Left, AppCUI::Graphics::TextAlignament::Left, 100);
        tree->AddColumnData(
              1, u"Last Write Time", AppCUI::Graphics::TextAlignament::Left, AppCUI::Graphics::TextAlignament::Left);
        tree->AddColumnData(2, u"Size", AppCUI::Graphics::TextAlignament::Left, AppCUI::Graphics::TextAlignament::Left);
        tree->SetToggleItemHandle(PopulateTree);

        tree->ClearItems();
        const auto path = std::filesystem::current_path().u16string();
        CharacterBuffer filename;
        filename.Set(std::filesystem::current_path().filename().u16string());
        const auto pathLastWriteTime = GetLastFileWriteText(std::filesystem::current_path());
        unsigned long long pathSize  = 0;
        CharacterBuffer pathSizeText;
        try
        {
            pathSize     = std::filesystem::file_size(path);
            pathSizeText = GetTextFromNumber(pathSize);
        }
        catch (...)
        {
            pathSizeText.Add("0");
        }

        const auto cpath = std::filesystem::current_path().u16string();
        const auto root  = tree->AddItem(
              InvalidItemHandle,
              { filename, *const_cast<CharacterBuffer*>(&pathLastWriteTime), pathSizeText },
              cpath,
              nullptr,
              false,
              std::filesystem::is_directory(path));

        CharacterBuffer cb;
        cb.Add(path);
        PopulateTree(tree, root, &cb);
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
                const auto res = FileDialog::ShowOpenFileWindow("", "", currentFolder->GetText());
                if (res.has_value())
                {
                    currentFolder->SetText(res->u8string());
                    tree->ClearItems();

                    const auto path = std::filesystem::path(res->u16string());
                    CharacterBuffer filename;
                    filename.Set(path.filename().u16string());
                    const auto pathLastWriteTime = GetLastFileWriteText(path);

                    unsigned long long pathSize = 0;
                    CharacterBuffer pathSizeText;
                    try
                    {
                        pathSize     = std::filesystem::file_size(path);
                        pathSizeText = GetTextFromNumber(pathSize);
                    }
                    catch (...)
                    {
                        pathSizeText.Add("0");
                    }

                    const auto localPath = path.u16string();
                    const auto root      = tree->AddItem(
                          InvalidItemHandle,
                          { filename, *const_cast<CharacterBuffer*>(&pathLastWriteTime), pathSizeText },
                          localPath,
                          nullptr,
                          false,
                          std::filesystem::is_directory(path));

                    CharacterBuffer cb;
                    cb.Add(res->u16string());
                    PopulateTree(tree, root, &cb);
                }

                return true;
            }
            }
        }

        return false;
    }

    static bool PopulateTree(Tree& tree, const ItemHandle handle, const void* context)
    {
        const auto cb = reinterpret_cast<CharacterBuffer*>(const_cast<void*>(context));
        std::u16string u16Path;
        CHECK(cb->ToString(u16Path), false, "");
        const auto fsPath = std::filesystem::path(u16Path);
        try
        {
            const auto rdi = std::filesystem::directory_iterator(fsPath);
            for (const auto& p : rdi)
            {
                CharacterBuffer filename;
                filename.Set(p.path().filename().u16string());
                const auto pathLastWriteTime = GetLastFileWriteText(p.path());
                unsigned long long pathSize  = p.file_size();
                const auto pathSizeText      = GetTextFromNumber(pathSize);
                const auto cpath             = p.path().u16string();
                tree.AddItem(
                      handle,
                      { filename,
                        *const_cast<CharacterBuffer*>(&pathLastWriteTime),
                        *const_cast<CharacterBuffer*>(&pathSizeText) },
                      cpath,
                      nullptr,
                      false,
                      p.is_directory());
            }
        }
        catch (std::exception e)
        {
            LOG_ERROR("%s", e.what());
        }

        return true;
    }

    static const CharacterBuffer GetLastFileWriteText(const std::filesystem::path& path)
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
        CharacterBuffer cb;
        cb.Add(dateBuffer);
        return cb;
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

    static const CharacterBuffer GetTextFromNumber(const unsigned long long value)
    {
        constexpr auto SIZE = 20;
        char cValue[SIZE]   = { 0 };
        CharacterBuffer cb;

        if (auto [ptr, ec] = std::to_chars(cValue, cValue + SIZE, value); ec == std::errc())
        {
            cb.Add(cValue);
        }
        else
        {
            cb.Add("0");
        }

        return cb;
    }
};

int main()
{
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(std::make_unique<TreeExample>());
    Application::Run();

    return 0;
}