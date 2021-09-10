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
    void AddExample(std::string_view layout, std::string_view info)
    {
        ItemHandle i = lst.AddItem(layout, info);
        lst.SetItemXOffset(i, 2);
    }
    void AddGroup(std::string_view name)
    {
        ItemHandle i = lst.AddItem(name);
        lst.SetItemType(i, ListViewItemType::Highlighted);
    }
  public:
    MainWin()
    {
        this->Create("Layout modex", "x:0,y:0,w:70,h:20");
        lst.Create(this, "x:1,y:1,w:66,h:14");
        lst.AddColumn("Layout", TextAlignament::Left, 25);
        lst.AddColumn("Explanation", TextAlignament::Left, 100);
        
        // add list of layouts 
        AddGroup("Docking");
        AddExample("d:c", "A fit parent layout");
        AddExample("d:c,w:50%,h:50%", "An object centered to its parent with half of it parent sized");
        AddExample("d:c,w:50%", "An object centered to its parent with the same height as its parent, but half width");
        AddExample("d:br,w:5,h:5", "An object located on the bottom right corner with size 5x5");
        AddExample("d:b,w:50%,h:5", "An object located on the bottom of it parent, with width 50% of its parent width and heigh of 5 characters");
        AddGroup("XY coordonates");
        AddExample("x:3,y:3,w:4,h:4", "A 4x4 character object that starts from (3,3)");
        AddExample("x:2,y:2,w:75%,h:75%", "An object that has 75% of its parent for width and height and starts from (2,2)");
        AddExample("x:10,y:10,a:br,w:7,h:3", "An object with its right-bottom corner at (10,10) and size 5x5");
        AddGroup("Corner anchors");
        AddExample("r:2,b:2,w:10,h:25%", "A 10x25% height object with its right bottom corner 2 characters away from parents right bottom corner");
        AddExample("l:2,b:2,w:10,h:25%", "A 10x25% height object with its left bottom corner 2 characters away from parents left bottom corner");
        AddGroup("Parallel margin anchors");
        AddExample("r:2,l:2,h:25%,y:10,a:t", "An object that has 2 characters until his parent from both left and right margin and it's allign on top of 10th Y character");
        AddExample("r:10,l:3,h:50%,y:50%", "An object that has 3 character from left and 10 characters right margin and it's center alligned with a height of 50%");
        AddExample("r:5,l:20,h:4,y:100%,a:b", "An object that has 20 character from left and 3 characters right margin and it's bottom alligned with a height of 4 characters");
        AddExample("t:3,b:6,w:15,a:c,x:50%", "An object that has 3 character from top and 6 characters from bottom margin and it's center alligned with a width of 15 characters");
        AddExample("t:3,b:6,w:15,a:r,x:90%", "An object that has 3 character from top and 6 characters from bottom margin and it's right alligned with a width of 15 characters and 10% padding until right margin");
        AddGroup("3 margins anchors");
        AddExample("l:1,t:2,r:3,h:4", "An object that has the following margins: left: 1 character, top: 2 characters, right: 3 characters, and height of 4 characters ");
        AddExample("l:1,b:2,r:3,h:4", "An object that has the following margins: left: 1 character, bottom: 2 characters, right: 3 characters, and height of 4 characters ");
        AddExample("t:1,l:2,b:3,w:4", "An object that has the following margins: top: 1 character, left: 2 characters, bottom: 3 characters, and width of 4 characters ");
        AddExample("t:1,r:2,b:3,w:4", "An object that has the following margins: top: 1 character, right: 2 characters, bottom: 3 characters, and width of 4 characters ");
        AddGroup("4 margins anchors");
        AddExample("l:1,t:2,r:3,b:4", "An object that has the following margins: left: 1 character, top: 2 characters, right: 3 characters, bottom: 4 characters ");
        AddExample("l:1,t:1,r:1,b:1", "An object that has the following margins: left: 1 character, top: 1 character, right: 1 character, bottom: 1 character ");
        b.Create(this, "&Show", "d:b,w:16", 1234);
        b.SetEnabled(false);
    }
    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ButtonClicked)
        {
            auto itm = lst.GetCurrentItem();
            if (itm != InvalidItemHandle)
            {
                LayoutWin dlg((std::string)lst.GetItemText(itm, 0));
                dlg.Show();
                return true;
            }
        }
        if (eventType == Event::ListViewCurrentItemChanged)
        {            
            b.SetEnabled(lst.GetItemXOffset(lst.GetCurrentItem())==2);
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
