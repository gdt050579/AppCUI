#include "Terminal/TerminalFactory.hpp"
#include "Terminal/WindowsTerminal/WindowsTerminal.hpp"
#include "Terminal/SDLTerminal/SDLTerminal.hpp"
#include <string.h>

using namespace AppCUI::Internal;

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData & initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;
    switch (initData.FrontEnd)
    {
        case TerminalType::Default:
        case TerminalType::Windows:
            term = std::make_unique<WindowsTerminal>();
            break;
        //case TerminalType::SDL:
        //    term = std::make_unique<SDLTerminal>();
        default:
            RETURNERROR(nullptr, "Unsuported terminal type for Windows OS (%d)", (unsigned int)initData.FrontEnd);
    }
    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");
    if (term->Init(initData) == false)
    {
        RETURNERROR(nullptr, "Fail to initialize the terminal !");
    }
    return term;
}
