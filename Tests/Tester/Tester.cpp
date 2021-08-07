#include <iostream>
#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#ifdef BUILD_FOR_WINDOWS
#    include "Windows.h"
#endif // BUILD_FOR_WINDOWS

class MyWin : public AppCUI::Controls::Window
{
    Label l1, l2, l3;
    Button b1, b2;
    CheckBox cb1;
    RadioBox rb1, rb2, rb3;
    Panel pleft, pright, pbottom;
    Splitter s;
    Splitter s2;

  public:
    MyWin()
    {
        this->Create("Test", "a:c,w:50%,h:50%", WindowFlags::Sizeable);
        s2.Create(this, "x:0,y:0,w:100%,h:100%", false);
        s.Create(&s2, "x:0,y:0,w:100%,h:100%", true);
        pleft.Create(&s, "x:0,y:0,w:100%,h:100%");
        pright.Create(&s, "x:0,y:0,w:100%,h:100%");
        pbottom.Create(&s2, "x:0,y:0,w:100%,h:100%");

        l1.Create(&pleft, "&Single line label with hot key 'S'", "x:1,y:2,w:15,h:1");
        l2.Create(&pleft, "&Multi line label \nwith hot key 'M'", "x:1,y:4,w:15,h:2");
        l3.Create(&pleft, "1234567890ABCDEFGHIJKLMNOPQRSTUV", "x:10,y:9,w:30,h:1");
        cb1.Create(&pleft, "Run on &Windows", "x:40,y:1,w:20");

        rb1.Create(&pright, "Option &1", "x:1,y:3,w:20", 5);
        rb2.Create(&pright, "Option &2", "x:1,y:4,w:20", 5);
        rb3.Create(&pright, "Option &3", "x:1,y:5,w:20", 5);

        b1.Create(&pbottom, "&OK", "l:1,b:0,w:90%,h:2", 100);
        b2.Create(&pbottom, "&Cancel", "x:2,y:5,w:15", 102);
    }
    bool OnEvent(const void* sender, AppCUI::Controls::Event eventType, int controlID) override
    {
        if (eventType == AppCUI::Controls::Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
    bool OnUpdateCommandBar(CommandBar& cmdBar) override
    {
        cmdBar.SetCommand(Input::Key::F1, "Run", 100);
        cmdBar.SetCommand(Input::Key::F2, "Debug", 100);
        cmdBar.SetCommand(Input::Key::F3 | Input::Key::Alt, "Run as admin", 100);
        return true;
    }
};
class MyWin2 : public AppCUI::Controls::Window
{
    TextField tx1;

  public:
    MyWin2()
    {
        this->Create("Test", "a:c,w:50%,h:50%", WindowFlags::None);
        tx1.Create(this, "Salut", "x:1,y:1,w:30,h:3");
    }
};
int main()
{
    if (!Application::Init(Application::InitializationFlags::HAS_COMMANDBAR))
        return 1;
    Application::AddWindow(new MyWin());
    Application::Run();
    Application::Close();
}
