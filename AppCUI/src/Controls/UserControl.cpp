#include "ControlContext.hpp"

using namespace AppCUI::Controls;

UserControl::UserControl(const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
    : Control(new ControlContext(), caption, layout, false)
{
}
UserControl::UserControl(const std::string_view& layout) : Control(new ControlContext(), "", layout, false)
{
}
