#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

int main()
{
    Log::ToFile("TerminalSettings.log");
    Application::InitializationData initData;
    initData.Width    = 120;
    initData.Height   = 80;
    initData.CharSize = Application::CharacterSize::Normal;
    if (!Application::Init(initData))
        return 1;
    auto wnd = Factory::Window::Create("Test", "d:c,w:40,h:8");
    Factory::Label::Create(
          wnd,
          "This is a simple Terminal example\nthat creates a 40x8 fixed window on\na 50x20 terminal with "
          "different\ncharacter sizes.",
          "x:1,y:1,w:36,h:5");
    Application::AddWindow(std::move(wnd));
    Application::Run();
    return 0;
}
