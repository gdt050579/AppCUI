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

    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);

    const auto* bc        = &Members->Cfg->Button.Normal;
    bool pressed          = false;
    params.Y              = 0;
    params.HotKeyPosition = Members->HotKeyOffset;
    params.Align          = TextAlignament::Center;

    // daca e disable
    if (!IsEnabled())
    {
        bc = &Members->Cfg->Button.Inactive;
    }
    else
    {
        if (IsChecked())
            pressed = true;
        else
        {
            if (this->HasFocus())
                bc = &Members->Cfg->Button.Focused;
            else
            {
                if (IsMouseOver())
                    bc = &Members->Cfg->Button.Hover;
            }
        }
    }

    if (Members->Flags && ButtonFlags::Flat)
    {
        renderer.FillHorizontalLine(0, 0, Members->Layout.Width, ' ', bc->TextColor);
        params.Color       = bc->TextColor;
        params.HotKeyColor = bc->HotKeyColor;
        params.X           = 0;
        params.Width       = Members->Layout.Width;
        renderer.WriteText(Members->Text, params);
    }
    else
    {
        params.Width = Members->Layout.Width - 1;
        if (pressed)
        {
            renderer.FillHorizontalLine(1, 0, Members->Layout.Width, ' ', Members->Cfg->Button.Focused.TextColor);
            params.Color       = Members->Cfg->Button.Focused.TextColor;
            params.HotKeyColor = Members->Cfg->Button.Focused.HotKeyColor;
            params.X           = 1;
            renderer.WriteText(Members->Text, params);
        }
        else
        {
            renderer.FillHorizontalLine(0, 0, Members->Layout.Width - 2, ' ', bc->TextColor);

            params.Color       = bc->TextColor;
            params.HotKeyColor = bc->HotKeyColor;
            params.X           = 0;
            renderer.WriteText(Members->Text, params);

            renderer.FillHorizontalLineWithSpecialChar(
                  1,
                  1,
                  Members->Layout.Width,
                  SpecialChars::BlockUpperHalf,
                  ColorPair{ Color::Black, Color::Transparent });
            renderer.WriteSpecialCharacter(
                  Members->Layout.Width - 1,
                  0,
                  SpecialChars::BlockLowerHalf,
                  ColorPair{ Color::Black, Color::Transparent });
        }
    }
}

bool Password::OnKeyEvent(Key KeyCode, char16_t)
{
    if ((KeyCode == Key::Space) || (KeyCode == Key::Enter))
    {
        OnHotKey();
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