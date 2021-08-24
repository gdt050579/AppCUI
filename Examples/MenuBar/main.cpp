#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

int main()
{
    if (!Application::Init(InitializationFlags::HAS_MENU))
        return 1;
    Menu* files = Application::AddMenu("&File");
    Menu* edit  = Application::AddMenu("&Edit");
    Menu* help  = Application::AddMenu("&Help");
    Application::Run();
    return 0;
}
