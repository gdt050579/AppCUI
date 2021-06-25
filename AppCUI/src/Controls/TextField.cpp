#include "../../include/ControlContext.h"

using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

#define C_WIDTH		        ((Members->Layout.Width-2)*Members->Layout.Height)
#define EXIT_IF_READONLY()	if ((Members->Flags & TextFieldFlags::READONLY_TEXT)!=0) { return; };

#define DEFAULT_TEXT_COLOR	0xFFFFFFFF
//*
void TextField_SendTextChangedEvent(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	if (Members->Flags & TextFieldFlags::SYNTAX_HIGHLIGHTING)
	{
        // todo highlight update
	}
	control->RaiseEvent(Events::EVENT_TEXT_CHANGED);
}

void TextField_MoveSelTo(TextField *control,int poz)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	if (Members->Selection.Start== -1) return;
	if (poz == Members->Selection.Origin) { control->ClearSelection(); return; }
	if (poz<Members->Selection.Origin)
	{
        Members->Selection.Start = poz;
		Members->Selection.End= Members->Selection.Origin - 1;
	}
	else
	if (poz>Members->Selection.Origin)
	{
        Members->Selection.End = poz - 1;
		Members->Selection.Start= Members->Selection.Origin;
	}
	//if (poz<SelStart) SelStart=poz; else SelEnd=poz-1;
}
void TextField_MoveTo(TextField *control, int newPoz, bool selected)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	int c_width = C_WIDTH;
	if ((!selected) && (Members->Selection.Start!= -1))
		control->ClearSelection();
	const char * Text = Members->Text.GetText();
	if ((Text[Members->Cursor.Pos] == 0) && (newPoz>Members->Cursor.Pos))
		return;
	if ((selected) && (Members->Selection.Start== -1))
	{
		Members->Selection.Start= Members->Selection.End = Members->Selection.Origin = Members->Cursor.Pos;
	}
	while (Members->Cursor.Pos != newPoz)
	{
		if (Members->Cursor.Pos>newPoz) Members->Cursor.Pos--; else if (Members->Cursor.Pos<newPoz) Members->Cursor.Pos++;
		if (Members->Cursor.Pos<0) { Members->Cursor.Pos = newPoz = 0; }
		if (Text[Members->Cursor.Pos] == 0) newPoz = Members->Cursor.Pos;
		if (Members->Cursor.Pos<Members->Cursor.StartOffset) Members->Cursor.StartOffset = Members->Cursor.Pos;
		if (Members->Cursor.Pos>Members->Cursor.StartOffset + c_width) Members->Cursor.StartOffset = Members->Cursor.Pos - c_width;
	}
	if (selected)
		TextField_MoveSelTo(control, Members->Cursor.Pos);
}
void TextField_DeleteSelected(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	EXIT_IF_READONLY();
	int ss, se;
	if (Members->Selection.Start== -1) return;
	ss = Members->Selection.Start; se = Members->Selection.End;
	if (Members->Selection.Start<Members->Selection.Origin)
	{
		Members->Selection.Start++;
	}
	if (Members->Cursor.Pos >= Members->Selection.Start)
	{
		TextField_MoveTo(control, Members->Cursor.Pos - (se - ss + 1), false);
	}
	Members->Text.Delete(ss, se+1);
	control->ClearSelection();
}
void TextField_AddChar(TextField *control, char ch)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	EXIT_IF_READONLY();
	TextField_DeleteSelected(control);
    if (Members->Cursor.Pos > (int)Members->Text.Len())
        Members->Text.InsertChar(ch, Members->Text.Len());
    else
        Members->Text.InsertChar(ch, (unsigned int)(Members->Cursor.Pos));
	TextField_MoveTo(control, Members->Cursor.Pos + 1, false);
	TextField_SendTextChangedEvent(control);
}
void TextField_KeyBack(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	EXIT_IF_READONLY();
	if (Members->Selection.Start!= -1) { TextField_DeleteSelected(control); return; }
	if (Members->Cursor.Pos == 0) return;
	Members->Text.DeleteChar(Members->Cursor.Pos - 1);
	TextField_MoveTo(control, Members->Cursor.Pos - 1, false);
	TextField_SendTextChangedEvent(control);
}
void TextField_KeyDelete(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	EXIT_IF_READONLY();
	if (Members->Selection.Start!= -1) { TextField_DeleteSelected(control); return; }
	Members->Text.DeleteChar(Members->Cursor.Pos);
	TextField_SendTextChangedEvent(control);;
}
bool TextField_HasSelection(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, false);
    return ((Members->Selection.Start >= 0) && (Members->Selection.End >= 0) && (Members->Selection.End >= Members->Selection.Start));
}
void TextField_SetSelection(TextField *control,int start, int end)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	if ((start >= 0) && (start <= end))
	{
		Members->Selection.Start= Members->Selection.Origin = start; 
		Members->Selection.End= end;
	}
}
void TextField_CopyToClipboard(TextField *control)
{

}
void TextField_PasteFromClipboard(TextField *control)
{
}
//============================================================================
TextField::~TextField()
{
	DELETE_CONTROL_CONTEXT(TextFieldControlContext);
}
bool TextField::Create(Control *parent, const char * text, const char * layout, TextFieldFlags::Type flags)
{
	CONTROL_INIT_CONTEXT(TextFieldControlContext);
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, false);
    Members->Layout.MinWidth = 3;
    Members->Layout.MinHeight = 1;
	CHECK(Init(parent, text, layout, false), false, "Failed to create text field !");
	
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int)flags;
	ClearSelection();
	Members->Cursor.Pos = Members->Cursor.StartOffset = 0;
	TextField_MoveTo(this, 0xFFFF, false);
	if (Members->Flags & TextFieldFlags::SYNTAX_HIGHLIGHTING) {
		// todo enable syntax coloring
	}
	return true;
}
void TextField::SelectAll()
{
	CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    Members->Selection.Start = 0;
	Members->Selection.End= Members->Text.Len() - 1;
	if (Members->Selection.End<0) 
		ClearSelection();
}
void TextField::ClearSelection()
{
	CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
	Members->Selection.Start= Members->Selection.End= Members->Selection.Origin = -1;
}

bool TextField::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, false);
	
	switch (keyCode)
	{
		case Key::Left:	
			TextField_MoveTo(this, Members->Cursor.Pos - 1, false); 
			return true;
		case Key::Right:	
			TextField_MoveTo(this, Members->Cursor.Pos + 1, false); 
			return true;
		case Key::Up:		
			TextField_MoveTo(this, Members->Cursor.Pos - (Members->Layout.Width - 2), false); 
			return true;
		case Key::Down:	
			TextField_MoveTo(this, Members->Cursor.Pos + (Members->Layout.Width - 2), false);
			return true;
		case Key::Home:	
			TextField_MoveTo(this, 0, false); 
			return true;
		case Key::End:		
			TextField_MoveTo(this, 0xFFFF, false); 
			return true;

		case Key::Shift | Key::Left:	
			TextField_MoveTo(this, Members->Cursor.Pos - 1, true); 
			return true;
		case Key::Shift | Key::Right:	
			TextField_MoveTo(this, Members->Cursor.Pos + 1, true); 
			return true;
		case Key::Shift | Key::Up:	
			TextField_MoveTo(this, Members->Cursor.Pos - (Members->Layout.Width - 2), true);
			return true;
		case Key::Shift | Key::Down:	
			TextField_MoveTo(this, Members->Cursor.Pos + (Members->Layout.Width - 2), true);
			return true;
		case Key::Shift | Key::Home:	
			TextField_MoveTo(this, 0, true); 
			return true;
		case Key::Shift | Key::End:	
			TextField_MoveTo(this, 0xFFFF, true); 
			return true;

		case Key::Backspace:	
			TextField_KeyBack(this); 
			return true;
		case Key::Delete:		
			TextField_KeyDelete(this); 
			return true;

		case Key::Ctrl | Key::A : 
			SelectAll(); 
			return true;
		case Key::Ctrl | Key::Insert :
		case Key::Ctrl | Key::C: 
			TextField_CopyToClipboard(this); 
			return true;
		case Key::Ctrl | Key::V: 
		case Key::Shift | Key::Insert:	
			TextField_PasteFromClipboard(this); 
			return true;

		case Key::Enter:
			if ((Members->Flags & TextFieldFlags::PROCESS_ENTER) != 0)
			{
				RaiseEvent(Events::EVENT_TEXTFIELD_VALIDATE);
				return true;
			}
			return false;
	}

	if (AsciiCode != 0)
	{
		TextField_AddChar(this, AsciiCode);
		return true;
	}

	return false;
}
void TextField::OnAfterSetText(const char *text)
{
	// repozitionez cursorul
	TextField_MoveTo(this, 0, false);
	TextField_MoveTo(this, 0xFFFF, false);
}

void TextField::Paint(Console::Renderer & renderer)
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    unsigned int color;

    if (!this->IsEnabled())
        color = Members->Cfg->TextField.InactiveColor;
    if (Members->Focused)
        color = Members->Cfg->TextField.FocusColor;
    else if (Members->MouseIsOver)
        color = Members->Cfg->TextField.HoverColor;
    else
        color = Members->Cfg->TextField.NormalColor;

    renderer.Clear(' ', color);
    if ((Members->Flags & TextFieldFlags::SYNTAX_HIGHLIGHTING) && (Members->Focused))
    {
    }
    else {
        const char * start = Members->Text.GetText() + Members->Cursor.StartOffset;
        int size = (int)(Members->Text.Len() - Members->Cursor.StartOffset);
        int y = 0;
        int w = Members->Layout.Width - 2;
        int txW;
        while ((size > 0) && (y< Members->Layout.Height))
        {
            txW = MINVALUE(size, w);
            renderer.WriteSingleLineText(1, y, start, color, txW);
            y++;
            start += txW;
            size -= txW;
        }
        if (Members->Focused)
        {
            y = (Members->Cursor.Pos - Members->Cursor.StartOffset) / w;
            renderer.SetCursor(((Members->Cursor.Pos - Members->Cursor.StartOffset) % w) + 1, y);
        }
    }
}
void TextField::OnFocus()
{
	SelectAll();
}

//*/