#include <string>
#include "Terminal/SDLTerminal/SDLTerminal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;



bool SDLTerminal::OnInit(const InitializationData& initData)
{
    if (!initScreen()) return false;
    if (!initInput()) return false;
    return true;
}

void SDLTerminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}