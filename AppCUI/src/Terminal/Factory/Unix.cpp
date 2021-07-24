#include "Terminal/TerminalFactory.hpp"

using namespace AppCUI::Internal;

#ifdef HAVE_SDL
#include "Terminal/SDLTerminal/SDLTerminal.hpp"
constexpr bool have_sdl = true;
#else
constexpr bool have_sdl = false;
#endif

#ifdef HAVE_CURSES
#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
constexpr bool have_curses = true;
#else
constexpr bool have_curses = false;
#endif

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData &initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;

    switch (initData.FrontEnd)
    {
    case TerminalType::Default:
        if (have_sdl)
        {
            term = std::make_unique<SDLTerminal>();
        }
        if (!term && have_curses)
        {
            term = std::make_unique<NcursesTerminal>();
        }
        break;
    case TerminalType::SDL:
        if (have_sdl)
        {
            term = std::make_unique<SDLTerminal>();
        }
        else
        {
            RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d): Please install SDL2",
                        (unsigned int)initData.FrontEnd);
        }
        break;
    case TerminalType::Terminal:
        if (have_curses)
        {
            term = std::make_unique<NcursesTerminal>();
        }
        else
        {
            RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d): Please install ncurses",
                        (unsigned int)initData.FrontEnd);
        }
        break;
    default:
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d)", (unsigned int)initData.FrontEnd);
    }

    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");

    if (term->Init(initData) == false)
    {
        RETURNERROR(nullptr, "Fail to initialize the terminal !");
    }
    return term;
}
