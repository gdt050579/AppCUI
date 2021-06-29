#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

class MyWin : public AppCUI::Controls::Window
{
    Label l1,l2,l3,l4;
    TextField t1, t2, t3, t4;

public:
    MyWin()
    {
        this->Create("Text Field Example", "a:c,w:70,h:20");
        l1.Create(this, "&Normal text", "x:1,y:1,w:15");
        t1.Create(this, "a normal text", "x:16,y:1,w:51");
        t1.SetHotKey('N');

        l2.Create(this, "&Read only", "x:1,y:3,w:15");
        t2.Create(this, "this text cannnot be changed", "x:16,y:3,w:51", TextFieldFlags::READONLY_TEXT);
        t2.SetHotKey('R');

        l3.Create(this, "Inactive text", "x:1,y:5,w:15");
        t3.Create(this, "this text is inactive", "x:16,y:5,w:51");
        t3.SetEnabled(false);

        l4.Create(this, "&Multi-line", "x:1,y:7,w:15");
        t4.Create(this, "this is a large text the expends for over the next lines", "x:16,y:7,w:51,h:3");
        t4.SetHotKey('M');
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
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
