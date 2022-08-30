#include "SDLTerminal.hpp"

namespace AppCUI::Internal
{
bool SDLTerminal::OnInit(const Application::InitializationData& initData)
{
    CHECK(InitScreen(initData), false, "");
    CHECK(InitInput(initData), false, "");
    return true;
}

void SDLTerminal::OnUninit()
{
    UninitInput();
    UninitScreen();
}
}