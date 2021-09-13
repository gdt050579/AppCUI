#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

class MyControl : public AppCUI::Controls::UserControl
{
  public:
    Color back;
    void Paint(AppCUI::Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::Black, back });
    }
};
class WindowControlsBarExample : public AppCUI::Controls::Window
{
    MyControl m;
  public:
    WindowControlsBarExample()
    {
        this->Create("Test", "l:2,t:1,r:2,b:1", WindowFlags::Sizeable);
        m.Create(this, "l:2,t:1,r:2,b:1");
        m.back = Color::Black;
        // associated a hot key
        this->SetHotKey('1');
        // add a TAG
        this->SetTag("TAG", "A tag is a small string\nthat explains what is the purpose\nof this window");
        // add buttons;
        auto cb = this->GetControlBar(WindowControlsBarLayout::TopBarFromLeft);
        cb.AddCommandItem("Close", 12345, "When you press this button the Window will close");
        cb.AddCommandItem("Center", 12346, "When you press this button the Window will center to the screen");
        cb = this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight);
        //cb.AddCommandItem("&Test", 12346);
        cb.AddRadioItem("&Red", 100, false);
        cb.AddRadioItem("&Green", 101, false);
        cb.AddRadioItem("&Blue", 102, false);
    }
    bool OnEvent(Control*, Event eventType, int ID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Command)
        {
            switch (ID)
            {
            case 12345:
                Application::Close();
                return true;
            case 12346:
                this->CenterScreen();
                return true;
            case 100:
                m.back = Color::Red;
                return true;
            case 101:
                m.back = Color::Green;
                return true;
            case 102:
                m.back = Color::Blue;
                return true;
            }
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(new WindowControlsBarExample());
    Application::Run();
    return 0;
}
