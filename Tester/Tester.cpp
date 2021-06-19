#include <iostream>
#include "../AppCUI/include/AppCUI.h"

using namespace AppCUI;

int main()
{
    Application::Init(Application::Flags::HAS_COMMANDBAR);
    Application::SetCommand(Input::Key::F1, "Run", 100);
    Application::SetCommand(Input::Key::F2, "Debug", 100);
    Application::SetCommand(Input::Key::F3, "Run as admin", 100);
    Application::Run();
    Application::Close();
}

