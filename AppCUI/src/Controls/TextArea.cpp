#include "ControlContext.hpp"
#include "Internal.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;
using namespace AppCUI::OS;

#define LINE_NUMBERS_WIDTH 4
#define INVALID_SELECTION  0xFFFFFFFF

#define CLEAR_SELECTION                                                                                                \
    if ((!selected) && (Selection.Start != INVALID_SELECTION))                                                         \
    {                                                                                                                  \
        ClearSel();                                                                                                    \
    }                                                                                                                  \
    if ((selected) && (Selection.Start == INVALID_SELECTION))                                                          \
    {                                                                                                                  \
        Selection.Origin = View.CurrentPosition;                                                                       \
    }

#define UPDATE_SELECTION                                                                                               \
    if (selected)                                                                                                      \
        MoveSelectionTo(View.CurrentPosition);

#define WRAPPER ((TextAreaControlContext*) this->Context)

void TextAreaControlContext::ComputeVisibleLinesAndRows()
{
    unsigned int extraY = 0;
    unsigned int extraX = 1; // one character has to be left of the cursor (at the end of the line)

    if (Flags & (unsigned int) TextAreaFlags::ShowLineNumbers)
        extraX += LINE_NUMBERS_WIDTH;
    if (Flags & (unsigned int) TextAreaFlags::Border)
    {
        extraX += 2;
        extraY += 2;
    }
    if ((int) extraX < Layout.Width)
        this->View.VisibleRowsCount = ((unsigned int) Layout.Width) - extraX;
    else
        this->View.VisibleRowsCount = 0;
    if ((int) extraY < Layout.Height)
        this->View.VisibleLinesCount = ((unsigned int) Layout.Height) - extraY;
    else
        this->View.VisibleLinesCount = 0;
}
void TextAreaControlContext::UpdateViewXOffset()
{
    unsigned int startLine;
    // some sanity checks
    if (Lines.Get(View.CurrentLine, startLine) == false)
        return; // internal error
    if (startLine > View.CurrentPosition)
        return; // internal error
    if ((startLine + View.HorizontalOffset) > View.CurrentPosition)
    {
        View.HorizontalOffset = View.CurrentPosition - startLine;
        return;
    }
    if ((startLine + View.HorizontalOffset + View.VisibleRowsCount) < View.CurrentPosition)
    {
        View.HorizontalOffset = View.CurrentPosition - (View.VisibleRowsCount + startLine);
        return;
    }
}
void TextAreaControlContext::SelAll()
{
    if (Text.Len() == 0)
        ClearSel();
    else
    {
        Selection.Start  = 0;
        Selection.Origin = 0;
        Selection.End    = Text.Len();
    }
}
void TextAreaControlContext::ClearSel()
{
    Selection.Start  = INVALID_SELECTION;
    Selection.End    = INVALID_SELECTION;
    Selection.Origin = INVALID_SELECTION;
}
void TextAreaControlContext::MoveSelectionTo(unsigned int poz)
{
    if ((Selection.Origin == INVALID_SELECTION) || (poz == Selection.Origin))
    {
        ClearSel();
        return;
    }
    if (poz < Selection.Origin)
    {
        Selection.Start = poz;
        Selection.End   = Selection.Origin;
    }
    else
    {
        Selection.Start = Selection.Origin;
        Selection.End   = poz;
    }
}

void TextAreaControlContext::DeleteSelected()
{
    if (Selection.Start == INVALID_SELECTION)
        return;
    if (Text.Delete(Selection.Start, Selection.End))
    {
        View.CurrentPosition = Selection.Start;
        UpdateLines();
        ClearSel();
    }
}

void TextAreaControlContext::UpdateView()
{
    unsigned int start, end;
    unsigned int idxStart, idxEnd, idxMiddle;
    bool currentLineComputedCorectly = false;
    if (GetLineRange(View.CurrentLine, start, end))
    {
        currentLineComputedCorectly = ((View.CurrentPosition >= start) && (View.CurrentPosition < end));
    }
    if (!currentLineComputedCorectly)
    {
        unsigned int* pLines = Lines.GetUInt32Array();
        unsigned int cnt     = Lines.Len();

        // binary search
        idxStart = 0;
        idxEnd   = cnt - 1; // there is implicitely at least one line
        do
        {
            idxMiddle = ((idxStart + idxEnd) >> 1);
            start     = pLines[idxMiddle];
            if (idxMiddle + 1 < cnt)
                end = pLines[idxMiddle + 1];
            else
                end = Text.Len() + 1;
            if ((View.CurrentPosition >= start) && (View.CurrentPosition < end))
            {
                // found the line
                View.CurrentLine            = idxMiddle;
                currentLineComputedCorectly = true;
                break;
            }
            if (View.CurrentPosition < start)
            {
                idxEnd = idxMiddle - 1;
            }
            else
            {
                idxStart = idxMiddle + 1;
            }
        } while ((idxStart <= idxEnd) && (idxEnd < cnt) && (idxEnd != 0xFFFFFFFF));
    }
    if (!currentLineComputedCorectly)
    {
        // reset everything to the first line / first character
        // --- error case --- (this condition should not be match - this is but a sanity check ---
        View.CurrentLine      = 0;
        View.CurrentPosition  = 0;
        View.HorizontalOffset = 0;
        View.TopLine          = 0;
        return;
    }

    // validate ToLine
    if (View.CurrentLine < View.TopLine)
    {
        View.TopLine = View.CurrentLine;
    }
    if (View.VisibleLinesCount == 0)
    {
        View.TopLine = 0;
    }
    else
    {
        if (View.CurrentLine >= View.TopLine + View.VisibleLinesCount)
        {
            View.TopLine = View.CurrentLine - (View.VisibleLinesCount - 1);
        }
    }

    // finaly - update "X" offset
    UpdateViewXOffset();
}
void TextAreaControlContext::UpdateLines()
{
    unsigned int txSize, lineNumber;
    Character* c     = Text.GetBuffer();
    Character* s     = c;
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
            txSize = (unsigned int) (c - s);
            if (View.CurrentPosition >= txSize)
                View.CurrentLine = lineNumber;
            Lines.Push(txSize);
            lineNumber++;
        }
    } while (c < c_End);
    if (this->Flags & (unsigned int) TextAreaFlags::SyntaxHighlighting)
    {
        this->Syntax.Handler(this->Host, this->Text.GetBuffer(), this->Text.Len(), this->Syntax.Context);
    }
    UpdateView();
}
unsigned int TextAreaControlContext::GetLineStart(unsigned int lineIndex)
{
    unsigned int value;
    if (Lines.Get(lineIndex, value) == false)
        return 0;
    return value;
}
bool TextAreaControlContext::GetLineRange(unsigned int lineIndex, unsigned int& start, unsigned int& end)
{
    unsigned int linesCount = Lines.Len();
    unsigned int* p         = Lines.GetUInt32Array();
    CHECK(lineIndex < linesCount, false, "Invalid line index: %d (should be less than %d)", lineIndex, linesCount);
    p += lineIndex;
    start = *p;
    if (lineIndex + 1 < linesCount)
        end = p[1];
    else
        end = Text.Len() + 1;
    return true;
}
void TextAreaControlContext::AnalyzeCurrentText()
{
    UpdateLines();
    View.CurrentLine      = 0;
    View.CurrentPosition  = 0;
    View.HorizontalOffset = 0;
    //   cLocation = px = 0;
    // MoveTo(textSize,false);
}
void TextAreaControlContext::SetTabCharacter(char tabCharacter)
{
    tabChar = tabCharacter;
}

void TextAreaControlContext::DrawToolTip()
{
}
void TextAreaControlContext::DrawLine(
      Graphics::Renderer& renderer, unsigned int lineIndex, int ofsX, int pozY, const ColorPair textColor)
{
    unsigned int poz, lineStart, lineEnd, tr;
    int pozX, cursorPoz;
    bool useHighlighing;
    Character* ch;
    Character* ch_end;
    ColorPair col;

    if (GetLineRange(lineIndex, lineStart, lineEnd) == false)
        return;
    poz = lineStart + View.HorizontalOffset;
    if (poz >= lineEnd)
    {
        if (Flags & GATTR_ENABLE)
        {
            // if its the last character (EOF) --> show the cursor
            if (poz == View.CurrentPosition)
                renderer.SetCursor(ofsX - ((int) View.HorizontalOffset), pozY);
        }
        return;
    }
    ch_end         = Text.GetBuffer() + Text.Len();
    ch             = Text.GetBuffer() + poz;
    pozX           = ofsX;
    useHighlighing = (Flags & (unsigned int) TextAreaFlags::SyntaxHighlighting) != 0;
    cursorPoz      = -1;
    col            = textColor;
    if (pozX < 4)
    {
        cursorPoz = -1;
    }
    for (tr = 0; (ch < ch_end) && (tr <= /* show last char*/ View.VisibleRowsCount) && (ch->Code != NEW_LINE_CODE);
         poz++, tr++, ch++)
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

        if (ch->Code == '\t')
            renderer.WriteCharacter(pozX, pozY, this->tabChar, col);
        else
            renderer.WriteCharacter(pozX, pozY, ch->Code, col);

        if (ch->Code == '\t')
        {
            if ((pozX % 4) == 0)
                pozX += 4;
            else
                pozX = ((pozX >> 2) + 1) << 2;
        }
        else
        {
            pozX++;
        }
    }
    if (Flags & GATTR_ENABLE)
    {
        if (poz == View.CurrentPosition)
            cursorPoz = pozX;
        if (cursorPoz >= 0)
            renderer.SetCursor(cursorPoz, pozY);
    }
}
void TextAreaControlContext::DrawLineNumber(
      Graphics::Renderer& renderer, int lineIndex, int pozY, const ColorPair lineNumberColor)
{
    char temp[32];
    int poz  = 30;
    temp[31] = 0;
    lineIndex++;

    do
    {
        temp[poz--] = lineIndex % 10 + '0';
        lineIndex /= 10;
    } while (lineIndex > 0);
    while (poz > 27)
    {
        temp[poz--] = ' ';
    }
    if (poz < 27)
        temp[28] = '.';

    renderer.WriteSingleLineText(0, pozY, std::string_view(temp + 28, LINE_NUMBERS_WIDTH - 1), lineNumberColor);
}
void TextAreaControlContext::Paint(Graphics::Renderer& renderer)
{
    auto col = &this->Cfg->Text.Normal;
    if ((this->Flags & GATTR_ENABLE) == 0)
        col = &this->Cfg->Text.Inactive;
    else if (this->Focused)
        col = &this->Cfg->Text.Focus;
    else if (this->MouseIsOver)
        col = &this->Cfg->Text.Hover;

    renderer.Clear(' ', col->Text);
    int lm, tm, rm, bm;
    lm = tm = rm = bm = 0;
    if (Flags & (unsigned int) TextAreaFlags::Border)
    {
        renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, col->Border, false);
        lm = tm = rm = bm = 1;
        renderer.SetClipMargins(1, 1, 1, 1);
    }
    if (Flags & (unsigned int) TextAreaFlags::ShowLineNumbers)
    {
        unsigned int lnCount = Lines.Len();
        unsigned int tr      = 0;
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
        lm += LINE_NUMBERS_WIDTH;
        renderer.DrawVerticalLine(lm - 1, 0, View.VisibleRowsCount, col->Border);
    }
    renderer.SetClipMargins(lm, tm, rm, bm);
    for (unsigned int tr = 0; tr < View.VisibleLinesCount; tr++)
    {
        DrawLine(renderer, tr + View.TopLine, lm, tr + tm, col->Text);
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
            UpdateView();
        }
        UpdateViewXOffset();
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
            UpdateView();
        }
        UpdateViewXOffset();
    }

    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveTo(int, bool)
{
}
void TextAreaControlContext::MoveUpDown(unsigned int times, bool moveUp, bool selected)
{
    unsigned int rowNumber;
    unsigned int start, end;

    CLEAR_SELECTION;
    if (GetLineRange(View.CurrentLine, start, end))
        rowNumber = View.CurrentPosition - start;
    else
        rowNumber = 0;
    if (moveUp)
    {
        if (times > View.CurrentLine)
            View.CurrentLine = 0;
        else
            View.CurrentLine -= times;
    }
    else
    {
        View.CurrentLine += times;
        if (View.CurrentLine >= Lines.Len())
            View.CurrentLine = Lines.Len() - 1;
    }
    // compute CurrentPosition
    if (GetLineRange(View.CurrentLine, start, end))
    {
        if (start + rowNumber >= end)
            View.CurrentPosition = end - 1;
        else
            View.CurrentPosition = start + rowNumber;
    }
    else
    {
        // internal error (this code should not me match) --> reset the position to the start of the file
        View.HorizontalOffset = 0;
        View.CurrentPosition  = 0;
        View.CurrentLine      = 0;
        View.TopLine          = 0;
    }
    UpdateView();
    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveHome(bool selected)
{
    CLEAR_SELECTION;
    View.HorizontalOffset = 0;
    View.CurrentPosition  = GetLineStart(View.CurrentLine);
    UpdateViewXOffset();
    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveEnd(bool selected)
{
    CLEAR_SELECTION;
    unsigned int start, end;
    if (Text.Len() == 0)
    {
        View.CurrentPosition  = 0;
        View.CurrentLine      = 0;
        View.HorizontalOffset = 0;
        UpdateViewXOffset();
    }
    else
    {
        if (GetLineRange(View.CurrentLine, start, end))
        {
            View.CurrentPosition  = end - 1;
            View.HorizontalOffset = 0;
            UpdateViewXOffset();
        }
    }
    UPDATE_SELECTION;
}
bool __is_sign__(unsigned int, Character ch)
{
    switch(ch.Code)
    {
    case '!':
    case '@':
    case '#':
    case '$':
    case '%':
    case '^':
    case '&':
    case '*':
    case '(':
    case ')':
    case '_':
    case '-':
    case '+':
    case '=':
    case '`':
    case '~':
    case '[':
    case ']':
    case '{':
    case '}':
    case '\\':
    case '|':
    case ';':
    case ':':
    case '"':
    case '\'':
    case '<':
    case ',':
    case '>':
    case '.':
    case '?':
    case '/':
        return true;
    default:
        return false;
    }
}
bool __is_not_sign(unsigned int, Character ch)
{
    if ((ch == '\n') || (ch == '\r') || (ch==' ') || (ch=='\t'))
        return false;
    return !__is_sign__(0, ch);
}
void TextAreaControlContext::MoveToPreviousWord(bool selected)
{
    CLEAR_SELECTION;
    if ((this->Text.Len() == 0) || (View.CurrentPosition >= this->Text.Len()))
        return;
    unsigned int start, end;
    if (!GetLineRange(View.CurrentLine, start, end))
        return;
    if (View.CurrentPosition <= start)
        return;
    auto startPoz                   = View.CurrentPosition - 1;
    auto currentChar                = this->Text.GetBuffer()[startPoz];
    std::optional<unsigned int> res = std::nullopt;
    
    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = this->Text.FindPrevious(
              View.CurrentPosition-1, [](unsigned int, Character ch) { return (ch == ' ') || (ch == '\t'); });
        if (res.has_value())
            startPoz = res.value();
    }
    if (startPoz >= start)
    {
        currentChar = this->Text.GetBuffer()[startPoz];
        if (__is_sign__(0, currentChar))
        {
            res = this->Text.FindPrevious(startPoz, __is_sign__);
        }
        else
        {
            res = this->Text.FindPrevious(startPoz, __is_not_sign);
        }
    }

    // set new pos
    if (res.has_value())
    {
        View.CurrentPosition  = res.value() + 1;
        View.HorizontalOffset = 0;
        UpdateViewXOffset();
    }
    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveToNextWord(bool selected)
{
    CLEAR_SELECTION;
    if ((this->Text.Len() == 0) || (View.CurrentPosition >= this->Text.Len()))
        return;
    auto currentChar                = this->Text.GetBuffer()[View.CurrentPosition];
    std::optional<unsigned int> res = std::nullopt;
    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = this->Text.FindNext(
              View.CurrentPosition, [](unsigned int, Character ch) { return (ch == ' ') || (ch == '\t'); });
    }
    else if (__is_sign__(0, currentChar))
    {
        res = this->Text.FindNext(View.CurrentPosition, __is_sign__);
    }
    else
    {
        res = this->Text.FindNext(View.CurrentPosition, __is_not_sign);
    }
    // skip spaces if exists
    if (res.has_value())
        res = this->Text.FindNext(res.value(), [](unsigned int, Character ch) { return (ch == ' ') || (ch == '\t'); });
    // set new pos
    if (res.has_value())
    {
        View.CurrentPosition  = res.value();
        View.HorizontalOffset = 0;
        UpdateViewXOffset();
    }
    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveToStartOfTheFile(bool selected)
{
    CLEAR_SELECTION;
    View.HorizontalOffset = 0;
    View.CurrentPosition  = 0;
    View.CurrentLine      = 0;
    View.TopLine          = 0;
    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveToEndOfTheFile(bool selected)
{
    CLEAR_SELECTION;
    View.CurrentPosition = Text.Len();
    UpdateView();
    UPDATE_SELECTION;
}

void TextAreaControlContext::AddChar(char16_t ch)
{
    if ((Flags & (unsigned int) TextAreaFlags::Readonly) != 0)
        return;
    DeleteSelected();
    if (Text.InsertChar(ch, View.CurrentPosition))
    {
        View.CurrentPosition++;
        UpdateLines();
        SendMsg(Event::TextChanged);
    }
}
void TextAreaControlContext::KeyBack()
{
    if ((Flags & (unsigned int) TextAreaFlags::Readonly) != 0)
        return;
    if (Selection.Start != INVALID_SELECTION)
    {
        DeleteSelected();
        return;
    }
    if (View.CurrentPosition == 0)
        return;
    if (Text.DeleteChar(View.CurrentPosition - 1))
    {
        View.CurrentPosition--;
        UpdateLines();
        SendMsg(Event::TextChanged);
    }
}
void TextAreaControlContext::KeyDelete()
{
    if ((Flags & (unsigned int) TextAreaFlags::Readonly) != 0)
        return;
    if (Selection.Start != INVALID_SELECTION)
    {
        DeleteSelected();
        return;
    }
    if (Text.DeleteChar(View.CurrentPosition))
    {
        UpdateLines();
        SendMsg(Event::TextChanged);
    }
}
bool TextAreaControlContext::HasSelection()
{
    return Selection.Start != INVALID_SELECTION;
}
void TextAreaControlContext::SetSelection(unsigned int start, unsigned int end)
{
    if ((start < end) && (end <= Text.Len()))
    {
        Selection.Start  = start;
        Selection.Origin = start;
        Selection.End    = end;
    }
}
void TextAreaControlContext::CopyToClipboard()
{
    if (this->Selection.Start == INVALID_SELECTION)
        return;
    if (!AppCUI::OS::Clipboard::SetText(this->Text.SubString(this->Selection.Start, this->Selection.End)))
    {
        LOG_WARNING("Fail to copy string to the clipboard");
    }
}
void TextAreaControlContext::PasteFromClipboard()
{
    if ((Flags & (unsigned int) TextAreaFlags::Readonly) != 0)
        return;
    LocalUnicodeStringBuilder<2048> temp;
    if (Clipboard::GetText(temp) == false)
    {
        LOG_WARNING("Fail to retrive a text from the clipboard.");
        return;
    }
    DeleteSelected();
    if (Text.Insert(temp.ToStringView(), View.CurrentPosition))
    {
        View.CurrentPosition += temp.Len();
        UpdateLines();
        SendMsg(Event::TextChanged);
    }
}

bool TextAreaControlContext::OnKeyEvent(AppCUI::Input::Key KeyCode, char16_t UnicodeChar)
{
    switch (KeyCode)
    {
    case Key::Left:
        MoveLeft(false);
        return true;
    case Key::Right:
        MoveRight(false);
        return true;
    case Key::Up:
        MoveUpDown(1, true, false);
        return true;
    case Key::PageUp:
        MoveUpDown(View.VisibleLinesCount, true, false);
        return true;
    case Key::Down:
        MoveUpDown(1, false, false);
        return true;
    case Key::PageDown:
        MoveUpDown(View.VisibleLinesCount, false, false);
        return true;
    case Key::Home:
        MoveHome(false);
        return true;
    case Key::End:
        MoveEnd(false);
        return true;
    case Key::Ctrl | Key::Home:
        MoveToStartOfTheFile(false);
        return true;
    case Key::Ctrl | Key::End:
        MoveToEndOfTheFile(false);
        return true;
    case Key::Ctrl | Key::Left:
        MoveToPreviousWord(false);
        return true;
    case Key::Ctrl | Key::Right:
        MoveToNextWord(false);
        return true;

    case Key::Shift | Key::Left:
        MoveLeft(true);
        return true;
    case Key::Shift | Key::Right:
        MoveRight(true);
        return true;
    case Key::Shift | Key::Up:
        MoveUpDown(1, true, true);
        return true;
    case Key::Shift | Key::PageUp:
        MoveUpDown(View.VisibleLinesCount, true, true);
        return true;
    case Key::Shift | Key::Down:
        MoveUpDown(1, false, true);
        return true;
    case Key::Shift | Key::PageDown:
        MoveUpDown(View.VisibleLinesCount, false, true);
        return true;
    case Key::Shift | Key::Home:
        MoveHome(true);
        return true;
    case Key::Shift | Key::End:
        MoveEnd(true);
        return true;
    case Key::Shift | Key::Ctrl | Key::Home:
        MoveToStartOfTheFile(true);
        return true;
    case Key::Shift | Key::Ctrl | Key::End:
        MoveToEndOfTheFile(true);
        return true;
    case Key::Shift | Key::Ctrl | Key::Left:
        MoveToPreviousWord(true);
        return true;
    case Key::Shift | Key::Ctrl | Key::Right:
        MoveToNextWord(true);
        return true;

    case Key::Tab:
        if (Flags && TextAreaFlags::ProcessTabKey)
        {
            AddChar('\t');
            return true;
        }
        return false;
    case Key::Enter:
        AddChar(NEW_LINE_CODE);
        return true;
    case Key::Backspace:
        KeyBack();
        return true;
    case Key::Delete:
        KeyDelete();
        return true;

    case Key::Ctrl | Key::A:
        SelAll();
        return true;
    case Key::Ctrl | Key::Insert:
    case Key::Ctrl | Key::C:
        CopyToClipboard();
        return true;
    case Key::Shift | Key::Insert:
    case Key::Ctrl | Key::V:
        PasteFromClipboard();
        return true;
    }
    if (UnicodeChar > 0)
    {
        AddChar(UnicodeChar);
        return true;
    }
    return false;
}

void TextAreaControlContext::OnAfterResize()
{
    ComputeVisibleLinesAndRows();
    UpdateLines();
}
void TextAreaControlContext::SetToolTip(char*)
{
    // toolTipInfo.Set(ss);
    // toolTipVisible=true;
}
bool TextAreaControlContext::IsReadOnly()
{
    return ((Flags & (unsigned int) TextAreaFlags::Readonly) != 0);
}
void TextAreaControlContext::SetReadOnly(bool value)
{
    if (value)
        this->Flags |= (unsigned int) TextAreaFlags::Readonly;
    else
        this->Flags -= ((this->Flags) & (unsigned int) TextAreaFlags::Readonly);
}
void TextAreaControlContext::SendMsg(Event eventType)
{
    Host->RaiseEvent(eventType);
}

//======================================================================================================================================================================
TextArea::~TextArea()
{
    DELETE_CONTROL_CONTEXT(TextAreaControlContext);
}
TextArea::TextArea(
      const AppCUI::Utils::ConstString& caption,
      std::string_view layout,
      TextAreaFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
    : Control(new TextAreaControlContext(), "", layout, false)
{
    auto Members = reinterpret_cast<TextAreaControlContext*>(this->Context);

    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 3;
    Members->Syntax.Handler   = nullptr;
    Members->Syntax.Context   = nullptr;    
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int) flags;
    // initializam
    ASSERT(Members->Text.Set(caption), "Fail to set text to internal CharactersBuffers object !");
    ASSERT(Members->Lines.Create(128), "Fail to create indexes for line numbers");
    // scroll bars
    if ((unsigned int) flags & (unsigned int) TextAreaFlags::ScrollBars)
    {
        Members->Flags |= GATTR_VSCROLL;
        Members->ScrollBars.OutsideControl = (((unsigned int) flags & (unsigned int) TextAreaFlags::Border) == 0);
    }
    if (Members->Flags & (unsigned int) TextAreaFlags::SyntaxHighlighting)
    {
        Members->Syntax.Handler = handler;
        Members->Syntax.Context = handlerContext;
        ASSERT(handler, "if 'TextAreaFlags::SyntaxHighlighting` is set a syntaxt highligh handler must be provided");
    }
    Members->tabChar              = ' ';
    Members->View.CurrentPosition = 0;
    Members->View.TopLine         = 0;
    Members->Host                 = this;
    Members->ComputeVisibleLinesAndRows();
    Members->ClearSel();
    Members->AnalyzeCurrentText();
    // all is good

}


void TextArea::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(TextAreaControlContext, Members, );
    Members->Paint(renderer);
}
bool TextArea::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar)
{
    return WRAPPER->OnKeyEvent(keyCode, UnicodeChar);
}
void TextArea::OnAfterResize(int, int)
{
    WRAPPER->OnAfterResize();
}
void TextArea::OnUpdateScrollBars()
{
    CREATE_TYPECONTROL_CONTEXT(TextAreaControlContext, Members, );
    UpdateVScrollBar(Members->View.CurrentLine, Members->Lines.Len() - 1);
}
void TextArea::OnFocus()
{
    WRAPPER->SelAll();
}
void TextArea::OnAfterSetText(const AppCUI::Utils::ConstString&)
{
    CREATE_TYPECONTROL_CONTEXT(TextAreaControlContext, Members, );
    Members->AnalyzeCurrentText();
}
void TextArea::SetReadOnly(bool value)
{
    WRAPPER->SetReadOnly(value);
}
bool TextArea::IsReadOnly()
{
    return WRAPPER->IsReadOnly();
}
void TextArea::SetTabCharacter(char tabCharacter)
{
    WRAPPER->SetTabCharacter(tabCharacter);
}
