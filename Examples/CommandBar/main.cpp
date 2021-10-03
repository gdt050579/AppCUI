#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define COMMAND_ID_PRINT_HELLO_WORLD 1
#define COMMAND_ID_PRINT_A_NUMBER    2
#define COMMAND_ID_CLEAR_TEXT        3
#define COMMAND_ID_NUMPAD            4
#define COMMAND_ID_ENTER             5
#define COMMAND_ID_ALT               6
#define COMMAND_ID_ALT_CTRL          7
#define COMMAND_ID_CTRL_A            8
#define COMMAND_ID_CTRL_SHIFT_A      9

class MyWin : public AppCUI::Controls::Window
{
    Label *l1;

  public:
    MyWin() : Window("Command Bar Example", "d:c,w:60,h:10", WindowFlags::None)
    {
        l1 = Factory::Label::Create(*this, "", "x:1,y:2,w:56");
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Print Hello World", COMMAND_ID_PRINT_HELLO_WORLD);
        cmd.SetCommand(Key::F2, "Print a number", COMMAND_ID_PRINT_A_NUMBER);
        cmd.SetCommand(Key::Alt | Key::F1, "Clear text", COMMAND_ID_CLEAR_TEXT);
        cmd.SetCommand(Key::N1, "Numpad or digit", COMMAND_ID_NUMPAD);
        cmd.SetCommand(Key::Enter, "Some action", COMMAND_ID_ENTER);
        cmd.SetCommand(Key::Alt | Key::A, "Alt Mod Letter", COMMAND_ID_ALT);
        cmd.SetCommand(Key::Alt | Key::Ctrl | Key::F1, "Alt+Ctrl command pressed", COMMAND_ID_ALT_CTRL);
        cmd.SetCommand(Key::Ctrl | Key::A, "Ctrl+A command!", COMMAND_ID_CTRL_A);
        cmd.SetCommand(Key::Ctrl | Key::Shift | Key::A, "Ctrl+Shift+A command!", COMMAND_ID_CTRL_SHIFT_A);
        return true;
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
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
