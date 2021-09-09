#ifndef _TERMINAL_FACTORY_INCLUDED_
#define _TERMINAL_FACTORY_INCLUDED_
#include "Internal.hpp"
#include <memory>

namespace AppCUI
{
namespace Internal
{
    std::unique_ptr<AbstractTerminal> GetTerminal(const AppCUI::Application::InitializationData& initData);
}
} // namespace AppCUI

#endif // _TERMINAL_FACTORY_INCLUDED_
