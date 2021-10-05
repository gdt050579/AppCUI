#include <iostream>
#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#ifdef BUILD_FOR_WINDOWS
#    include "Windows.h"
#endif // BUILD_FOR_WINDOWS

class MyWin : public Window
{
  public:
    MyWin() : Window("Test", "d:c,w:50%,h:50%", WindowFlags::Sizeable)
    {
        auto s2      = Factory::Splitter::Create(this, "x:0,y:0,w:100%,h:100%", false);
        auto s       = Factory::Splitter::Create(s2, "x:0,y:0,w:100%,h:100%", true);
        auto pleft   = Factory::Panel::Create(s, "x:0,y:0,w:100%,h:100%");
        auto pright  = Factory::Panel::Create(s, "x:0,y:0,w:100%,h:100%");
        auto pbottom = Factory::Panel::Create(s2, "x:0,y:0,w:100%,h:100%");

        Factory::Label::Create(pleft, "&Single line label with hot key 'S'", "x:1,y:2,w:15,h:1");
        Factory::Label::Create(pleft, "&Multi line label \nwith hot key 'M'", "x:1,y:4,w:15,h:2");
        Factory::Label::Create(pleft, "1234567890ABCDEFGHIJKLMNOPQRSTUV", "x:10,y:9,w:30,h:1");
        Factory::CheckBox::Create(pleft, "Run on &Windows", "x:40,y:1,w:20");

        Factory::RadioBox::Create(pright, "Option &1", "x:1,y:3,w:20", 5);
        Factory::RadioBox::Create(pright, "Option &2", "x:1,y:4,w:20", 5);
        Factory::RadioBox::Create(pright, "Option &3", "x:1,y:5,w:20", 5);

        Factory::Button::Create(pbottom, "&OK", "l:1,b:0,w:90%,h:2", 100);
        Factory::Button::Create(pbottom, "&Cancel", "x:2,y:5,w:15", 102);
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
  public:
    MyWin2() : Window("Test", "d:c,w:50%,h:50%", WindowFlags::None)
    {
        Factory::TextField::Create(this, "Salut", "x:1,y:1,w:30,h:3");
    }
};
int main()
{
    if (!Application::Init(Application::InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    Application::Close();
}
