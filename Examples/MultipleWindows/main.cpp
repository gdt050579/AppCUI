#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public AppCUI::Controls::Window
{
public:
    MyWin(const char * name, const char * layout)
    {
        this->Create(name, layout, WindowFlags::SIZEABLE);
    }
    bool OnEvent(const void* sender, Event eventType, int controlID) override
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
    Application::AddWindow(new MyWin("Win 1", "x:0,y:0,w:50%,h:50%"));
    Application::AddWindow(new MyWin("Win 2", "x:0,y:50%,w:50%,h:50%"));
    Application::AddWindow(new MyWin("Win 3", "x:50%,y:0,w:50%,h:100%"));
    Application::Run();
    return 0;
}