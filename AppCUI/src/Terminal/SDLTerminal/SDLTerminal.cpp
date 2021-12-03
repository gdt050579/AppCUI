#include <string>
#include "SDLTerminal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

bool SDLTerminal::OnInit(const AppCUI::Application::InitializationData& initData)
{
    if (!initScreen(initData))
        return false;
    if (!initInput(initData))
        return false;
    return true;
}

void SDLTerminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}
