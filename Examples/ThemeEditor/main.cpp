#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

int main()
{
    if (!Application::Init())
        return 1;
    Application::GetDesktop()->Handlers()->OnStart = [](Reference<Control>) { Dialogs::ThemeEditor::Show(); };
    Application::Run();
    return 0;
}
