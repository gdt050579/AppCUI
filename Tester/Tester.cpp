#include <iostream>
#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#ifdef BUILD_FOR_WINDOWS
#include "Windows.h"
#endif // BUILD_FOR_WINDOWS

class MyWin : public AppCUI::Controls::Window
{
    Label l1, l2;
public:
    MyWin() 
    {
        this->Create("Test", "a:c,w:50%,h:50%",  WindowFlags::SIZEABLE);
        l1.Create(this, "&Single line label with hot key 'S'", "x:1,y:2,w:90%,h:1");
        l2.Create(this, "&Multi line label \nwith hot key 'M'", "x:1,y:4,w:90%,h:2");
    }
    bool OnEvent(const void* sender, AppCUI::Controls::Events::Event eventType, int controlID) override
    {
        if (eventType == AppCUI::Controls::Events::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
    bool OnUpdateCommandBar(CommandBar & cmdBar) override
    {
        cmdBar.SetCommand(Input::Key::F1, "Run", 100);
        cmdBar.SetCommand(Input::Key::F2, "Debug", 100);
        cmdBar.SetCommand(Input::Key::F3|Input::Key::Alt, "Run as admin", 100);
        return true;
    }

};

int main()
{
    Application::Init(Application::Flags::HAS_COMMANDBAR);
    Application::AddWindow(new MyWin());
    Application::Run();
    Application::Close();
}

