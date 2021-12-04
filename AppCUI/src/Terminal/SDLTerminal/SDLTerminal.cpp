#include <string>
#include "SDLTerminal.hpp"

using namespace Internal;
using namespace Input;

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
