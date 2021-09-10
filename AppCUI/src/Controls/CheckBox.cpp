#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

bool AppCUI::Controls::CheckBox::Create(Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int controlID)
{
    CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;
    CHECK(Init(parent, caption, layout, true), false, "Unable to create check box !");
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    SetControlID(controlID);
    return true;
}
void AppCUI::Controls::CheckBox::Paint(Graphics::Renderer& renderer)
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
void AppCUI::Controls::CheckBox::OnHotKey()
{
    SetChecked(!IsChecked());
    RaiseEvent(Event::CheckedStatusChanged);
}
bool AppCUI::Controls::CheckBox::OnKeyEvent(AppCUI::Input::Key KeyCode, char16_t)
{
    if (KeyCode == Key::Space)
    {
        OnHotKey();
        return true;
    }
    return false;
}
void AppCUI::Controls::CheckBox::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton)
{
    if (IsMouseInControl(x, y))
        OnHotKey();
}
bool AppCUI::Controls::CheckBox::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if ((int) Members->Text.Len() >= Members->Layout.Width)
        this->ShowToolTip(Members->Text);
    return true;
}
bool AppCUI::Controls::CheckBox::OnMouseLeave()
{
    return true;
}
