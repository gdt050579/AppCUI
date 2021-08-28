#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public AppCUI::Controls::Window
{
    NumericSelector n1;

  public:
    SimpleWin()
    {
        this->Create("NumericSelectorSample", "a:c,w:40,h:10");
        n1.Create(this, 0, 100, 50, "x:1,y:1,w:36");
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
