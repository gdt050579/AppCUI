#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define COMMAND_ID_PRINT_HELLO_WORLD    1
#define COMMAND_ID_PRINT_A_NUMBER       2
#define COMMAND_ID_CLEAR_TEXT           3

class MyWin : public AppCUI::Controls::Window
{
    Label l1;
public:
    MyWin()
    {
        this->Create("Command Bar Example", "a:c,w:60,h:10");
        l1.Create(this, "", "x:1,y:2,w:56");
    }
    bool OnUpdateCommandBar(CommandBar & cmd)
    {
        cmd.SetCommand(Key::F1, "Print Hello World", COMMAND_ID_PRINT_HELLO_WORLD);
        cmd.SetCommand(Key::F2, "Print a number", COMMAND_ID_PRINT_A_NUMBER);
        cmd.SetCommand(Key::Alt | Key::F1, "Clear text", COMMAND_ID_CLEAR_TEXT);
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
