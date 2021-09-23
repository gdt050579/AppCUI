#pragma once

#include "Internal.hpp"
#include <memory>

namespace AppCUI
{
namespace Internal
{
    std::unique_ptr<AbstractTerminal> GetTerminal(const AppCUI::Application::InitializationData& initData);
}
} // namespace AppCUI

