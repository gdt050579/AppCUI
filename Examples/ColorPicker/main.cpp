#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Example", "d:c,w:40,h:10", WindowFlags::None)
    {
        Factory::ColorPicker::Create(this, "x:1,y:1,w:38", Graphics::Color::DarkGreen);
        Factory::Button::Create(this, "A button", "d:b,w:20", 123);
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    Application::Run();
    return 0;
}
