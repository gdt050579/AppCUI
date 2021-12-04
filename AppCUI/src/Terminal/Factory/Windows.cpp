#include <string.h>
#include "../TerminalFactory.hpp"
#include "../WindowsTerminal/WindowsTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;

unique_ptr<AbstractTerminal> Internal::GetTerminal(const InitializationData& initData)
{
    unique_ptr<AbstractTerminal> term = nullptr;
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
} // namespace AppCUI::Internal