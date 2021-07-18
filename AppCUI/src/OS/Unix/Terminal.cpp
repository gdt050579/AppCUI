#include <string>

#include "os.h"
#include "ncurses.h"
#include "Color.h"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

bool Terminal::OnInit()
{
    if (!initScreen()) return false;
    if (!initInput()) return false;
    return true;
}

void Terminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}