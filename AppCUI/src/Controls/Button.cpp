#include "../../include/ControlContext.h"


using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;



bool Button::Create(Control *parent, const char *ss, const char * layout, int controlID)
{
	CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MaxHeight = 2;
    Members->Layout.MinHeight = 2; // Exactly 2 characters
    Members->Layout.MinWidth = 4;
	CHECK(Init(parent, ss, layout,  true), false, "Unable to create check box !");
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    Members->Layout.Height = 2;
	SetControlID(controlID);
	return true;
}
void Button::Paint(Console::Renderer & renderer)
{
	CREATE_CONTROL_CONTEXT(this, Members, );

    auto * bc = &Members->Cfg->Button.Normal;
    bool pressed = false;

	// daca e disable
	if (!IsEnabled())
	{
        bc = &Members->Cfg->Button.Inactive; 
    }
    else {
        if (IsChecked())
            pressed = true;
        else {
            if (this->HasFocus())
                bc = &Members->Cfg->Button.Focused;
            else {
                if (IsMouseOver())
                    bc = &Members->Cfg->Button.Hover;
            }
        }
    }
    // draw
    int x;
    int sz = (int)Members->Text.Len();
    if (Members->Layout.Width >= 4)
    {
        if (sz > Members->Layout.Width - 3)
        {
            x = 0;
            sz = Members->Layout.Width - 3;
        }
        else {
            x = (Members->Layout.Width - 2 - sz) >> 1; // one char could be &
        }
    }
    else {
        sz = 0;
    }

    if (pressed)
    {
        renderer.FillHorizontalLine(1, 0, Members->Layout.Width, ' ', Members->Cfg->Button.Focused.TextColor);
        if (sz > 0)
            renderer.WriteSingleLineTextWithHotKey(x + 2, 0, Members->Text.GetText(), Members->Cfg->Button.Focused.TextColor, Members->Cfg->Button.Focused.HotKeyColor, sz);
    }
    else {
        renderer.FillHorizontalLine(0, 0, Members->Layout.Width-2, ' ', bc->TextColor);
        if (sz > 0)
            renderer.WriteSingleLineTextWithHotKey(x + 1, 0, Members->Text.GetText(), bc->TextColor, bc->HotKeyColor, sz);
        renderer.FillHorizontalLineWithSpecialChar(1, 1, Members->Layout.Width, SpecialChars::BlockUpperHalf, COLOR(Color::Black, Color::Transparent));
        renderer.WriteSpecialCharacter(Members->Layout.Width-1, 0, SpecialChars::BlockLowerHalf, COLOR(Color::Black, Color::Transparent));
    }
}
void Button::OnHotKey()
{
	RaiseEvent(Events::EVENT_BUTTON_CLICKED);
}
bool Button::OnKeyEvent(Key::Type KeyCode, char AsciiCode)
{
	if ((KeyCode == Key::Space) || (KeyCode == Key::Enter))
	{
		OnHotKey();
		return true;
	}
	return false;
}
bool Button::OnMouseDrag(int x, int y, int Button)
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
void Button::OnMouseReleased(int x, int y, int butonState)
{
	SetChecked(false);
	if (IsMouseInControl(x,y))
		OnHotKey();
}
void Button::OnMousePressed(int x, int y, int butonState)
{
	SetChecked(true);
}
bool Button::OnMouseEnter()
{
    return true;
}
bool Button::OnMouseLeave()
{
    return true;
}