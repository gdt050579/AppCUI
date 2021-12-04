#pragma once

#include "../Internal.hpp"
#include <memory>

namespace AppCUI
{
namespace Internal
{
    unique_ptr<AbstractTerminal> GetTerminal(const Application::InitializationData& initData);
}
} // namespace AppCUI

