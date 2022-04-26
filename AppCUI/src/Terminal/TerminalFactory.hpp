#pragma once

#include "../ControlContext.hpp"

namespace AppCUI
{
namespace Internal
{
    unique_ptr<AbstractTerminal> GetTerminal(const Application::InitializationData& initData);
}
} // namespace AppCUI
