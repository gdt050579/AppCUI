#include "../TerminalFactory.hpp"
#include "../TestTerminal/TestTerminal.hpp"
#include "../CustomTerminal/CustomTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;

namespace
{
    struct TerminalEntry
    {
        TerminalCreator creator;
        DefaultFrontendPriority defaultPriority;
    };

    constexpr uint32 FRONTEND_TYPES_COUNT = static_cast<uint32>(FrontendType::Custom) + 1;
    static_assert(static_cast<uint32>(FrontendType::Default) == 0);
    static_assert(static_cast<uint32>(FrontendType::Tests) == 4);
    static_assert(static_cast<uint32>(FrontendType::Custom) == 5);

    // Indexed by FrontendType. Only constant expressions are used to initialize it (constant initialization), so it is
    // valid before any dynamic initializer runs -> the registrars of the frontends can use it regardless of the order
    // in which the static objects of the different translation units are initialized.
    TerminalEntry registeredTerminals[FRONTEND_TYPES_COUNT] = {
        { nullptr, DefaultFrontendPriority::None },                        // Default (resolved, never registered)
        { nullptr, DefaultFrontendPriority::None },                        // SDL            (AppCUI::Frontend::SDL)
        { nullptr, DefaultFrontendPriority::None },                        // Terminal       (AppCUI::Frontend::Ncurses)
        { nullptr, DefaultFrontendPriority::None },                        // WindowsConsole (AppCUI::Frontend::WindowsConsole)
        { CreateTerminal<TestTerminal>, DefaultFrontendPriority::None },   // Tests          (core)
        { CreateTerminal<CustomTerminal>, DefaultFrontendPriority::None }, // Custom         (core)
    };

    [[maybe_unused]] const char* GetFrontendName(uint32 index)
    {
        constexpr const char* names[FRONTEND_TYPES_COUNT] = { "Default", "SDL",   "Terminal",
                                                              "WindowsConsole", "Tests", "Custom" };
        return index < FRONTEND_TYPES_COUNT ? names[index] : "Unknown";
    }

    uint32 FindDefaultFrontend()
    {
        uint32 result     = FRONTEND_TYPES_COUNT;
        auto bestPriority = DefaultFrontendPriority::None;
        for (uint32 index = 0; index < FRONTEND_TYPES_COUNT; index++)
        {
            const auto& entry = registeredTerminals[index];
            if ((entry.creator != nullptr) && (entry.defaultPriority > bestPriority))
            {
                bestPriority = entry.defaultPriority;
                result       = index;
            }
        }
        return result;
    }
} // namespace

bool RegisterTerminal(FrontendType frontend, TerminalCreator creator, DefaultFrontendPriority defaultPriority)
{
    const auto index = static_cast<uint32>(frontend);
    if ((index == 0) || (index >= FRONTEND_TYPES_COUNT) || (creator == nullptr))
        return false;
    if (registeredTerminals[index].creator != nullptr)
        return false;
    registeredTerminals[index] = { creator, defaultPriority };
    return true;
}

unique_ptr<AbstractTerminal> GetTerminal(const InitializationData& initData)
{
    auto index = static_cast<uint32>(initData.Frontend);
    if (initData.Frontend == FrontendType::Default)
    {
        index = FindDefaultFrontend();
        CHECK(index < FRONTEND_TYPES_COUNT,
              nullptr,
              "FrontendType::Default requested, but no native frontend is linked into this build of AppCUI "
              "(enable an APPCUI_FRONTEND_xxx CMake option or request FrontendType::Tests / FrontendType::Custom)");
    }
    CHECK(index < FRONTEND_TYPES_COUNT, nullptr, "Unknown frontend type (%d)", index);
    CHECK(registeredTerminals[index].creator != nullptr,
          nullptr,
          "Frontend '%s' (%d) is not available: its backend is not linked into this build of AppCUI",
          GetFrontendName(index),
          index);

    unique_ptr<AbstractTerminal> term = registeredTerminals[index].creator();
    CHECK(term, nullptr, "Fail to allocate memory for a terminal !");
    CHECK(term->Init(initData), nullptr, "Fail to initialize the terminal!");

    return term;
}
} // namespace AppCUI::Internal
