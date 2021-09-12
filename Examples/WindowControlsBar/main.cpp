#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

class WindowControlsBarExample : public AppCUI::Controls::Window
{
  public:
    WindowControlsBarExample()
    {
        this->Create("Test", "l:2,t:1,r:2,b:1", WindowFlags::Sizeable);
        // associated a hot key
        this->SetHotKey('1');
        // add a TAG
        this->SetTag("TAG", "A tag is a small string\nthat explains what is the purpose\nof this window");

    }
    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
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
    Application::AddWindow(new WindowControlsBarExample());
    Application::Run();
    return 0;
}
