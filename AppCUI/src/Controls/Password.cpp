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

    const auto state = Members->GetControlState(ControlStateFlags::ProcessHoverStatus);
    const auto col   = Members->Cfg->Editor.GetColor(state);

    int sz = Members->Text.Len();
    if ((sz + 3) > Members->Layout.Width)
        sz = Members->Layout.Width - 3;

    renderer.FillHorizontalLineSize(0, 0, Members->Layout.Width, ' ', col);
    if (this->IsChecked())
    {
        renderer.WriteSingleLineText(
              1, 0, Members->Text.SubString(Members->Text.Len() - (uint32) sz, Members->Text.Len()), col);
        renderer.WriteSpecialCharacter(
              Members->Layout.Width - 1, 0, SpecialChars::CircleFilled, Members->Cfg->Symbol.Checked);
    }
    else
    {
        if (sz > 0)
            renderer.FillHorizontalLine(1, 0, sz, '*', col);
        renderer.WriteSpecialCharacter(
              Members->Layout.Width - 1, 0, SpecialChars::CircleEmpty, Members->Cfg->Symbol.Unchecked);
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
void Password::OnMouseReleased(int /*x*/, int /*y*/, Input::MouseButton)
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