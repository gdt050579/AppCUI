#pragma once

#include "../ControlContext.hpp"

namespace AppCUI
{
namespace Internal
{
    using TerminalCreator = unique_ptr<AbstractTerminal> (*)();

    // When FrontendType::Default is requested, the registered frontend with the highest priority is created (on a tie,
    // the one with the lowest FrontendType value). A frontend registered with 'None' is only created when it is
    // requested explicitly.
    enum class DefaultFrontendPriority : uint8
    {
        None     = 0,
        Fallback = 1, // usable as a default (e.g. SDL) but a native frontend is preferred
        Native   = 2, // the native frontend of the platform (Windows console, ncurses, ...)
    };

    // Frontends are linked into libAppCUI as separate components (AppCUI::Frontend::xxx CMake targets) and register
    // themselves at static-initialization time through a TerminalRegistrar object (no logging is possible at that
    // point: an invalid or duplicated registration is ignored and RegisterTerminal returns false). TestTerminal and
    // CustomTerminal are part of AppCUI::Core and are always available.
    bool RegisterTerminal(
          Application::FrontendType frontend,
          TerminalCreator creator,
          DefaultFrontendPriority defaultPriority);

    template <typename T>
    unique_ptr<AbstractTerminal> CreateTerminal()
    {
        return std::make_unique<T>();
    }

    struct TerminalRegistrar
    {
        TerminalRegistrar(
              Application::FrontendType frontend,
              TerminalCreator creator,
              DefaultFrontendPriority defaultPriority = DefaultFrontendPriority::None)
        {
            RegisterTerminal(frontend, creator, defaultPriority);
        }
    };

    // Creates and initializes the terminal requested by initData.Frontend (fails if that frontend is not linked).
    unique_ptr<AbstractTerminal> GetTerminal(const Application::InitializationData& initData);
}
} // namespace AppCUI
