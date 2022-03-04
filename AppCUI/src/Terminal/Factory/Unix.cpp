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

static unique_ptr<AbstractTerminal> GetTerminalImpl(FrontendType frontend)
{
    switch (frontend)
    {
    case FrontendType::Default:
        if (auto term = GetTerminalImpl(FrontendType::SDL))
        {
            return term;
        }
        if (auto term = GetTerminalImpl(FrontendType::Terminal))
        {
            return term;
        }
        break;
    case FrontendType::SDL:
#ifdef APPCUI_HAVE_SDL
        return std::make_unique<SDLTerminal>();
#else
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d): Please install SDL2", (uint32) frontend);
#endif
        break;
    case FrontendType::Terminal:
#ifdef APPCUI_HAVE_NCURSES
        return std::make_unique<NcursesTerminal>();
#else
        RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d): Please install ncurses", (uint32) frontend);
#endif
        break;
    }
    RETURNERROR(nullptr, "Unsuported terminal type for UNIX OS (%d)", (uint32) frontend);
}

unique_ptr<AbstractTerminal> GetTerminal(const InitializationData& initData)
{
    unique_ptr<AbstractTerminal> term = GetTerminalImpl(initData.Frontend);
    CHECK(term, nullptr, "No terminal");
    CHECK(term->Init(initData), nullptr, "Fail to initialize the terminal!");

    return term;
}
} // namespace AppCUI::Internal