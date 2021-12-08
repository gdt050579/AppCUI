#include "ControlContext.hpp"

namespace AppCUI::Controls
{
Password::Password(const ConstString& caption, string_view layout)
    : Control(new ControlContext(), caption, layout, false)
{
    auto Members              = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinWidth  = 4;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    Members->Layout.Height    = 1;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}

void Password::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    auto col = &Members->Cfg->Password.Normal;

    if (!this->IsEnabled())
        col = &Members->Cfg->Password.Inactive;
    else if (Members->Focused)
        col = &Members->Cfg->Password.Focus;
    else if (Members->MouseIsOver)
        col = &Members->Cfg->Password.Hover;

    int sz = Members->Text.Len();
    if ((sz + 3) > Members->Layout.Width)
        sz = Members->Layout.Width - 3;

    renderer.FillHorizontalLineSize(0, 0, Members->Layout.Width, ' ', col->Text);
    if (this->IsChecked())
    {
        renderer.WriteSingleLineText(
              1, 0, Members->Text.SubString(Members->Text.Len() - (unsigned int) sz, Members->Text.Len()), col->Text);
        renderer.WriteSpecialCharacter(Members->Layout.Width - 1, 0, SpecialChars::CircleFilled, col->VisibleSign);
    }
    else
    {
        if (sz > 0)
            renderer.FillHorizontalLine(1, 0, sz, '*', col->Text);
        renderer.WriteSpecialCharacter(Members->Layout.Width - 1, 0, SpecialChars::CircleEmpty, col->VisibleSign);
    }
    if (Members->Focused)
        renderer.SetCursor(1 + sz, 0);
}

bool Password::OnKeyEvent(Key KeyCode, char16 characterCode)
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if (characterCode > 0)
    {
        CHECK(Members->Text.InsertChar(characterCode, Members->Text.Len()), false, "");
        return true;
    }
    if (KeyCode == Key::Backspace)
    {
        if (Members->Text.Len() > 0)
        {
            CHECK(Members->Text.DeleteChar(Members->Text.Len() - 1), false, "");
        }
        return true;
    }
    if (KeyCode == Key::Enter)
    {
        RaiseEvent(Event::PasswordValidate);
        return true;
    }
    if ((KeyCode == (Key::Ctrl | Key::V)) || (KeyCode == (Key::Shift | Key::Insert)))
    {
        LocalUnicodeStringBuilder<2048> temp;
        if (OS::Clipboard::GetText(temp))
            Members->Text.Add(temp);
        return true;
    }

    return false;
}
bool Password::OnMouseDrag(int x, int y, Input::MouseButton)
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
void Password::OnMouseReleased(int x, int y, Input::MouseButton)
{
    SetChecked(false);
}
void Password::OnMousePressed(int, int, Input::MouseButton)
{
    SetChecked(true);
}
bool Password::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    this->ShowToolTip("Click and hold to see the password");
    return true;
}
bool Password::OnMouseLeave()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    this->HideToolTip();
    return true;
}
} // namespace AppCUI::Controls