#include "ControlContext.hpp"

namespace AppCUI::Controls
{
Label::Label(const ConstString& caption, string_view layout) : Control(new ControlContext(), caption, layout, true)
{
    auto Members              = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinHeight = 1;
    Members->Layout.MinWidth  = 1;
    Members->HotKey           = Input::Key::None; // A label can draw a hot key, but does not have an associated one
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE;
}

void Label::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );
    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey);
    const bool enabled    = (Members->Flags & GATTR_ENABLE) != 0;
    params.X              = 0;
    params.Y              = 0;
    params.Color          = enabled ? Members->Cfg->Text.Normal : Members->Cfg->Text.Inactive;
    params.HotKeyColor    = enabled ? Members->Cfg->Text.HotKey : Members->Cfg->Text.Inactive;
    params.HotKeyPosition = Members->HotKeyOffset;
    if (Members->Layout.Height == 1)
    {
        params.Flags |= WriteTextFlags::SingleLine;
    }
    else
    {
        params.Flags |= WriteTextFlags::MultipleLines | WriteTextFlags::WrapToWidth;
        params.Width = Members->Layout.Width;
    }
    renderer.WriteText(Members->Text, params);
}
} // namespace AppCUI::Controls