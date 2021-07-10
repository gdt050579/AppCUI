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
        Members->Syntax.Handler(control, Members->Text.GetBuffer(), Members->Text.Len(), Members->Syntax.Context);
	}
	control->RaiseEvent(Event::EVENT_TEXT_CHANGED);
}

void TextField_MoveSelTo(TextField *control,int poz)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Selection.Start == -1)
        return;
	if (poz == Members->Selection.Origin) { 
        control->ClearSelection(); 
        Members->Modified = true;
        return; 
    }
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
    Members->Modified = true;
	//if (poz<SelStart) SelStart=poz; else SelEnd=poz-1;
}
void TextField_MoveTo(TextField *control, int newPoz, bool selected)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	int c_width = C_WIDTH;
	if ((!selected) && (Members->Selection.Start!= -1))
		control->ClearSelection();
    if ((Members->Cursor.Pos == Members->Text.Len()) && (newPoz > Members->Cursor.Pos))
        return;


	if ((selected) && (Members->Selection.Start== -1))
	{
		Members->Selection.Start= Members->Selection.End = Members->Selection.Origin = Members->Cursor.Pos;
	}
	while (Members->Cursor.Pos != newPoz)
	{
		if (Members->Cursor.Pos>newPoz) Members->Cursor.Pos--; else if (Members->Cursor.Pos<newPoz) Members->Cursor.Pos++;
		if (Members->Cursor.Pos<0) { Members->Cursor.Pos = newPoz = 0; }
        if (Members->Cursor.Pos == Members->Text.Len())
            newPoz = Members->Cursor.Pos;
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
    Members->Modified = true;
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
    Members->Modified = true;
}
void TextField_KeyBack(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	EXIT_IF_READONLY();
	if (Members->Selection.Start!= -1) 
    { 
        TextField_DeleteSelected(control);
        Members->Modified = true;
        return; 
    }
	if (Members->Cursor.Pos == 0) 
        return;
	Members->Text.DeleteChar(Members->Cursor.Pos - 1);
	TextField_MoveTo(control, Members->Cursor.Pos - 1, false);
	TextField_SendTextChangedEvent(control);
    Members->Modified = true;
}
void TextField_KeyDelete(TextField *control)
{
	CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
	EXIT_IF_READONLY();
	if (Members->Selection.Start!= -1) 
    { 
        TextField_DeleteSelected(control);
        Members->Modified = true;
        return; 
    }
	Members->Text.DeleteChar(Members->Cursor.Pos);
	TextField_SendTextChangedEvent(control);
    Members->Modified = true;
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
        Members->Modified = true;
	}
}
void TextField_CopyToClipboard(TextField *control)
{

}
void TextField_PasteFromClipboard(TextField *control)
{
    //Members->Modified = true;
}
//============================================================================
TextField::~TextField()
{
	DELETE_CONTROL_CONTEXT(TextFieldControlContext);
}
bool TextField::Create(Control *parent, const char * text, const char * layout, TextFieldFlags::Type flags, Handlers::SyntaxHighlightHandler handler, void* handlerContext)
{
	CONTROL_INIT_CONTEXT(TextFieldControlContext);
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, false);
    Members->Layout.MinWidth = 3;
    Members->Layout.MinHeight = 1;
    Members->Syntax.Handler = nullptr;
    Members->Syntax.Context = nullptr;
	
    CHECK(Init(parent, text, layout, false), false, "Failed to create text field !");
	
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int)flags;
    Members->Modified = true;   
	ClearSelection();
	Members->Cursor.Pos = Members->Cursor.StartOffset = 0;
	TextField_MoveTo(this, 0xFFFF, false);
	if (Members->Flags & TextFieldFlags::SYNTAX_HIGHLIGHTING) {
        Members->Syntax.Handler = handler;
        Members->Syntax.Context = handlerContext;
        CHECK(handler, false, "if 'TextFieldFlags::SYNTAX_HIGHLIGHTING` is set a syntaxt highligh handler must be provided");
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
    Members->Modified = true;
}
void TextField::ClearSelection()
{
	CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
	Members->Selection.Start= Members->Selection.End= Members->Selection.Origin = -1;
    Members->Modified = true;
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
				RaiseEvent(Event::EVENT_TEXTFIELD_VALIDATE);
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
    
    WriteCharacterBufferParams params(WriteCharacterBufferFlags::BUFFER_RANGE | WriteCharacterBufferFlags::WRAP_TO_WIDTH);
    params.Start = Members->Cursor.StartOffset;
    params.End = Members->Text.Len();
    params.Width = Members->Layout.Width - 2;
    if (Members->Layout.Height == 1)
        params.Flags |= WriteCharacterBufferFlags::SINGLE_LINE;
    else
        params.Flags |= WriteCharacterBufferFlags::MULTIPLE_LINES;

    if (!this->IsEnabled())
        params.Color = Members->Cfg->Text.Inactive.Text;
    else if (Members->Focused)
        params.Color = Members->Cfg->Text.Focus.Text;
    else if (Members->MouseIsOver)
        params.Color = Members->Cfg->Text.Hover.Text;
    else
        params.Color = Members->Cfg->Text.Normal.Text;

    renderer.Clear(' ', params.Color);

    if (Members->Focused)
    {
        if (Members->Modified)
        {
            if (Members->Flags & TextFieldFlags::SYNTAX_HIGHLIGHTING)
            {
                Members->Syntax.Handler(this, Members->Text.GetBuffer(), Members->Text.Len(), Members->Syntax.Context);
            } else 
                Members->Text.SetColor(params.Color);
            if ((Members->Selection.Start >= 0) && (Members->Selection.End >= 0) && (Members->Selection.End >= Members->Selection.Start))
                Members->Text.SetColor(Members->Selection.Start, Members->Selection.End + 1, Members->Cfg->Text.SelectionColor);
            Members->Modified = false;
        }
        else {
            // if no selection is present and no syntax highlighting --> use overwrite colors as it is faster
            if ((Members->Selection.Start<0) && ((Members->Flags & TextFieldFlags::SYNTAX_HIGHLIGHTING)==0))
                params.Flags |= WriteCharacterBufferFlags::OVERWRITE_COLORS;
        }
    }
    else {
        params.Flags |= WriteCharacterBufferFlags::OVERWRITE_COLORS;        
    }
    renderer.WriteCharacterBuffer(1, 0, Members->Text, params);
    if (Members->Focused)
    {       
        int y = (Members->Cursor.Pos - Members->Cursor.StartOffset) / params.Width;
        int x = (Members->Cursor.Pos - Members->Cursor.StartOffset) % params.Width;
        if ((x == 0) && (y == Members->Layout.Height)) {
            x = params.Width;
            y--;
        }
        renderer.SetCursor(x + 1, y);
    }
}
void TextField::OnFocus()
{
	SelectAll();
}
bool TextField::OnMouseEnter()
{
    return true;
}
bool TextField::OnMouseLeave()
{
    return true;
}

//*/