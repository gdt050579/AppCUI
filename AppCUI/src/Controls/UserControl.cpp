#include "ControlContext.hpp"

namespace AppCUI::Controls
{
UserControl::UserControl(const ConstString& caption, string_view layout, bool verticalScrollBar)
    : Control(new ControlContext(), caption, layout, false)
{
    auto cc   = reinterpret_cast<ControlContext*>(this->Context);
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    if (verticalScrollBar)
    {
        cc->Flags |= GATTR_VSCROLL;
        cc->ScrollBars.OutsideControl = true;
    }
}

UserControl::UserControl(string_view layout, bool verticalScrollBar) : Control(new ControlContext(), "", layout, false)
{
    auto cc   = reinterpret_cast<ControlContext*>(this->Context);
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    if (verticalScrollBar)
    {
        cc->Flags |= GATTR_VSCROLL;
        cc->ScrollBars.OutsideControl = true;
    }
}
} // namespace AppCUI::Controls
