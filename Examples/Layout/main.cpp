#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class MyControl : public AppCUI::Controls::UserControl
{
  public:
    void Paint(AppCUI::Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::Black, Color::Red });
    }
};
class LayoutWin : public AppCUI::Controls::Window
{
    MyControl m;
  public:
    LayoutWin(const std::string_view & layout)
    {
        this->Create("Test", "d:c,w:40,h:14", WindowFlags::Sizeable);
        m.Create(this, layout);        
    }
};

class MainWin : public AppCUI::Controls::Window
{
    ListView lst;
    Button b;
  public:
    MainWin()
    {
        this->Create("Layout modex", "x:0,y:0,w:70,h:20");
        lst.Create(this, "x:1,y:1,w:66,h:14");
        lst.AddColumn("Layout", TextAlignament::Left, 25);
        lst.AddColumn("Explanation", TextAlignament::Left, 100);
        
        // add list of layouts        
        lst.AddItem("x:3,y:3,w:4,h:4", "A 4x4 character object that starts from (3,3)");
        lst.AddItem("x:2,y:2,w:75%,h:75%", "An object that has 75% of its parent for width and height and starts from (2,2)");
        lst.AddItem("d:br,w:5,h:5", "An object located on the bottom right corner with size 5x5");
        lst.AddItem("x:10,y:10,a:br,w:7,h:3", "An object with its right-bottom corner at (10,10) and size 5x5");
        b.Create(this, "&Show", "X:28,y:16,w:16", 1234);
        b.SetEnabled(lst.GetCurrentItem() != InvalidItemHandle);
    }
    bool OnEvent(Control* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_BUTTON_CLICKED)
        {
            auto itm = lst.GetCurrentItem();
            if (itm != InvalidItemHandle)
            {
                LayoutWin dlg((std::string)lst.GetItemText(itm, 0));
                dlg.Show();
                return true;
            }
        }
        if (eventType == Event::EVENT_LISTVIEW_CURRENTITEM_CHANGED)
        {
            b.SetEnabled(lst.GetCurrentItem() != InvalidItemHandle);
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(new MainWin());
    Application::Run();
    return 0;
}
