#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Test", "d:c,w:70,h:24", WindowFlags::None)
    {
        Factory::Label::Create(this, "This is an example of how themes work on AppCUI", "x:8,y:1,w:66");
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Help", 1);
        cmd.SetCommand(Key::F2, "Save", 1);
        cmd.SetCommand(Key::F3, "Load", 1);
        cmd.SetCommand(Key::F4, "Edit", 1);
        return true;
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::Menu | InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    Application::Run();
    return 0;
}
