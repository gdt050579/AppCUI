#include "NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>
#include <string>

using namespace Internal;
using namespace Input;

bool NcursesTerminal::OnInit(const Application::InitializationData&)
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