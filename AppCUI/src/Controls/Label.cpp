#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

Label::Label(const AppCUI::Utils::ConstString& caption, std::string_view layout)
    : Control(new ControlContext(), caption, layout, true)
{
    auto Members = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinHeight = 1;
    Members->Layout.MinWidth  = 1;
    Members->HotKey = AppCUI::Input::Key::None; // A label can draw a hot key, but does not have an associated one
    Members->Flags  = GATTR_ENABLE | GATTR_VISIBLE;
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