#include "Terminal/TerminalFactory.hpp"
#ifdef HAVE_SDL
#include "Terminal/SDLTerminal/SDLTerminal.hpp"
#endif 

#ifdef HAVE_CURSES
#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
#endif 

using namespace AppCUI::Internal;

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData& initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;
    switch (initData.FrontEnd)
    {
#ifdef HAVE_SDL
        case TerminalType::Default:
        case TerminalType::SDL:
            term = std::make_unique<SDLTerminal>();
            break;
#endif
#if HAVE_CURSES
        case TerminalType::Terminal:
            term = std::make_unique<NcursesTerminal>();
#endif 
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
