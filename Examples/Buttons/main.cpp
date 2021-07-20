#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


#define BUTTON_1_ID     1
#define BUTTON_2_ID     2
#define BUTTON_3_ID     3

class MyWin : public AppCUI::Controls::Window
{
    Button b1, b2, b3;
    Panel p1;
    Label l1, l2;
public:
    MyWin()
    {
        this->Create("Button example", "a:c,w:60,h:20");
        b1.Create(this, "Buton &1", "x:1,y:16,w:14", BUTTON_1_ID);
        b2.Create(this, "Buton &2", "x:16,y:16,w:14", BUTTON_2_ID);
        b3.Create(this, "Inactive", "x:31,y:16,w:14", BUTTON_3_ID);
        b3.SetEnabled(false);
        p1.Create(this, "Information panel", "x:1,y:1,w:56,h:5");
        l1.Create(&p1, "Use 'TAB' and 'CTRL+TAB'to switch betweens buttons.\nUse 'Enter' or 'Space' to press a button.\nUse 'ESC' to exit.", "x:0,y:0,w:100%,h:100%");
        l2.Create(this, "", "x:1,y:10,w:56");

    }
    bool OnEvent(const void* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_BUTTON_CLICKED)
        {
            switch (controlID)
            {
                case BUTTON_1_ID: l2.SetText("Button 1 was pressed !"); break;
                case BUTTON_2_ID: l2.SetText("Button 2 was pressed !"); break;
            }
            return true;
        }
        return false;
    }
};

int main()
{
    Application::Init();
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
