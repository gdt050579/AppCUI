#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Console;


class MyWin : public AppCUI::Controls::Window
{
    ListView lv;

public:
    MyWin()
    {
        this->Create("List View Example", "a:c,w:70,h:20");
        lv.Create(this, "x:1,y:1,w:66,h:16", ListViewFlags::NONE);
        lv.AddColumn("Name", TextAlignament::Left, 15);
        lv.AddColumn("Towm", TextAlignament::Left, 15);
        lv.AddColumn("Income", TextAlignament::Right, 20);

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
