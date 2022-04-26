#include "SDLTerminal.hpp"

namespace AppCUI::Internal
{
bool SDLTerminal::OnInit(const Application::InitializationData& initData)
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
}