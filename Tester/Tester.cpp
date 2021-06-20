#include <iostream>
#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Controls;

#ifdef BUILD_FOR_WINDOWS
#include "Windows.h"
#endif // BUILD_FOR_WINDOWS

class MyWin : public AppCUI::Controls::Window
{
public:
    MyWin() 
    {
        this->Create("Test", "x:10,y:10,w:40,h:20", WindowFlags::CENTERED | WindowFlags::SIZEABLE);
    }
    bool OnEvent(const void* sender, AppCUI::Controls::Events::Event eventType, int controlID) override
    {
        if (eventType == AppCUI::Controls::Events::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
    bool OnUpdateCommandBar() override
    {
        Application::SetCommand(Input::Key::F1, "Run", 100);
        Application::SetCommand(Input::Key::F2, "Debug", 100);
        Application::SetCommand(Input::Key::F3, "Run as admin", 100);
        return true;
    }

};

int main()
{
    Application::Init(Application::Flags::HAS_COMMANDBAR);
    Application::AddWindow(new MyWin());
    Application::AddWindow(new MyWin());
    Application::AddWindow(new MyWin());
    Application::Run();
}

