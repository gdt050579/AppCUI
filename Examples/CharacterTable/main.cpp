#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Example", "d:c,w:40,h:20", WindowFlags::Sizeable)
    {
        Factory::CharacterTable::Create(this, "l:1,t:1,r:1,b:3");
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
