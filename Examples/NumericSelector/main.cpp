#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public AppCUI::Controls::Window
{
    NumericSelector n1;
    NumericSelector n2;
    NumericSelector n3;
    NumericSelector n4;

  public:
    SimpleWin()
    {
        this->Create("NumericSelectorSample", "a:c,w:40,h:10");

        n1.Create(this, 0, 100, 50, "x:1,y:1,w:36");
        n2.Create(this, -100, 100, 50, "x:1,y:3,w:36");
        n3.Create(this, 0, 100, 50, "x:1,y:5,w:36");
        n4.Create(this, 0, 100, 50, "x:1,y:7,w:36");

        n4.SetEnabled(false);
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
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(new SimpleWin());
    Application::Run();

    return 0;
}
