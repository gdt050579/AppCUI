#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

Password::Password(const AppCUI::Utils::ConstString& caption, std::string_view layout)
    : Control(new ControlContext(), caption, layout, false)
{
    auto Members = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinWidth  = 4;
    Members->Layout.MinHeight = 1; 
    Members->Layout.MaxHeight = 1;
    Members->Layout.Height    = 1;
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}

void Password::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    ColorPair color;

    if (!this->IsEnabled())
        color = Members->Cfg->Text.Inactive.Text;
    else if (Members->Focused)
        color = Members->Cfg->Text.Focus.Text;
    else if (Members->MouseIsOver)
        color = Members->Cfg->Text.Hover.Text;
    else
        color = Members->Cfg->Text.Normal.Text;

    auto sz = Members->Text.Len();
    if ((sz + 3) > Members->Layout.Width)
        sz = Members->Layout.Width - 3;
   
    if (this->IsChecked())
    {
        renderer.WriteSingleLineText(
              1, 0, Members->Text.SubString(Members->Text.Len() - sz, Members->Text.Len()), color);
    }
    else
    {
        renderer.FillHorizontalLine(1, 0, sz + 1, '*', color);
    }
    if (Members->Focused)
        renderer.SetCursor(1 + sz, 0);
}

bool Password::OnKeyEvent(Key KeyCode, char16_t characterCode)
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
    return false;
}
bool Password::OnMouseDrag(int x, int y, AppCUI::Input::MouseButton)
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
void Password::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton)
{
    SetChecked(false);
    if (IsMouseInControl(x, y))
        OnHotKey();
}
void Password::OnMousePressed(int, int, AppCUI::Input::MouseButton)
{
    SetChecked(true);
}
bool Password::OnMouseEnter()
{
    return true;
}
bool Password::OnMouseLeave()
{
    return true;
}