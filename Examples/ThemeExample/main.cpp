#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

constexpr int CMD_CHANGE_THEME_DEFAULT = 0;
constexpr int CMD_CHANGE_THEME_DARK    = 1;
constexpr int CMD_CHANGE_THEME_LIGHT   = 2;

class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Test", "d:c,w:70,h:24", WindowFlags::None)
    {
        Factory::Label::Create(this, "This is an example of how themes work on AppCUI", "x:8,y:1,w:66");
        auto p = Factory::Panel::Create(this, "Themes", "x:2,y:3,w:20,h:5");
        Factory::RadioBox::Create(p, "Default", "x:1,y:0,w:15", 100, CMD_CHANGE_THEME_DEFAULT)->SetChecked(true);
        Factory::RadioBox::Create(p, "Dark", "x:1,y:1,w:15", 100, CMD_CHANGE_THEME_DARK);
        Factory::RadioBox::Create(p, "Light", "x:1,y:2,w:15", 100, CMD_CHANGE_THEME_LIGHT);
    }
    bool OnEvent(Reference<Control> control, Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;
        if (eventType == Event::CheckedStatusChanged)
        {
            switch (ID)
            {
            case CMD_CHANGE_THEME_DEFAULT:
                Application::SetTheme(Application::ThemeType::Default);
                return true;
            case CMD_CHANGE_THEME_DARK:
                Application::SetTheme(Application::ThemeType::Dark);
                return true;
            case CMD_CHANGE_THEME_LIGHT:
                Application::SetTheme(Application::ThemeType::Light);
                return true;
            }
        }
        return false;
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Help", 1);
        cmd.SetCommand(Key::F2, "Save", 1);
        cmd.SetCommand(Key::F3, "Load", 1);
        cmd.SetCommand(Key::F4, "Edit", 1);
        return true;
    }
};
void BuildAppMenu()
{
    Menu* files = Application::AddMenu("&File");
    files->AddCommandItem("&Save", 100, Key::Ctrl | Key::S);
    files->AddCommandItem("Save All", 101);
    files->AddCommandItem("&Open", 102, Key::Ctrl | Key::O);
    files->AddSeparator();
    files->AddCommandItem("E&xit", 103, Key::Ctrl | Key::Q);
    Menu* edit = Application::AddMenu("&Edit");
    edit->AddCommandItem("&Copy", 200, Key::Ctrl | Key::Insert);
    edit->AddCommandItem("&paste", 201, Key::Shift | Key::Insert);
    edit->AddCommandItem("&Cut", 202, Key::Shift | Key::Delete);
    edit->AddCommandItem("Special copy", 203, Key::Ctrl | Key::Alt | Key::Shift | Key::Insert);
    Menu* help = Application::AddMenu("&Help");
    help->AddCommandItem("&About", 300);
    help->AddCommandItem("Search &online", 301);
    help->AddSeparator();
    help->AddCommandItem("Check for &updates", 302);
}
int main()
{
    InitializationData initData;
    initData.Flags = InitializationFlags::Menu | InitializationFlags::CommandBar;
    initData.Theme = ThemeType::Default;
    if (!Application::Init(initData))
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    BuildAppMenu();
    Application::Run();
    return 0;
}
