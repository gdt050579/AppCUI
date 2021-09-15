#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public AppCUI::Controls::Window
{
    Button b1;
    CheckBox cb1, cb2, cb3;
    RadioBox rb1, rb2, rb3;
    Panel pleft, pright, pbottom;
    Splitter s;
    Splitter s2;

  public:
    MyWin()
    {
        this->Create("Splitter example", "d:c,w:60,h:10", WindowFlags::Sizeable);
        s2.Create(this, "x:0,y:0,w:100%,h:100%", false);
        s.Create(&s2, "x:0,y:0,w:100%,h:100%", true);
        s2.SetSecondPanelSize(2);
        pleft.Create(&s, "x:0,y:0,w:100%,h:100%");
        pright.Create(&s, "x:0,y:0,w:100%,h:100%");
        pbottom.Create(&s2, "x:0,y:0,w:100%,h:100%");

        cb1.Create(&pleft, "Enable first seeting", "x:1,y:1,w:30");
        cb2.Create(&pleft, "Enable second seeting", "x:1,y:2,w:30");
        cb3.Create(&pleft, "Enable third seeting", "x:1,y:3,w:30");

        rb1.Create(&pright, "Option &1", "x:1,y:1,w:20", 5);
        rb2.Create(&pright, "Option &2", "x:1,y:2,w:20", 5);
        rb3.Create(&pright, "Option &3", "x:1,y:3,w:20", 5);

        b1.Create(&pbottom, "&Press me", "l:1,b:0,w:90%,h:2", 100);
    }
    bool OnEvent(Control*, AppCUI::Controls::Event eventType, int) override
    {
        if (eventType == AppCUI::Controls::Event::WindowClose)
        {
            Application::Close();
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
