#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define COMMAND_ID_PRINT_HELLO_WORLD    1
#define COMMAND_ID_PRINT_A_NUMBER       2
#define COMMAND_ID_CLEAR_TEXT           3
#define COMMAND_ID_NUMPAD               4
#define COMMAND_ID_ENTER                5
#define COMMAND_ID_ALT                  6
#define COMMAND_ID_ALT_CTRL             7

class MyWin : public AppCUI::Controls::Window
{
    Label l1;
public:
    MyWin()
    {
        this->Create("Command Bar Example", "a:c,w:60,h:10");
        l1.Create(this, "", "x:1,y:2,w:56");
    }
    bool OnUpdateCommandBar(CommandBar & cmd) override
    {
        cmd.SetCommand(Key::F1, "Print Hello World", COMMAND_ID_PRINT_HELLO_WORLD);
        cmd.SetCommand(Key::F2, "Print a number", COMMAND_ID_PRINT_A_NUMBER);
        cmd.SetCommand(Key::Alt | Key::F1, "Clear text", COMMAND_ID_CLEAR_TEXT);
        cmd.SetCommand(Key::N1, "Numpad or digit", COMMAND_ID_NUMPAD);
        cmd.SetCommand(Key::Enter, "Enter", COMMAND_ID_ENTER);
        cmd.SetCommand(Key::Alt|Key::A, "Alt Mod Letter", COMMAND_ID_ALT);
        cmd.SetCommand(Key::Alt|Key::Ctrl|Key::F1, "Alt+Ctrl Mod F1", COMMAND_ID_ALT_CTRL);
        return true;
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_COMMAND)
        {
            switch (controlID)
            {
                case COMMAND_ID_PRINT_HELLO_WORLD: l1.SetText("Hello world"); break;
                case COMMAND_ID_PRINT_A_NUMBER: l1.SetText("12345"); break;
                case COMMAND_ID_CLEAR_TEXT: l1.SetText(""); break;
                case COMMAND_ID_NUMPAD: l1.SetText("Numpad pressed!"); break;
                case COMMAND_ID_ENTER: l1.SetText("Enter pressed!"); break;
                case COMMAND_ID_ALT: l1.SetText("Alt modifier!"); break;
                case COMMAND_ID_ALT_CTRL: l1.SetText("Alt+Ctrl+F1 modifier!"); break;
            }
        }
        return false;
    }
};
int main()
{
    Application::Init(Application::Flags::HAS_COMMANDBAR);
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
