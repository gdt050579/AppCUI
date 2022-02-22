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
int main()
{
    InitializationData initData;
    initData.Flags = InitializationFlags::Menu | InitializationFlags::CommandBar;
    initData.Theme = ThemeType::Default;
    if (!Application::Init(initData))
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    Application::Run();
    return 0;
}
