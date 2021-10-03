#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public Window
{
  public:
    MyWin() : Window("Splitter example", "d:c,w:60,h:10", WindowFlags::Sizeable)
    {        
        auto s2 = Factory::Splitter::Create(*this, "x:0,y:0,w:100%,h:100%", false);
        auto s  = Factory::Splitter::Create(*s2, "x:0,y:0,w:100%,h:100%", true);
        s2->SetSecondPanelSize(2);
        s->SetSecondPanelSize(30);
        auto pleft   = Factory::Panel::Create(*s, "x:0,y:0,w:100%,h:100%");
        auto pright  = Factory::Panel::Create(*s, "x:0,y:0,w:100%,h:100%");
        auto pbottom = Factory::Panel::Create(*s2, "x:0,y:0,w:100%,h:100%");

        Factory::CheckBox::Create(*pleft, "Enable first seeting", "x:1,y:1,w:30");
        Factory::CheckBox::Create(*pleft, "Enable second seeting", "x:1,y:2,w:30");
        Factory::CheckBox::Create(*pleft, "Enable third seeting", "x:1,y:3,w:30");

        Factory::RadioBox::Create(*pright, "Option &1", "x:1,y:1,w:20", 5);
        Factory::RadioBox::Create(*pright, "Option &2", "x:1,y:2,w:20", 5);
        Factory::RadioBox::Create(*pright, "Option &3", "x:1,y:3,w:20", 5);

        Factory::Button::Create(*pbottom, "Empy &example", "l:1,b:0,w:90%,h:2", 100);
    }
    bool OnEvent(Control*, AppCUI::Controls::Event eventType, int) override
    {
        if (eventType == AppCUI::Controls::Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == AppCUI::Controls::Event::ButtonClicked)
        {
            auto win = Factory::Window::Create("Empty example", "d:c,w:60,h:20", WindowFlags::Sizeable);
            auto sp1 = Factory::Splitter::Create(*win, "d:c%", false);
            sp1->SetSecondPanelSize(10);
            Factory::Splitter::Create(*sp1, "d:c", true)->SetSecondPanelSize(10);
            win->Show();
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
