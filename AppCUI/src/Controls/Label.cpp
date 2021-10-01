#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

Label* Label::Create(Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    INIT_CONTROL(Label, ControlContext);
    Members->Layout.MinHeight = 1;
    Members->Layout.MinWidth  = 1;
    CHECK(Init(std::move(me), parent, caption, layout, true), nullptr, "Failed to create label !");
    Members->HotKey = AppCUI::Input::Key::None; // A label can draw a hot key, but does not have an associated one
    Members->Flags  = GATTR_ENABLE | GATTR_VISIBLE;
    return p_me;
}

void Label::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );
    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey);
    params.X              = 0;
    params.Y              = 0;
    params.Color          = Members->Cfg->Label.NormalColor;
    params.HotKeyColor    = Members->Cfg->Label.HotKeyColor;
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