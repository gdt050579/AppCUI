#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


class MyWin : public AppCUI::Controls::Window
{
    ComboBox cb1;
    Label inf, col;
public:
    MyWin()
    {
        this->Create("ComboBox example", "a:c,w:60,h:20");
        inf.Create(this, "Select a color", "x:1,y:1,w:15");
        col.Create(this, "", "x:1,y:2,w:15");
        cb1.Create(this, "x:18,y:1,w:30", "White,Blue,Red,Aqua,Metal,Yellow,Green,Orange");

    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED)
        {
            col.SetText(cb1.GetUnsafeCurrentItemText());
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
