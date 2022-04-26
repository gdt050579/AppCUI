#include "NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>

namespace AppCUI::Internal
{
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
}