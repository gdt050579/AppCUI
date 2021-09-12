#include "AppCUI.hpp"

#include <cuchar>

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
        Create("Folder tree example", "d:c, w:80%, h:60%");
        open.Create(this, "&Open", "x:1%, y:6%, w:10%", static_cast<unsigned int>(ControlIds::ButtonShowOpen));
        vertical.Create(this, "x:6%, y:0, w:11%, h:15%", true);
        horizontal.Create(this, "x:1%, y:15%, w:99%, h:5%", false);
        currentFolder.Create(this, std::filesystem::current_path().u8string(), "x:12%, y:1%, h:15%, w:87%");
        tree.Create(this, "x:1%, y:20%, w:99%, h:20%");
        tree.SetValue(std::filesystem::current_path().string());
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
                const auto res = FolderDialog::ShowOpenFileWindow("", currentFolder.GetText());
                if (res.has_value())
                {
                    const auto value = res->u8string();

                    currentFolder.SetText(value);

                    std::string valueString;
                    std::mbstate_t state{ 0 };
                    char out[MB_LEN_MAX]{ 0 };
                    for (const auto& c16 : value)
                    {
                        const std::size_t rc = std::c16rtomb(out, c16, &state);
                        if (rc == static_cast<std::size_t>(-1))
                        {
                            continue;
                        }

                        for (const char c8 : std::string_view{ out, rc })
                        {
                            valueString += c8;
                        }
                    }

                    tree.SetValue(valueString);
                }

                return true;
            }
            }
        }

        return false;
    }
};

int main()
{
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(new ExampleMainWindow());
    Application::Run();

    return 0;
}