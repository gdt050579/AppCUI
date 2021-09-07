#include "Terminal/TerminalFactory.hpp"

using namespace AppCUI::Internal;

#ifdef HAVE_SDL
#    include "Terminal/SDLTerminal/SDLTerminal.hpp"
#endif

#ifdef HAVE_CURSES
#    include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
#endif

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData& initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;

    switch (initData.FrontEnd)
    {
    case TerminalType::Default:
    case TerminalType::SDL:
#ifdef HAVE_SDL
        term = std::make_unique<SDLTerminal>();
#else
        RETURNERROR(
              nullptr,
              "Unsuported terminal type for UNIX OS (%d): Please install SDL2",
              (unsigned int) initData.FrontEnd);
#endif
        break;
    case TerminalType::Terminal:
#ifdef HAVE_CURSES
        term = std::make_unique<NcursesTerminal>();
#else
        RETURNERROR(
              nullptr,
              "Unsuported terminal type for UNIX OS (%d): Please install ncurses",
              (unsigned int) initData.FrontEnd);
#endif
        break;
    default:
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d)", (unsigned int) initData.FrontEnd);
    }

    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");

    if (term->Init(initData) == false)
    {
        RETURNERROR(nullptr, "Fail to initialize the terminal !");
    }
    return term;
}
