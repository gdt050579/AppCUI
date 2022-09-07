#include "../TerminalFactory.hpp"
#include "../TestTerminal/TestTerminal.hpp"
#include "../SDLTerminal/SDLTerminal.hpp"
#include "../NcursesTerminal/NcursesTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;

static unique_ptr<AbstractTerminal> GetTerminalImpl(FrontendType frontend)
{
    switch (frontend)
    {
    case FrontendType::Default:
    case FrontendType::Terminal:
        return std::make_unique<NcursesTerminal>();
    case FrontendType::SDL:
        return std::make_unique<SDLTerminal>();
    case FrontendType::Tests:
        return std::make_unique<TestTerminal>();
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