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
    Label l1, l2, l3;
    Button b1, b2;
    CheckBox cb1;
    RadioBox rb1, rb2, rb3;
public:
    MyWin() 
    {
        this->Create("Test", "a:c,w:50%,h:50%",  WindowFlags::SIZEABLE);
        l1.Create(this, "&Single line label with hot key 'S'", "x:1,y:2,w:15,h:1");
        l2.Create(this, "&Multi line label \nwith hot key 'M'", "x:1,y:4,w:15,h:2");
        l3.Create(this, "1234567890ABCDEFGHIJKLMNOPQRSTUV", "x:10,y:9,w:30,h:1");
        cb1.Create(this, "Run on &Windows", "x:40,y:1,w:20");
        rb1.Create(this, "Option &1", "x:40,y:3,w:20", 5);
        rb2.Create(this, "Option &2", "x:40,y:4,w:20", 5);
        rb3.Create(this, "Option &3", "x:40,y:5,w:20", 5);

        b1.Create(this, "&OK", "l:1,b:0,w:90%,h:2", 100);
        b2.Create(this, "&Cancel", "x:26,y:10,w:15", 102);
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

