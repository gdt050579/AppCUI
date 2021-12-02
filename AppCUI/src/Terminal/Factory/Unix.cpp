#include "../TerminalFactory.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Application;

#ifdef HAVE_SDL
#    include "../SDLTerminal/SDLTerminal.hpp"
#endif

#ifdef HAVE_CURSES
#    include "../NcursesTerminal/NcursesTerminal.hpp"
#endif

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData& initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;

    switch (initData.Frontend)
    {
    case FrontendType::Default:
    case FrontendType::SDL:
#ifdef HAVE_SDL
        term = std::make_unique<SDLTerminal>();
#else
        RETURNERROR(
              nullptr,
              "Unsuported terminal type for UNIX OS (%d): Please install SDL2",
              (unsigned int) initData.Frontend);
#endif
        break;
    case FrontendType::Terminal:
#ifdef HAVE_CURSES
        term = std::make_unique<NcursesTerminal>();
#else
        RETURNERROR(
              nullptr,
              "Unsuported terminal type for UNIX OS (%d): Please install ncurses",
              (unsigned int) initData.Frontend);
#endif
        break;
    default:
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d)", (unsigned int) initData.Frontend);
    }

    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");
    CHECK(term->Init(initData), nullptr, "Fail to initialize the terminal!");

    return term;
}
