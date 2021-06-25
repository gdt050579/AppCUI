#include "../../include/ControlContext.h"


using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

bool AppCUI::Controls::CheckBox::Create(Control *parent,const char *ss, const char * layout, int controlID)
{
	CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MinWidth = 5;
    Members->Layout.MinHeight = 1;
	CHECK(Init(parent, ss, layout, true), false, "Unable to create check box !");	
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
	SetControlID(controlID);
	return true;
}
void AppCUI::Controls::CheckBox::Paint(Console::Renderer & renderer)
{
	CREATE_CONTROL_CONTEXT(this, Members, );
    
    auto * cbc = &Members->Cfg->StateControl.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->StateControl.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->StateControl.Focused;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->StateControl.Hover;

    renderer.WriteSingleLineText(0, 0, "[ ] ", cbc->TextColor, 4);

    WriteCharacterBufferParams params(WriteCharacterBufferFlags::OVERWRITE_COLORS);
    if (Members->Layout.Height == 1) {
        params.Color = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteCharacterBufferFlags::SINGLE_LINE;
    } else {
        params.Color = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteCharacterBufferFlags::MULTIPLE_LINES;
    }
    renderer.WriteCharacterBuffer(4, 0, Members->Text, params);

    if (IsChecked())
        renderer.WriteCharacter(1, 0, 'X', cbc->StateSymbolColor);   
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}
void AppCUI::Controls::CheckBox::OnHotKey()
{
	SetChecked(!IsChecked());
	RaiseEvent(Events::EVENT_CHECKED_STATUS_CHANGED);
}
bool AppCUI::Controls::CheckBox::OnKeyEvent(AppCUI::Input::Key::Type KeyCode, char AsciiCode)
{
	if (KeyCode == Key::Space)
	{
		OnHotKey();
		return true;
	}
	return false;
}
void AppCUI::Controls::CheckBox::OnMouseReleased( int x, int y, int butonState)
{
	if (IsMouseInControl(x,y))
		OnHotKey();
}
bool AppCUI::Controls::CheckBox::OnMouseEnter()
{
    return true;
}
bool AppCUI::Controls::CheckBox::OnMouseLeave()
{
    return true;
}
