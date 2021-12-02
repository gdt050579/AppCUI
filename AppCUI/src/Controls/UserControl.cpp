#include "ControlContext.hpp"

using namespace AppCUI::Controls;

UserControl::UserControl(const AppCUI::Utils::ConstString& caption, std::string_view layout)
    : Control(new ControlContext(), caption, layout, false)
{
    auto Members   = reinterpret_cast<ControlContext*>(this->Context);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}
UserControl::UserControl(std::string_view layout) : Control(new ControlContext(), "", layout, false)
{
    auto Members   = reinterpret_cast<ControlContext*>(this->Context);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}
