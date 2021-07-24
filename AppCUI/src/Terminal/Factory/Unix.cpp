#include "Terminal/TerminalFactory.hpp"
#include "Terminal/SDLTerminal/SDLTerminal.hpp"

using namespace AppCUI::Internal;

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData& initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;
    switch (initData.FrontEnd)
    {
        case TerminalType::Default:
        case TerminalType::SDL:
            term = std::make_unique<SDLTerminal>();
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
