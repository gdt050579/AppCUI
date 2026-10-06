// Sample of docs/source/getting_started.rst - compiled with the examples (docs/snippets/CMakeLists.txt).
#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

// [handlers]
int main()
{
    if (!Application::Init(InitializationFlags::CommandBar))
        return 1;
    auto window = Factory::Window::Create("Handlers", "d:c,w:40,h:8");
    auto label  = Factory::Label::Create(*window, "Press the button", "x:1,y:1,w:36");
    auto button = Factory::Button::Create(*window, "&Press me", "x:50%,y:3,w:16,a:t");

    // a plain function (or a lambda without captures) instead of a new class
    button->Handlers()->OnButtonPressed = [](Reference<Button> btn) {
        btn->SetText("Pressed !");
    };
    label->SetText("Use Tab / Shift+Tab to move the focus");

    Application::AddWindow(std::move(window));
    Application::Run();
    return 0;
}
// [/handlers]
