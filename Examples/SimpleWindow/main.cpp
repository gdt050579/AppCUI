#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public AppCUI::Controls::Window
{
    Label l1;

  public:
    SimpleWin()
    {
        this->Create("Test", "a:c,w:40,h:10");
        l1.Create(
              this,
              "This is a simple Window example that creates a 40x20 fixed window that  will close if you click on the "
              "[X]  button from the bottom-top corner.",
              "x:1,y:1,w:36,h:5");
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
    Application::AddWindow(new SimpleWin());
    Application::Run();
    return 0;
}
