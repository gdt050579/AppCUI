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
        lv.Create(this, "x:1,y:1,w:66,h:16", ListViewFlags::ALLOWSELECTION);
        lv.AddColumn("&Name", TextAlignament::Left, 15);
        lv.AddColumn("&Town", TextAlignament::Center, 15);
        lv.AddColumn("&Income", TextAlignament::Right, 20);
        // add items
        lv.AddItem("John", "Georgia", "100.000");
        lv.AddItem("Mike", "California", "150.000");
        lv.AddItem("Vincent", "Alaska", "20.000");
        lv.AddItem("Marry", "Georgia", "110.000");
        lv.AddItem("Sarah", "California", "80.000");
        lv.AddItem("George", "Florida", "130.000");
        lv.AddItem("Tudor", "Florida", "80.000");
        lv.AddItem("Laura", "Alaska", "40.000");
        lv.AddItem("John", "California", "140.000");
        // sort them after the name (first column)
        lv.Sort(0, true);
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
