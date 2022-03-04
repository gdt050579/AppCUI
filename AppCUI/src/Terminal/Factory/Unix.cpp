#include "../TerminalFactory.hpp"

#ifdef APPCUI_HAVE_SDL
#    include "../SDLTerminal/SDLTerminal.hpp"
#endif

#ifdef APPCUI_HAVE_NCURSES
#    include "../NcursesTerminal/NcursesTerminal.hpp"
#endif

namespace AppCUI::Internal
{
using namespace Application;

unique_ptr<AbstractTerminal> GetTerminal(const InitializationData& initData)
{
    unique_ptr<AbstractTerminal> term = nullptr;

    switch (initData.Frontend)
    {
    case FrontendType::Default:
    case FrontendType::SDL:
#ifdef APPCUI_HAVE_SDL
        term = std::make_unique<SDLTerminal>();
#else
        RETURNERROR(
              nullptr,
              "Unsuported terminal type for UNIX OS (%d): Please install SDL2",
              (uint32) initData.Frontend);
#endif
        break;
    case FrontendType::Terminal:
#ifdef APPCUI_HAVE_NCURSES
        term = std::make_unique<NcursesTerminal>();
#else
        RETURNERROR(
              nullptr,
              "Unsuported terminal type for UNIX OS (%d): Please install ncurses",
              (uint32) initData.Frontend);
#endif
        break;
    default:
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d)", (uint32) initData.Frontend);
    }

    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");
    CHECK(term->Init(initData), nullptr, "Fail to initialize the terminal!");

    return term;
}
}