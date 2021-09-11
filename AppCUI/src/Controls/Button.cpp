#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

bool Button::Create(
      Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int controlID,
      ButtonFlags flags)
{
    CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    
    Members->Layout.MinWidth  = 4;
    if ((flags & ButtonFlags::Flat) != ButtonFlags::None)
    {
        Members->Layout.MinHeight = 1; // one character (flat button)
        Members->Layout.MaxHeight = 1;
    }
    else
    {
        Members->Layout.MinHeight = 2; // Exactly 2 characters
        Members->Layout.MaxHeight = 2;
    }
    
    CHECK(Init(parent, caption, layout, true), false, "Unable to create check box !");

    Members->Flags         = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | flags;
    Members->Layout.Height = 2;
    SetControlID(controlID);
    return true;
}
void Button::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);

    auto* bc              = &Members->Cfg->Button.Normal;
    bool pressed          = false;
    params.Y              = 0;
    params.HotKeyPosition = Members->HotKeyOffset;
    params.Width          = Members->Layout.Width - 1;
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
    // draw
    int x;
    int sz = (int) Members->Text.Len();
    if (Members->Layout.Width >= 4)
    {
        if (sz > Members->Layout.Width - 3)
        {
            x  = 0;
            sz = Members->Layout.Width - 3;
        }
        else
        {
            x = (Members->Layout.Width - 1 - sz) >> 1;
        }
    }
    else
    {
        sz = 0;
    }

    if (pressed)
    {
        renderer.FillHorizontalLine(1, 0, Members->Layout.Width, ' ', Members->Cfg->Button.Focused.TextColor);
        if (sz > 0)
        {
            params.Color       = Members->Cfg->Button.Focused.TextColor;
            params.HotKeyColor = Members->Cfg->Button.Focused.HotKeyColor;
            params.X           = x + 1;            
            renderer.WriteText(Members->Text, params);
        }
    }
    else
    {
        renderer.FillHorizontalLine(0, 0, Members->Layout.Width - 2, ' ', bc->TextColor);
        if (sz > 0)
        {
            params.Color       = bc->TextColor;
            params.HotKeyColor = bc->HotKeyColor;
            params.X           = x ;
            renderer.WriteText(Members->Text, params);
        }
        if (!(Members->Flags && ButtonFlags::Flat))
        {
            // if not flat --> draw button shaddow
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
void Button::OnHotKey()
{
    RaiseEvent(Event::ButtonClicked);
}
bool Button::OnKeyEvent(Key KeyCode, char16_t)
{
    if ((KeyCode == Key::Space) || (KeyCode == Key::Enter))
    {
        OnHotKey();
        return true;
    }
    return false;
}
bool Button::OnMouseDrag(int x, int y, AppCUI::Input::MouseButton)
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
void Button::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton)
{
    SetChecked(false);
    if (IsMouseInControl(x, y))
        OnHotKey();
}
void Button::OnMousePressed(int, int, AppCUI::Input::MouseButton)
{
    SetChecked(true);
}
bool Button::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if ((int)Members->Text.Len() >= Members->Layout.Width)
        this->ShowToolTip(Members->Text);
    return true;
}
bool Button::OnMouseLeave()
{
    return true;
}