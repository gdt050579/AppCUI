#include "ControlContext.hpp"

namespace AppCUI::Controls
{
Button::Button(const ConstString& caption, string_view layout, int controlID, ButtonFlags flags)
    : Control(new ControlContext(), caption, layout, true)
{
    auto Members = reinterpret_cast<ControlContext*>(this->Context);

    if ((flags & ButtonFlags::Flat) != ButtonFlags::None)
    {
        Members->Layout.MinWidth  = 3;
        Members->Layout.MinHeight = 1; // one character (flat button)
        Members->Layout.MaxHeight = 1;
        Members->Layout.Height    = 1;
    }
    else
    {
        Members->Layout.MinWidth  = 4;
        Members->Layout.MinHeight = 2; // Exactly 2 characters
        Members->Layout.MaxHeight = 2;
        Members->Layout.Height    = 2;
    }
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | flags;

    this->SetControlID(controlID);
}

void Button::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);

    const auto btnState   = Members->GetControlState(ControlStateFlags::All);
    params.Color          = Members->Cfg->Button.Text.GetColor(btnState);
    params.HotKeyColor    = Members->Cfg->Button.HotKey.GetColor(btnState);
    bool pressed          = IsChecked();
    params.Y              = 0;
    params.HotKeyPosition = Members->HotKeyOffset;
    params.Align          = TextAlignament::Center;

    if (Members->Flags && ButtonFlags::Flat)
    {
        params.X     = 0;
        params.Width = Members->Layout.Width;
        renderer.FillHorizontalLine(0, 0, Members->Layout.Width, ' ', params.Color);
        renderer.WriteText(Members->Text, params);
    }
    else
    {
        params.Width = Members->Layout.Width - 1;
        if (pressed)
        {
            renderer.FillHorizontalLine(1, 0, Members->Layout.Width, ' ', params.Color);
            params.X = 1;
            renderer.WriteText(Members->Text, params);
        }
        else
        {
            renderer.FillHorizontalLine(0, 0, Members->Layout.Width - 2, ' ', params.Color);
            params.X = 0;
            renderer.WriteText(Members->Text, params);

            renderer.FillHorizontalLineWithSpecialChar(
                  1, 1, Members->Layout.Width, SpecialChars::BlockUpperHalf, Members->Cfg->Button.ShadowColor);
            renderer.WriteSpecialCharacter(
                  Members->Layout.Width - 1, 0, SpecialChars::BlockLowerHalf, Members->Cfg->Button.ShadowColor);
        }
    }
}
void Button::OnHotKey()
{
    CREATE_CONTROL_CONTEXT(this, Members, );
    if (Members->handlers)
    {
        auto bh = this->Handlers();
        if (bh->OnButtonPressed.obj)
        {
            bh->OnButtonPressed.obj->OnButtonPressed(this);
            return;
        }
    }

    // if no handler is present --> call RaiseEvent
    RaiseEvent(Event::ButtonClicked);
}
bool Button::OnKeyEvent(Key KeyCode, char16)
{
    if ((KeyCode == Key::Space) || (KeyCode == Key::Enter))
    {
        OnHotKey();
        return true;
    }
    return false;
}
bool Button::OnMouseDrag(int x, int y, Input::MouseButton, Input::Key)
{
    if (IsChecked() == false)
        return false;
    if (IsMouseInControl(x, y) == false)
    {
        SetChecked(false);
        return true;
    }
    return false;
}
void Button::OnMouseReleased(int x, int y, Input::MouseButton, Input::Key)
{
    SetChecked(false);
    if (IsMouseInControl(x, y))
        OnHotKey();
}
void Button::OnMousePressed(int, int, Input::MouseButton, Input::Key)
{
    SetChecked(true);
}
bool Button::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if ((int) Members->Text.Len() >= Members->Layout.Width)
        this->ShowToolTip(Members->Text);
    return true;
}
bool Button::OnMouseLeave()
{
    return true;
}
Handlers::Button* Button::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::Button);
}
} // namespace AppCUI::Controls