#include <string.h>
#include "Terminal/TerminalFactory.hpp"
#include "Terminal/WindowsTerminal/WindowsTerminal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Application;

std::unique_ptr<AbstractTerminal> AppCUI::Internal::GetTerminal(const InitializationData& initData)
{
    std::unique_ptr<AbstractTerminal> term = nullptr;
    switch (initData.Frontend)
    {
    case FrontendType::Default:
    case FrontendType::WindowsConsole:
        term = std::make_unique<WindowsTerminal>();
        break;
    default:
        RETURNERROR(nullptr, "Unsuported terminal type for Windows OS (%d)", (unsigned int) initData.Frontend);
    }
    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");
    CHECK(term->Init(initData), nullptr, "Fail to initialize the terminal!");

    return term;
}
