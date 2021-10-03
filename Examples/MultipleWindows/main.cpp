#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(Factory::Window::Create("Win 1", "x:0,y:0,w:50%,h:50%"));
    Application::AddWindow(Factory::Window::Create("Win 2", "x:0,y:50%,w:50%,h:50%"));
    Application::AddWindow(Factory::Window::Create("Win 3", "x:50%,y:0,w:50%,h:100%"));
    Application::Run();
    return 0;
}