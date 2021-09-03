#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class MyControl: public UserControl
{
  public:
    void Paint(AppCUI::Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::Black, Color::Red });        
    }
    bool OnMouseEnter() override
    {
        this->ShowToolTip("This is a custom user control that has two functions overridden:\n"
                          "- Paint(...) --> to draw a red rectangle\n"
                          "- OnMouseEnter() --> to display this tooltip");
        return true;
    }
    bool OnMouseLeave() override
    {
        return true;
    }
};
class ToolTipExampleWin : public AppCUI::Controls::Window
{
    Label l1;
    Button b1;
    CheckBox c1;
    RadioBox r1;
    MyControl m;
  public:
    ToolTipExampleWin()
    {
        this->Create("Tool tip example", "d:c,w:40,h:9");
        l1.Create(
              this,
              "Hover the mouse over the Button, \nCheckbox, RadioBox and [X] button \nfrom the window to see a tool "
              "tip !",
              "x:1,y:1,w:36,h:3");
        b1.Create(this, "Button with a long string caption", "x:1,y:5,w:7");
        c1.Create(this, "An option with a long description", "x:10,y:5,w:7");
        r1.Create(this, "Select this option", "x:19,y:5,w:7", 100);
        m.Create(this, "l:30,t:5,w:5,h:2");
    }
    bool OnEvent(Control*, Event eventType, int) override
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
    if (!Application::Init())
        return 1;
    Application::AddWindow(new ToolTipExampleWin());
    Application::Run();
    return 0;
}
