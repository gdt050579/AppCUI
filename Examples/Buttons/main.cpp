#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define BUTTON_1_ID 1
#define BUTTON_2_ID 2
#define BUTTON_3_ID 3

class MyWin : public AppCUI::Controls::Window
{
    Button b1, b2, b3;
    Button fb1, fb2, fb3;
    Panel p1;
    Label l1, l2;

  public:
    MyWin()
    {
        this->Create("Button example", "d:c,w:60,h:20");
        b1.Create(this, "Buton &1", "x:1,y:16,w:14", BUTTON_1_ID);
        b2.Create(this, "Buton &2", "x:16,y:16,w:14", BUTTON_2_ID);
        b3.Create(this, "Inactive", "x:31,y:16,w:14", BUTTON_3_ID);
        b3.SetEnabled(false);

        fb1.Create(this, "Flat Buton 1", "x:1,y:14,w:15", BUTTON_1_ID, ButtonFlags::Flat);
        fb2.Create(this, "Flat Buton 2", "x:16,y:14,w:15", BUTTON_2_ID, ButtonFlags::Flat);
        fb3.Create(this, "Inactive", "x:31,y:14,w:15", BUTTON_3_ID, ButtonFlags::Flat);
        fb3.SetEnabled(false);

        p1.Create(this, "Information panel", "x:1,y:1,w:56,h:5");
        l1.Create(
              &p1,
              "Use 'TAB' and 'CTRL+TAB'to switch betweens buttons.\nUse 'Enter' or 'Space' to press a button.\nUse "
              "'ESC' to exit.",
              "x:0,y:0,w:100%,h:100%");
        l2.Create(this, "", "x:1,y:10,w:56");
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ButtonClicked)
        {
            switch (controlID)
            {
            case BUTTON_1_ID:
                l2.SetText("Button 1 was pressed !");
                break;
            case BUTTON_2_ID:
                l2.SetText("Button 2 was pressed !");
                break;
            }
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
