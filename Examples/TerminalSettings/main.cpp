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
        this->Create("Test", "a:c,w:40,h:8");
        l1.Create(
              this,
              "This is a simple Terminal example\nthat creates a 40x8 fixed window on\na 50x20 terminal with "
              "different\ncharacter sizes.",
              "x:1,y:1,w:36,h:5");
    }
    bool OnEvent(Control*, Event eventType, int) override
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
    Log::ToFile("TerminalSettings.log");
    if (!Application::Init(50, 20, InitializationFlags::CHAR_SIZE_LARGE))
        return 1;
    Application::AddWindow(new SimpleWin());
    Application::Run();
    return 0;
}
