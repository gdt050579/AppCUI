#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class MyControl: public UserControl
{
  public:
    MyControl(std::string_view layout) : UserControl(layout)
    {
    } 
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
  public:
    ToolTipExampleWin() : Window("Tool tip example", "d:c,w:40,h:9",WindowFlags::None)
    {
        Factory::Label::Create(
              this,
              "Hover the mouse over the Button, \nCheckbox, RadioBox and [X] button \nfrom the window to see a tool "
              "tip !",
              "x:1,y:1,w:36,h:3");
        Factory::Button::Create(this, "Button with a long string caption", "x:1,y:5,w:7");
        Factory::CheckBox::Create(this, "An option with a long description", "x:10,y:5,w:7");
        Factory::RadioBox::Create(this, "Select this option", "x:19,y:5,w:7", 100);
        this->AddControl<MyControl>(std::make_unique<MyControl>("l:30,t:5,w:5,h:2"));
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<ToolTipExampleWin>());
    Application::Run();
    return 0;
}
