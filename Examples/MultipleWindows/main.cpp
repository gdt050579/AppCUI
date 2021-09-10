#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public AppCUI::Controls::Window
{
  public:
    MyWin(const char* name, const std::string_view& layout)
    {
        this->Create(name, layout, WindowFlags::Sizeable);
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
    Application::AddWindow(new MyWin("Win 1", "x:0,y:0,w:50%,h:50%"));
    Application::AddWindow(new MyWin("Win 2", "x:0,y:50%,w:50%,h:50%"));
    Application::AddWindow(new MyWin("Win 3", "x:50%,y:0,w:50%,h:100%"));
    Application::Run();
    return 0;
}