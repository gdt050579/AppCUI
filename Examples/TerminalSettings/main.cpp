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
        this->Create("Test", "d:c,w:40,h:8");
        l1.Create(
              this,
              "This is a simple Terminal example\nthat creates a 40x8 fixed window on\na 50x20 terminal with "
              "different\ncharacter sizes.",
              "x:1,y:1,w:36,h:5");
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
    Log::ToFile("TerminalSettings.log");
    Application::InitializationData initData;
    initData.Width = 120;
    initData.Height = 80;
    initData.CharSize = Application::CharacterSize::Normal;
    if (!Application::Init(initData))
        return 1;
    Application::AddWindow(std::make_unique<SimpleWin>());
    Application::Run();
    return 0;
}
