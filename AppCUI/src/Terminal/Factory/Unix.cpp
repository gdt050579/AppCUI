#include "Terminal/TerminalFactory.hpp"

using namespace AppCUI::Internal;

#ifdef HAVE_SDL
#include "Terminal/SDLTerminal/SDLTerminal.hpp"
std::unique_ptr<AbstractTerminal> sdl_terminal()
{
    return std::make_unique<SDLTerminal>();
}
#else
std::unique_ptr<AbstractTerminal> sdl_terminal()
{
    RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d): Please install SDL2", (unsigned int)initData.FrontEnd);
}
#endif

#ifdef HAVE_CURSES
#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
std::unique_ptr<AbstractTerminal> curses_terminal()
{
    return std::make_unique<NcursesTerminal>();
}
#else 
std::unique_ptr<AbstractTerminal> curses_terminal()
{
    RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d): Please install ncurses", (unsigned int)initData.FrontEnd);
}
#endif

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData &initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;

    switch (initData.FrontEnd)
    {
    case TerminalType::Default:
        term = sdl_terminal();
        if (!term)
        {
            term = curses_terminal();
        }
        break;
    case TerminalType::SDL:
        term = sdl_terminal();
        break;
    case TerminalType::Terminal:
        term = curses_terminal();
        break;
    default:
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d)", (unsigned int)initData.FrontEnd);
    }
    CHECK(term, nullptr, "No terminal available or fail to allocate memory for a terminal !");
    if (term->Init(initData) == false)
    {
        RETURNERROR(nullptr, "Fail to initialize the terminal !");
    }
    return term;
}
