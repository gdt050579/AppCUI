#include "ControlContext.hpp"

using namespace Controls;
using namespace Graphics;
using namespace Input;

CheckBox::CheckBox(const Utils::ConstString& caption, std::string_view layout, int controlID)
    : Control(new ControlContext(), caption, layout, true)
{
    auto Members              = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    this->SetControlID(controlID);
}

void Controls::CheckBox::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    auto* cbc = &Members->Cfg->StateControl.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->StateControl.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->StateControl.Focused;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->StateControl.Hover;

    renderer.WriteSingleLineText(0, 0, "[ ] ", cbc->TextColor);

    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey);
    params.HotKeyPosition = Members->HotKeyOffset;
    params.X              = 4;
    params.Y              = 0;
    if (Members->Layout.Height == 1)
    {
        params.Color       = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteTextFlags::SingleLine;
    }
    else
    {
        params.Color       = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteTextFlags::MultipleLines | WriteTextFlags::WrapToWidth;
        params.Width = Members->Layout.Width - 4; // without the '[ ] ' characters
    }
    renderer.WriteText(Members->Text, params);

    if (IsChecked())
    {
        renderer.WriteCharacter(1, 0, 'X', cbc->StateSymbolColor);
    }
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}
void Controls::CheckBox::OnHotKey()
{
    SetChecked(!IsChecked());
    CREATE_CONTROL_CONTEXT(this, Members, );
    if (Members->handlers)
    {
        auto ch = this->Handlers();
        if (ch->OnCheck.obj)
        {
            ch->OnCheck.obj->OnCheck(this, IsChecked());
            return;
        }
    }
    RaiseEvent(Event::CheckedStatusChanged);
}
bool Controls::CheckBox::OnKeyEvent(Input::Key KeyCode, char16_t)
{
    if (KeyCode == Key::Space)
    {
        OnHotKey();
        return true;
    }
    return false;
}
void Controls::CheckBox::OnMouseReleased(int x, int y, Input::MouseButton)
{
    if (IsMouseInControl(x, y))
        OnHotKey();
}
bool Controls::CheckBox::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if ((int) Members->Text.Len() >= Members->Layout.Width)
        this->ShowToolTip(Members->Text);
    return true;
}
bool Controls::CheckBox::OnMouseLeave()
{
    return true;
}
// handlers covariant
Handlers::CheckState* CheckBox::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::CheckState);
}