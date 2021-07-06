#include <string>

#include "os.h"
#include "ncurses.h"
#include "Color.h"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

bool Terminal::OnInit()
{
    return initScreen() && initInput();
}

void Terminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}