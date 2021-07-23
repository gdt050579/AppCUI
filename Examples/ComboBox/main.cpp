#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


class MyWin : public AppCUI::Controls::Window
{
    ComboBox cb1, cb2, cb3;
    Label inf, col, inf2, inf3;
public:
    MyWin()
    {
        this->Create("ComboBox example", "a:c,w:60,h:11");
        inf.Create(this, "Select a color", "x:1,y:1,w:15");
        col.Create(this, "", "x:1,y:2,w:15");
        cb1.Create(this, "x:18,y:1,w:30", "White,Blue,Red,Aqua,Metal,Yellow,Green,Orange");
        inf2.Create(this, "Select a form", "x:1,y:4,w:15");
        cb2.Create(this, "x:18,y:4,w:30", "Rectangle,Circle,Triangle");
        inf3.Create(this, "Select a fruit", "x:1,y:7,w:15");
        cb3.Create(this, "x:18,y:7,w:30", "Apple,Orange,Pinaple");
    }
    bool OnEvent(const void* sender, Event eventType, int controlID) override
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
    if (!Application::Init())
        return 1;
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
