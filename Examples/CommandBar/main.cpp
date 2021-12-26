#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;

#define COMMAND_ID_PRINT_HELLO_WORLD 1
#define COMMAND_ID_PRINT_A_NUMBER    2
#define COMMAND_ID_CLEAR_TEXT        3
#define COMMAND_ID_NUMPAD            4
#define COMMAND_ID_ENTER             5
#define COMMAND_ID_ALT               6
#define COMMAND_ID_ALT_CTRL          7
#define COMMAND_ID_CTRL_A            8
#define COMMAND_ID_CTRL_SHIFT_A      9
#define COMMAND_ID_CHANGE_STATUS     10

int cmp_ID = 0;

class MyUserControl : public UserControl
{
    Key k;

  public:
    MyUserControl(std::string_view layout) : UserControl("", layout)
    {
        switch (cmp_ID)
        {
        case 0:
            k = Key ::F5;
            break;
        case 1:
            k = Key::F6;
            break;
        case 2:
            k = Key::F7;
            break;
        default:
            k = Key::F8;
            break;
        }
        cmp_ID++;
    }
    void Paint(AppCUI::Graphics::Renderer& r) override
    {
        if (this->HasFocus())
            r.Clear('x', ColorPair{ Color::White, Color::Red });
        else
            r.Clear('.', ColorPair{ Color::White, Color::Blue });
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(k, "User Command Specific", COMMAND_ID_PRINT_HELLO_WORLD);
        return false;
    }
};
class MyUserControl2 : public UserControl
{
  public:
    MyUserControl2(std::string_view layout) : UserControl("", layout)
    {
    }
    void Paint(AppCUI::Graphics::Renderer& r) override
    {
        if (this->HasFocus())
            r.Clear('#', ColorPair{ Color::White, Color::DarkGreen });
        else
            r.Clear('.', ColorPair{ Color::White, Color::Blue });
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F5, "Just this command", COMMAND_ID_PRINT_HELLO_WORLD);
        return true;
    }
};
class MyWin : public AppCUI::Controls::Window
{
    Reference<Label> l1;
    bool status;

  public:
    MyWin() : Window("Command Bar Example", "d:c,w:60,h:25", WindowFlags::None)
    {
        l1      = Factory::Label::Create(this, "", "x:1,y:2,w:56");
        auto t  = Factory::Tab::Create(this, "x:1,y:3,w:56,h:14");
        auto tp = Factory::TabPage::Create(t, "TabPage");
        auto v  = Factory::Splitter::Create(tp, "d:c", false);
        auto h  = Factory::Splitter::Create(v, "x:1,y:4,w:56,h:10", true);
        Factory::Button::Create(this, "A botton", "l:1,b:1,w:20", 1234);
        h->CreateChildControl<MyUserControl>("x:1,y:4,w:56,h:10");
        v->CreateChildControl<MyUserControl2>("x:1,y:4,w:56,h:10");

        auto t2  = Factory::Tab::Create(h, "d:c");
        auto tp2 = Factory::TabPage::Create(t2, "TabPage");
        tp2->CreateChildControl<MyUserControl>("d:c");

        status = true;
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Print Hello World", COMMAND_ID_PRINT_HELLO_WORLD);
        cmd.SetCommand(Key::F2, "Print a number", COMMAND_ID_PRINT_A_NUMBER);
        if (status)
            cmd.SetCommand(Key::F3, "Status:ON", COMMAND_ID_CHANGE_STATUS);
        else
            cmd.SetCommand(Key::F3, "Status:OFF", COMMAND_ID_CHANGE_STATUS);
        cmd.SetCommand(Key::Alt | Key::F1, "Clear text", COMMAND_ID_CLEAR_TEXT);
        cmd.SetCommand(Key::N1, "Numpad or digit", COMMAND_ID_NUMPAD);
        cmd.SetCommand(Key::Enter, "Some action", COMMAND_ID_ENTER);
        cmd.SetCommand(Key::Alt | Key::A, "Alt Mod Letter", COMMAND_ID_ALT);
        cmd.SetCommand(Key::Alt | Key::Ctrl | Key::F1, "Alt+Ctrl command pressed", COMMAND_ID_ALT_CTRL);
        cmd.SetCommand(Key::Ctrl | Key::A, "Ctrl+A command!", COMMAND_ID_CTRL_A);
        cmd.SetCommand(Key::Ctrl | Key::Shift | Key::A, "Ctrl+Shift+A command!", COMMAND_ID_CTRL_SHIFT_A);
        return true;
    }
    bool OnEvent(Reference<Control>, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Command)
        {
            switch (controlID)
            {
            case COMMAND_ID_PRINT_HELLO_WORLD:
                l1->SetText("Hello world");
                break;
            case COMMAND_ID_PRINT_A_NUMBER:
                l1->SetText("12345");
                break;
            case COMMAND_ID_CHANGE_STATUS:
                status = !status;
                return true;
            case COMMAND_ID_CLEAR_TEXT:
                l1->SetText("");
                break;
            case COMMAND_ID_NUMPAD:
                l1->SetText("Numpad pressed!");
                break;
            case COMMAND_ID_ENTER:
                l1->SetText("Enter pressed!");
                break;
            case COMMAND_ID_ALT:
                l1->SetText("Alt modifier!");
                break;
            case COMMAND_ID_ALT_CTRL:
                l1->SetText("Alt+Ctrl+F1 modifier!");
                break;
            case COMMAND_ID_CTRL_A:
                l1->SetText("Ctrl+A modifier");
                break;
            case COMMAND_ID_CTRL_SHIFT_A:
                l1->SetText("Ctrl+Shift+A modifier");
                break;
            }
        }
        return false;
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
