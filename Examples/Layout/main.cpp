#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class MyControl : public UserControl
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
    LayoutWin(std::string_view & layout)
    {
        this->Create("Test", "a:c,w:40,h:14", WindowFlags::Sizeable);
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
        l1.Create(
              this,
              "This is a simple Window example that creates a 40x20 fixed window that  will close if you click on the "
              "[X]  button from the bottom-top corner.",
              "x:1,y:1,w:36,h:5");
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
    Application::AddWindow(new SimpleWin());
    Application::Run();
    return 0;
}
