#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class ToolTipExampleWin : public AppCUI::Controls::Window
{
    Label l1;
    Button b1;
    CheckBox c1;
    RadioBox r1;
  public:
    ToolTipExampleWin()
    {
        this->Create("Tool tip example", "a:c,w:40,h:9");
        l1.Create(
              this,
              "Hover the mouse over the Button, \nCheckbox, RadioBox and [X] button \nfrom the window to see a tool "
              "tip !",
              "x:1,y:1,w:36,h:3");
        b1.Create(this, "Button with a long string caption", "x:1,y:5,w:5");
        c1.Create(this, "An option with a long descriptions", "x:10,y:5,w:5");
        r1.Create(this, "Select this option", "x:19,y:5,w:5", 100);
    }
    bool OnEvent(Control* sender, Event eventType, int controlID) override
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
