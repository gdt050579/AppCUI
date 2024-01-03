#include "ControlContext.hpp"

namespace AppCUI::Controls
{
UserControl::UserControl(const ConstString& caption, string_view layout, UserControlFlags flags)
    : Control(new ControlContext(), caption, layout, false)
{
    auto cc   = reinterpret_cast<ControlContext*>(this->Context);
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    if ((flags & UserControlFlags::ShowVerticalScrollBar) != UserControlFlags::None)
    {
        cc->Flags |= GATTR_VSCROLL;
        cc->ScrollBars.TopMargin = 4;
    }

    if ((flags & UserControlFlags::ShowHorizontalScrollBar) != UserControlFlags::None)
    {
        cc->Flags |= GATTR_HSCROLL;
        cc->ScrollBars.LeftMargin = 4;
    }

    cc->ScrollBars.OutsideControl = ((flags & UserControlFlags::ScrollBarOutsideControl) != UserControlFlags::None);
}

UserControl::UserControl(string_view layout, UserControlFlags flags) : Control(new ControlContext(), "", layout, false)
{
    auto cc   = reinterpret_cast<ControlContext*>(this->Context);
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    if ((flags & UserControlFlags::ShowVerticalScrollBar) != UserControlFlags::None)
    {
        cc->Flags |= GATTR_VSCROLL;
        cc->ScrollBars.TopMargin = 4;
    }

    if ((flags & UserControlFlags::ShowHorizontalScrollBar) != UserControlFlags::None)
    {
        cc->Flags |= GATTR_HSCROLL;
        cc->ScrollBars.LeftMargin = 4;
    }

    cc->ScrollBars.OutsideControl = ((flags & UserControlFlags::ScrollBarOutsideControl) != UserControlFlags::None);
}
} // namespace AppCUI::Controls
