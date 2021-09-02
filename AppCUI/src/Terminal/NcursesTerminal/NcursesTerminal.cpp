#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>
#include <string>

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

bool NcursesTerminal::OnInit(const InitializationData&)
{
    if (!initScreen())
        return false;
    if (!initInput())
        return false;
    return true;
}

void NcursesTerminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}