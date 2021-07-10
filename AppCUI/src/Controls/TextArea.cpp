#include "../../include/ControlContext.h"
#include <Internal.h>

using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;
using namespace AppCUI::OS;

#define LINE_NUMBERS_WIDTH  4
#define INVALID_SELECTION   0xFFFFFFFF

#define CLEAR_SELECTION     \
    if ((!selected) && (Selection.Start != INVALID_SELECTION)) { ClearSel(); } \
    if ((selected) && (Selection.Start == INVALID_SELECTION)) { Selection.Origin = View.CurrentPosition; }

#define UPDATE_SELECTION    if (selected) MoveSelectionTo(View.CurrentPosition);


#define WRAPPER	((TextAreaControlContext*)this->Context)

void TextAreaControlContext::ComputeVisibleLinesAndRows()
{
    unsigned int extraY = 0;
    unsigned int extraX = 0;

    if (Flags & (unsigned int)TextAreaFlags::SHOW_LINE_NUMBERS)
        extraX += LINE_NUMBERS_WIDTH;
    if (Flags & (unsigned int)TextAreaFlags::BORDER)
    {
        extraX += 2;
        extraY += 2;
    }
    if (extraX < Layout.Width)
        this->View.VisibleRowsCount = ((unsigned int)Layout.Width) - extraX;
    else
        this->View.VisibleRowsCount = 0;
    if (extraY < Layout.Height)
        this->View.VisibleLinesCount = ((unsigned int)Layout.Height) - extraY;
    else
        this->View.VisibleLinesCount = 0;
}
void TextAreaControlContext::SelAll()
{
    if (Text.Len() == 0)
        ClearSel();
    else {
        Selection.Start = 0;
        Selection.Origin = 0;
        Selection.End = Text.Len();
    }
}
void TextAreaControlContext::ClearSel()
{
    Selection.Start = INVALID_SELECTION;
    Selection.End = INVALID_SELECTION;
    Selection.Origin = INVALID_SELECTION;
}
void TextAreaControlContext::MoveSelectionTo(unsigned int poz)
{
    if ((Selection.Origin == INVALID_SELECTION) || (poz==Selection.Origin))
    {
        ClearSel();
        return;
    }
    if (poz < Selection.Origin)
    {
        Selection.Start = poz;
        Selection.End = Selection.Origin;
    } else {
        Selection.Start = Selection.Origin;
        Selection.End = poz;
    }
}

void TextAreaControlContext::DeleteSelected()
{
	int ss,se;
	if (Selection.Start == INVALID_SELECTION) 
		return;
    Text.Delete(Selection.Start, Selection.End);
    View.CurrentPosition = Selection.Start;
	UpdateLines();
	ClearSel();
}

void TextAreaControlContext::UpdateView()
{
	unsigned int *pLines = Lines.GetUInt32Array();
    unsigned int cnt = Lines.Len();
    View.CurrentLine = 0;
    for (unsigned int tr = 0; tr < cnt; tr++)
    {
        if (View.CurrentPosition < pLines[tr])
            break;
        else
            View.CurrentLine = tr;
    }
    if (View.CurrentLine < View.TopLine)
	{
        View.TopLine = View.CurrentLine;
	}
    if (View.CurrentLine >= View.TopLine + View.VisibleLinesCount)
    {
        View.TopLine = View.CurrentLine - View.VisibleLinesCount;
    }
}
void TextAreaControlContext::UpdateLines()
{
    unsigned int txSize, lineNumber;
    Character* c = Text.GetBuffer();
    Character* s = c;
    Character* c_End = c + Text.Len();

    View.CurrentLine = 0;
    Lines.Clear();
    Lines.Push(0);  // first line
    lineNumber = 1; // at least one line

    do
	{
        while ((c < c_End) && (c->Code != NEW_LINE_CODE))
            c++;
        if (c < c_End) // a new line was found
        {
            c++;
            txSize = (unsigned int)(c - s);
            if (View.CurrentPosition >= txSize)
                View.CurrentLine = lineNumber;
            Lines.Push(txSize);
            lineNumber++;            
        }
    } while (c < c_End);
	UpdateView();
}
unsigned int  TextAreaControlContext::GetLineSize(unsigned int lineIndex)
{
    unsigned int linesCount = Lines.Len();
    unsigned int *p = Lines.GetUInt32Array();
    if ((lineIndex + 1) < linesCount)
        return p[lineIndex + 1] - (p[lineIndex] + 1); // ingnore the final NEW_LINE character
    if (lineIndex >= linesCount)
        return 0; // line index outside 
    return Text.Len() - p[lineIndex];
}
unsigned int  TextAreaControlContext::GetLineStart(unsigned int lineIndex)
{
    unsigned int value;
    if (Lines.Get(lineIndex, value) == false)
        return 0;
    return value;
}
void TextAreaControlContext::AnalyzeCurrentText()
{
	UpdateLines();
    View.CurrentLine = 0;
    View.CurrentPosition = 0;
    View.HorizontalOffset = 0;
 //   cLocation = px = 0;
	//MoveTo(textSize,false);
}
void TextAreaControlContext::SetTabCharacter(char tabCharacter)
{
	tabChar = tabCharacter;
}
void TextAreaControlContext::SetColorFunction(Handlers::TextAreaSyntaxHighlightHandler GetLineColorFunction, void *Context)
{
	fnGetLineColor = GetLineColorFunction;
	//colorPData = Context;
}

void TextAreaControlContext::DrawToolTip()
{
}
void TextAreaControlContext::DrawLine(Console::Renderer & renderer, unsigned int lineIndex, int ofsX, int pozY,const ColorPair textColor)
{
    unsigned int    poz, lnSize;
	int				c,pozX;
	int				tr,cursorPoz;
    bool            useHighlighing;
	Character*		ch;
    ColorPair       col;

    if (lineIndex >= Lines.Len())
        return;
    poz = *(Lines.GetUInt32Array() + lineIndex);
    if (poz >= Text.Len())
        return;

    lnSize = GetLineSize(lineIndex);
    ch = Text.GetBuffer() + poz;
    pozX = ofsX - ((int)View.HorizontalOffset);
    useHighlighing = false;
    cursorPoz = -1;
    col = textColor;
    for (tr = 0; tr < lnSize; tr++, poz++, ch++)
    {
        if (Flags & GATTR_ENABLE)
        {
            if (poz == View.CurrentPosition)
                cursorPoz = pozX;
            if ((poz >= Selection.Start) && (poz < Selection.End))
                col = Cfg->Text.SelectionColor;
            else if (useHighlighing)
                col = ch->Color;
            else
                col = textColor;
        }
        
        renderer.WriteCharacter(pozX, pozY, ch->Code, col);

        if (ch->Code == '\t')
        {
            if ((pozX % 4) == 0)
                pozX += 4;
            else
                pozX = ((pozX >> 2) + 1) << 2;
        }
        else {
            pozX++;
        }
        if (pozX > (int)View.VisibleRowsCount)
            break;
    }
    if (Flags & GATTR_ENABLE)
    {
        if (poz == View.CurrentPosition)
            cursorPoz = pozX;
        if (cursorPoz>=0)
            renderer.SetCursor(cursorPoz, pozY);
    }
}
void TextAreaControlContext::DrawLineNumber(Console::Renderer & renderer, int lineIndex,int pozY, const ColorPair lineNumberColor)
{
	char temp[32];
	int poz = 30;
	temp[31] = 0;
	lineIndex++;

	do
	{
		temp[poz--] = lineIndex % 10 + '0';
		lineIndex /= 10;
	} while (lineIndex > 0);
	while (poz > 27) {
		temp[poz--] = ' ';
	}
	if (poz < 27)
		temp[28] = '.';

    renderer.WriteSingleLineText(0, pozY, temp + 28, lineNumberColor, LINE_NUMBERS_WIDTH-1);
}
void TextAreaControlContext::Paint(Console::Renderer & renderer)
{
    auto col = &this->Cfg->Text.Normal;
    if ((this->Flags & GATTR_ENABLE)==0)
        col = &this->Cfg->Text.Inactive;
    else if (this->Focused)
        col = &this->Cfg->Text.Focus;
    else if (this->MouseIsOver)
        col = &this->Cfg->Text.Hover;

    renderer.Clear(' ',col->Text);
    int lm, tm, rm, bm;
    lm = tm = rm = bm = 0;
	if (Flags & (unsigned int)TextAreaFlags::BORDER)
	{
        renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, col->Border, false);
        lm = tm = rm = bm = 1;
        renderer.SetClipMargins(1, 1, 1, 1);
	}
    if (Flags & (unsigned int)TextAreaFlags::SHOW_LINE_NUMBERS)
    {
        unsigned int lnCount = Lines.Len();
        unsigned int tr = 0;
        unsigned int lnIndex = View.TopLine;
        while ((lnIndex < lnCount) && (tr < View.VisibleLinesCount))
        {
            if (lnIndex == View.CurrentLine)
                DrawLineNumber(renderer, lnIndex, tr, col->CurrentLineNumber);
            else
                DrawLineNumber(renderer, lnIndex, tr, col->LineNumbers);
            lnIndex++;
            tr++;
        }
        if (tr < View.VisibleLinesCount)
        {
            renderer.FillRectSize(0, tr, LINE_NUMBERS_WIDTH - 1, View.VisibleLinesCount - tr, ' ', col->LineNumbers);
        }
        renderer.DrawVerticalLineWithSpecialChar(View.TopLine + LINE_NUMBERS_WIDTH - 1, 0, View.VisibleRowsCount,SpecialChars::BoxVerticalSingleLine,col->Border);
        lm += LINE_NUMBERS_WIDTH;
    }
    renderer.SetClipMargins(lm, tm, rm, bm);
    for (unsigned int tr = 0; tr < View.VisibleLinesCount; tr++)
    {
        DrawLine(renderer, tr + View.TopLine, lm, tr+tm, col->Text);
    }
}
void TextAreaControlContext::MoveLeft(bool selected)
{
    CLEAR_SELECTION;

    if (View.CurrentPosition > 0)
    {
        View.CurrentPosition--;
        if (Text.GetBuffer()[View.CurrentPosition].Code == NEW_LINE_CODE)
        {
            // move to another line (next line)
            View.CurrentLine--;
            View.HorizontalOffset = 0;
        }
    }

    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveRight(bool selected)
{
    CLEAR_SELECTION;

    if (View.CurrentPosition < Text.Len())
    {
        bool isEOL = (Text.GetBuffer()[View.CurrentPosition].Code == NEW_LINE_CODE);
        View.CurrentPosition++;
        if (isEOL)
        {
            // move to another line (next line)
            View.CurrentLine++;
            View.HorizontalOffset = 0;            
        }
    }

    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveTo(int newPoz,bool selected)
{
	//if ((!selected) && (SelStart!=-1)) 
	//	ClearSel();
	//
	//if ((cLocation>=(int)Text.Len()) && (newPoz>cLocation)) 
	//	return;
	//if ((selected) && (SelStart==-1)) 
	//{ 
 //       SelStart = SelEnd = SelOrigin = cLocation;
	//}
	//while (cLocation!=newPoz)
	//{
	//	if (cLocation>newPoz) cLocation--; else if (cLocation<newPoz) cLocation++; 
	//	if (cLocation<0) { cLocation=newPoz=0; }
	//	if (cLocation >= (int)Text.Len())  newPoz=cLocation;
	//	if (cLocation<px) px=cLocation;
 //       if (cLocation > px + Layout.Width - 2) px = cLocation - Layout.Width + 2;
	//} 
	//if (selected) MoveSelTo(cLocation);
	//UpdateView();
}
void TextAreaControlContext::MoveToLine(int times,bool selected)
{
	//int xOffset,newLine,newLineSize;

	//if (Lines.Len()==0)
	//	return;

	//xOffset = cLocation - GetLineStart(cLine);
	//newLine = cLine+times;
	//if (newLine<0) 
	//	newLine=0;
	//if (newLine>=(int)Lines.Len())
	//	newLine=Lines.Len()-1;
	//newLineSize = GetLineSize(newLine);
	//if (newLineSize<xOffset)
	//	xOffset = newLineSize;
	//MoveTo(xOffset+GetLineStart(newLine),selected);
}
void TextAreaControlContext::MoveHome(bool selected)
{
	//MoveTo(GetLineStart(cLine),selected);
}
void TextAreaControlContext::MoveEnd(bool selected)
{
	//MoveTo(GetLineStart(cLine)+GetLineSize(cLine),selected);
}

void TextAreaControlContext::AddChar(char ch)
{
	if ((Flags & (unsigned int)TextAreaFlags::READONLY)!=0) 
		return;
	DeleteSelected();
	Text.InsertChar(ch, View.CurrentPosition);
	MoveTo(View.CurrentPosition +1,false);
	UpdateLines();
	SendMsg(Event::EVENT_TEXT_CHANGED);
}
void TextAreaControlContext::KeyBack()
{
	if ((Flags & (unsigned int)TextAreaFlags::READONLY) != 0)
		return;
    if (Selection.Start != INVALID_SELECTION)
    {
        DeleteSelected();
        return;
    }
    if (View.CurrentPosition == 0)
        return;
	Text.DeleteChar(View.CurrentPosition - 1);
    MoveTo(View.CurrentPosition - 1, false);
	UpdateLines();
	SendMsg(Event::EVENT_TEXT_CHANGED);
}
void TextAreaControlContext::KeyDelete()
{
	if ((Flags & (unsigned int)TextAreaFlags::READONLY) != 0)
		return;
    if (Selection.Start != INVALID_SELECTION)
    {
        DeleteSelected();
        return;
    }
	Text.DeleteChar(View.CurrentPosition);
	UpdateLines();
	SendMsg(Event::EVENT_TEXT_CHANGED);
}
bool TextAreaControlContext::HasSelection()
{
    return Selection.Start != INVALID_SELECTION;
}
void TextAreaControlContext::SetSelection(unsigned int start,unsigned int end)
{
	if ((start<end) && (end<=Text.Len()))
	{	
        Selection.Start = start;
        Selection.Origin = start;
        Selection.End = end;
	}
}
void TextAreaControlContext::CopyToClipboard()
{
	//if (HasSelection())  
	//	Clipboard::SetText(Text.GetText(),SelStart,SelEnd);
}
void TextAreaControlContext::PasteFromClipboard()
{
	if ((Flags & (unsigned int)TextAreaFlags::READONLY) != 0)
		return;
	//int tr;
	//const char *ss;
	//ss=Clipboard::GetText();
	//if (ss!=nullptr)
	//{
	//	if (HasSelection()) 
	//		DeleteSelected();
	//	for (tr=0;ss[tr]!=0;tr++) AddChar(ss[tr]);
	//}
	//UpdateLines();
}

bool TextAreaControlContext::OnKeyEvent(int KeyCode, char AsciiCode)
{
	switch (KeyCode)
	{
		case Key::Left							: MoveLeft(false); return true;
		case Key::Right						    : MoveRight(false); return true;
		//case Key::Up							: MoveToLine(-1,false); return true;
		//case Key::PageUp						: MoveToLine(-viewLines,false); return true;
		//case Key::Down							: MoveToLine(1,false); return true;
		//case Key::PageDown						: MoveToLine(viewLines,false); return true;
		//case Key::Home							: MoveHome(false); return true;
		//case Key::End							: MoveEnd(false); return true;
		//case Key::Ctrl|Key::Home				: MoveTo(0,false); return true;
		//case Key::Ctrl|Key::End				    : MoveTo(textSize,false); return true;

		case Key::Shift | Key::Left				: MoveLeft(true); return true;
		case Key::Shift | Key::Right			: MoveRight(true); return true;
		//case Key::Shift | Key::Up				: MoveToLine(-1, true); return true;
		//case Key::Shift | Key::PageUp			: MoveToLine(-viewLines, true); return true;
		//case Key::Shift | Key::Down			    : MoveToLine(1, true); return true;
		//case Key::Shift | Key::PageDown		    : MoveToLine(viewLines, true); return true;
		//case Key::Shift | Key::Home			    : MoveHome(true); return true;
		//case Key::Shift | Key::End			    : MoveEnd(true); return true;
		//case Key::Shift | Key::Ctrl | Key::Home : MoveTo(0, true); return true;
		//case Key::Shift | Key::Ctrl | Key::End  : MoveTo(textSize, true); return true;

		case Key::Tab							: if (Flags & (unsigned int)TextAreaFlags::PROCESS_TAB) { AddChar('\t'); return true; }
												  return false;
		case Key::Enter						    : AddChar('\n'); return true;
		case Key::Backspace					    : KeyBack(); return true;
		case Key::Delete						: KeyDelete(); return true;

		case Key::Ctrl | Key::A				    : SelAll(); return true;
		case Key::Ctrl | Key::Insert			:
		case Key::Ctrl | Key::C				    : CopyToClipboard(); return true;
		case Key::Shift| Key::Insert			:
		case Key::Ctrl | Key::V				    : PasteFromClipboard(); return true;
	}
	if (AsciiCode>0)
	{
		AddChar(AsciiCode);
		return true;
	}
	return false;
}

void TextAreaControlContext::OnAfterResize()
{
    ComputeVisibleLinesAndRows();
	UpdateLines();
}
void TextAreaControlContext::SetToolTip(char *ss)
{
	//toolTipInfo.Set(ss);
	//toolTipVisible=true;
}
bool TextAreaControlContext::IsReadOnly()
{
	return ((Flags & (unsigned int)TextAreaFlags::READONLY)!=0);
}
void TextAreaControlContext::SetReadOnly(bool value)
{
	if (value)
		this->Flags |= (unsigned int)TextAreaFlags::READONLY;
	else
		this->Flags -= ((this->Flags) & (unsigned int)TextAreaFlags::READONLY);
}
void TextAreaControlContext::SendMsg(Event::Type eventType)
{
	Host->RaiseEvent(eventType);
}

//======================================================================================================================================================================
TextArea::~TextArea()
{
	DELETE_CONTROL_CONTEXT(TextAreaControlContext);
}
bool		TextArea::Create(Control *parent, const char * text, const char * layout, TextAreaFlags flags)
{
	CHECK(text != nullptr, false, "Text should not be null !");
	CONTROL_INIT_CONTEXT(TextAreaControlContext);
    CREATE_TYPECONTROL_CONTEXT(TextAreaControlContext, Members, false);
    Members->Layout.MinWidth = 5;
    Members->Layout.MinHeight = 3;
    CHECK(Init(parent, "", layout, false), false, "Failed to create text area  !");
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int)flags;
	// initializam
    CHECK(Members->Text.SetWithNewLines(text), false, "Fail to set text to internal CharactersBuffers object !");
    CHECK(Members->Lines.Create(128), false, "Fail to create indexes for line numbers");
	Members->fnGetLineColor = nullptr;
	Members->tabChar = ' ';
	Members->Host = this;
    Members->View.CurrentPosition = 0;
    Members->View.TopLine = 0;
    Members->ComputeVisibleLinesAndRows();
    Members->ClearSel();    
	Members->AnalyzeCurrentText();
	// all is good
	return true;
}
void		TextArea::Paint(Console::Renderer & renderer)
{
	CREATE_TYPECONTROL_CONTEXT(TextAreaControlContext, Members, );
	Members->Paint(renderer);
}
bool		TextArea::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	return WRAPPER->OnKeyEvent(keyCode, AsciiCode);
}
void		TextArea::OnAfterResize(int newWidth,int newHeight)
{
	WRAPPER->OnAfterResize();
}
void		TextArea::OnFocus()
{
	WRAPPER->SelAll();
}
void		TextArea::OnAfterSetText(const char* newText)
{
	CREATE_TYPECONTROL_CONTEXT(TextAreaControlContext, Members, );
	Members->AnalyzeCurrentText();
}
void		TextArea::SetReadOnly(bool value)
{
	WRAPPER->SetReadOnly(value);
}
bool		TextArea::IsReadOnly()
{
	return WRAPPER->IsReadOnly();
}
void		TextArea::SetTabCharacter(char tabCharacter)
{
	WRAPPER->SetTabCharacter(tabCharacter);
}
void		TextArea::SetColorFunction(Handlers::TextAreaSyntaxHighlightHandler handler, void *Context)
{
	WRAPPER->SetColorFunction(handler, Context);
}

