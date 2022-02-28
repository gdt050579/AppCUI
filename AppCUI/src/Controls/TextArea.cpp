#include "ControlContext.hpp"
#include "Internal.hpp"

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

namespace AppCUI
{
using namespace OS;
constexpr uint32 INVALID_SELECTION  = 0xFFFFFFFFU;
constexpr uint32 LINE_NUMBERS_WIDTH = 4U;
Internal::TextControlDefaultMenu* textAreaContexMenu = nullptr;

void Controls::UninitTextAreaDefaultMenu()
{
    if (textAreaContexMenu)
        delete textAreaContexMenu;
    textAreaContexMenu = nullptr;
}
void TextAreaControlContext::ComputeVisibleLinesAndRows()
{
    uint32 extraY = 0;
    uint32 extraX = 1; // one character has to be left of the cursor (at the end of the line)

    if (Flags & (uint32) TextAreaFlags::ShowLineNumbers)
        extraX += LINE_NUMBERS_WIDTH;
    if (Flags & (uint32) TextAreaFlags::Border)
    {
        extraX += 2;
        extraY += 2;
    }
    if ((int) extraX < Layout.Width)
        this->View.VisibleRowsCount = ((uint32) Layout.Width) - extraX;
    else
        this->View.VisibleRowsCount = 0;
    if ((int) extraY < Layout.Height)
        this->View.VisibleLinesCount = ((uint32) Layout.Height) - extraY;
    else
        this->View.VisibleLinesCount = 0;
}
void TextAreaControlContext::UpdateViewXOffset()
{
    uint32 startLine;
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
void TextAreaControlContext::MoveSelectionTo(uint32 poz)
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
    uint32 start, end;
    uint32 idxStart, idxEnd, idxMiddle;
    bool currentLineComputedCorectly = false;
    if (GetLineRange(View.CurrentLine, start, end))
    {
        currentLineComputedCorectly = ((View.CurrentPosition >= start) && (View.CurrentPosition < end));
    }
    if (!currentLineComputedCorectly)
    {
        uint32* pLines = Lines.GetUInt32Array();
        uint32 cnt     = Lines.Len();

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
    uint32 txSize, lineNumber;
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
            txSize = (uint32) (c - s);
            if (View.CurrentPosition >= txSize)
                View.CurrentLine = lineNumber;
            Lines.Push(txSize);
            lineNumber++;
        }
    } while (c < c_End);
    if (this->Flags & (uint32) TextAreaFlags::SyntaxHighlighting)
    {
        if (this->handlers != nullptr)
        {
            auto t_h = (Controls::Handlers::TextControl*) this->handlers.get();
            if (t_h->OnTextColor.obj)
            {
                t_h->OnTextColor.obj->OnTextColor(this->Host, this->Text.GetBuffer(), this->Text.Len());
            }
        }
    }
    UpdateView();
}
uint32 TextAreaControlContext::GetLineStart(uint32 lineIndex)
{
    uint32 value;
    if (Lines.Get(lineIndex, value) == false)
        return 0;
    return value;
}
bool TextAreaControlContext::GetLineRange(uint32 lineIndex, uint32& start, uint32& end)
{
    uint32 linesCount = Lines.Len();
    uint32* p         = Lines.GetUInt32Array();
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
      Graphics::Renderer& renderer, uint32 lineIndex, int ofsX, int pozY, const ColorPair textColor)
{
    uint32 poz, lineStart, lineEnd, tr;
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
        if (Focused)
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
    useHighlighing = (Flags & (uint32) TextAreaFlags::SyntaxHighlighting) != 0;
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
            if ((Focused) && (poz >= Selection.Start) && (poz < Selection.End))
                col = Cfg->Selection.Editor;
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
    if (Focused)
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

    renderer.WriteSingleLineText(0, pozY, string_view(temp + 28, LINE_NUMBERS_WIDTH - 1), lineNumberColor);
}
void TextAreaControlContext::Paint(Graphics::Renderer& renderer)
{
    const auto state  = this->GetControlState(ControlStateFlags::ProcessHoverStatus);
    const auto colTxt = Cfg->Editor.GetColor(state);
    const auto colB   = Cfg->Border.GetColor(state);
    const auto colLn  = Cfg->LineMarker.GetColor(state);

    renderer.Clear(' ', colTxt);
    int lm, tm, rm, bm;
    lm = tm = rm = bm = 0;
    if (Flags & (uint32) TextAreaFlags::Border)
    {
        renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, colB, LineType::Single);
        lm = tm = rm = bm = 1;
        renderer.SetClipMargins(1, 1, 1, 1);
    }
    if (Flags & (uint32) TextAreaFlags::ShowLineNumbers)
    {
        uint32 lnCount = Lines.Len();
        uint32 tr      = 0;
        uint32 lnIndex = View.TopLine;
        while ((lnIndex < lnCount) && (tr < View.VisibleLinesCount))
        {
            if (lnIndex == View.CurrentLine)
                DrawLineNumber(renderer, lnIndex, tr, Cfg->Selection.Editor);
            else
                DrawLineNumber(renderer, lnIndex, tr, colLn);
            lnIndex++;
            tr++;
        }
        if (tr < View.VisibleLinesCount)
        {
            renderer.FillRectSize(0, tr, LINE_NUMBERS_WIDTH - 1, View.VisibleLinesCount - tr, ' ', colLn);
        }
        lm += LINE_NUMBERS_WIDTH;
        renderer.DrawVerticalLine(lm - 1, 0, View.VisibleRowsCount, colB);
    }
    renderer.SetClipMargins(lm, tm, rm, bm);
    for (uint32 tr = 0; tr < View.VisibleLinesCount; tr++)
    {
        DrawLine(renderer, tr + View.TopLine, lm, tr + tm, colTxt);
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
void TextAreaControlContext::MoveTo(uint32 lineIndex, uint32 newPos, bool selected)
{
    CLEAR_SELECTION;
    if (lineIndex < View.CurrentLine)
        MoveUpDown(View.CurrentLine - lineIndex, false, selected);
    else if (lineIndex > View.CurrentLine)
        MoveUpDown(lineIndex - View.CurrentLine, true, selected);
    while (newPos != View.CurrentPosition)
    {
        auto cPoz = View.CurrentPosition;
        if (newPos < View.CurrentPosition)
            MoveLeft(selected);
        else
            MoveRight(selected);
        // sanity check to avoid infitine loops
        if (cPoz == View.CurrentPosition)
            break;
    }
    UPDATE_SELECTION;
}
void TextAreaControlContext::MoveUpDown(uint32 times, bool moveUp, bool selected)
{
    uint32 rowNumber;
    uint32 start, end;

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
    uint32 start, end;
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
bool __is_sign__(uint32, Character ch)
{
    switch (ch.Code)
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
bool __is_not_sign(uint32, Character ch)
{
    if ((ch == '\n') || (ch == '\r') || (ch == ' ') || (ch == '\t'))
        return false;
    return !__is_sign__(0, ch);
}
void TextAreaControlContext::MoveToPreviousWord(bool selected)
{
    CLEAR_SELECTION;
    if ((this->Text.Len() == 0) || (View.CurrentPosition >= this->Text.Len()))
        return;
    uint32 start, end;
    if (!GetLineRange(View.CurrentLine, start, end))
        return;
    if (View.CurrentPosition <= start)
        return;
    auto startPoz        = View.CurrentPosition - 1;
    auto currentChar     = this->Text.GetBuffer()[startPoz];
    optional<uint32> res = std::nullopt;

    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = this->Text.FindPrevious(
              View.CurrentPosition - 1, [](uint32, Character ch) { return (ch == ' ') || (ch == '\t'); });
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
    auto currentChar     = this->Text.GetBuffer()[View.CurrentPosition];
    optional<uint32> res = std::nullopt;
    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = this->Text.FindNext(
              View.CurrentPosition, [](uint32, Character ch) { return (ch == ' ') || (ch == '\t'); });
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
        res = this->Text.FindNext(res.value(), [](uint32, Character ch) { return (ch == ' ') || (ch == '\t'); });
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

void TextAreaControlContext::AddChar(char16 ch)
{
    if ((Flags & (uint32) TextAreaFlags::Readonly) != 0)
        return;
    if (this->handlers)
    {
        auto h = (Handlers::TextControl*) (this->handlers.get());
        if ((h->OnValidateCharacter.obj) && (h->OnValidateCharacter.obj->OnValidateCharacter(this->Host, ch) == false))
            return;
    }
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
    if ((Flags & (uint32) TextAreaFlags::Readonly) != 0)
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
    if ((Flags & (uint32) TextAreaFlags::Readonly) != 0)
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
void TextAreaControlContext::SetSelection(uint32 start, uint32 end)
{
    if ((start < end) && (end <= Text.Len()))
    {
        Selection.Start  = start;
        Selection.Origin = start;
        Selection.End    = end;
    }
}
void TextAreaControlContext::ToUpper()
{
    if (Selection.Start == INVALID_SELECTION)
        return;
    this->Text.ConvertToUpper(Selection.Start, Selection.End + 1);
}
void TextAreaControlContext::ToLower()
{
    if (Selection.Start == INVALID_SELECTION)
        return;
    this->Text.ConvertToLower(Selection.Start, Selection.End + 1);
}
void TextAreaControlContext::CopyToClipboard()
{
    if (this->Selection.Start == INVALID_SELECTION)
        return;
    if (!OS::Clipboard::SetText(this->Text.SubString(this->Selection.Start, this->Selection.End)))
    {
        LOG_WARNING("Fail to copy string to the clipboard");
    }
}
void TextAreaControlContext::PasteFromClipboard()
{
    if ((Flags & (uint32) TextAreaFlags::Readonly) != 0)
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

bool TextAreaControlContext::OnKeyEvent(Input::Key KeyCode, char16 UnicodeChar)
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

    case Key::Ctrl | Key::Shift | Key::U:
        ToUpper();
        return true;
    case Key::Ctrl | Key::U:
        ToLower();
        return true;
    }
    if (UnicodeChar > 0)
    {
        AddChar(UnicodeChar);
        return true;
    }
    return false;
}

void TextAreaControlContext::MousePosToFilePos(int x, int y, uint32& lineIndex, uint32& offset)
{
    lineIndex = (y + (int32) View.TopLine) >= 0 ? (uint32) (y + (int32) View.TopLine) : 0U;

    if (this->Lines.Len() == 0)
    {
        lineIndex = 0;
        offset    = 0;
        return;
    }
    if (lineIndex >= Lines.Len())
    {
        // move to the end of text
        lineIndex = Lines.Len() - 1;
        offset    = Text.Len();
        return;
    }

    uint32 start, end;
    if (GetLineRange(lineIndex, start, end))
    {
        offset = (x + (int) View.HorizontalOffset - (int) LINE_NUMBERS_WIDTH) >= 0
                       ? ((uint32) (x + (int) View.HorizontalOffset - (int) LINE_NUMBERS_WIDTH)) + start
                       : start;
        offset = std::min<>(offset, end);
        return;
    }
    else
    {
        // move to the end of text (however, this code should not be reached).
        lineIndex = Lines.Len() - 1;
        offset    = Text.Len();
        return;
    }
}
void TextAreaControlContext::OnMouseReleased(int x, int y, Input::MouseButton button)
{
}
void TextAreaControlContext::OnMousePressed(int x, int y, Input::MouseButton button)
{
    uint32 lineIndex, ofs;
    if (button == MouseButton::Left)
    {
        MousePosToFilePos(x, y, lineIndex, ofs);
        MoveTo(lineIndex, ofs, false);
    }
    if (button == (MouseButton::DoubleClicked | MouseButton::Left))
    {
        MousePosToFilePos(x, y, lineIndex, ofs);
        MoveTo(lineIndex, ofs, false);
        MoveToPreviousWord(false);
        MoveToNextWord(true);
    }
    if (button == MouseButton::Right)
    {
        if ((this->handlers) &&
            ((reinterpret_cast<Handlers::TextControl*>(this->handlers.get()))->OnTextRightClick.obj))
        {
            (reinterpret_cast<Handlers::TextControl*>(this->handlers.get()))
                  ->OnTextRightClick.obj->OnTextRightClick(this->Host, x, y);
        }
        else
        {
            if (textAreaContexMenu == nullptr)
            {
                textAreaContexMenu = new Internal::TextControlDefaultMenu();
            }
            textAreaContexMenu->Show(this->Host, x, y + 1, this->Selection.Start >= 0);
        }
    }
}
bool TextAreaControlContext::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    uint32 lineIndex, ofs;
    if (button == MouseButton::Left)
    {
        MousePosToFilePos(x, y, lineIndex, ofs);
        MoveTo(lineIndex, ofs, true);
    }
    return true;
}
bool TextAreaControlContext::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    switch (direction)
    {
    case MouseWheel::Up:
        MoveUpDown(1, true, false);
        return true;
    case MouseWheel::Down:
        MoveUpDown(1, false, false);
        return true;
    }
    return false;
}
bool TextAreaControlContext::OnMouseOver(int x, int y)
{
    NOT_IMPLEMENTED(false);
}
bool TextAreaControlContext::OnMouseLeave()
{
    NOT_IMPLEMENTED(false);
}
bool TextAreaControlContext::OnMouseEnter()
{
    NOT_IMPLEMENTED(false);
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
    return ((Flags & (uint32) TextAreaFlags::Readonly) != 0);
}
void TextAreaControlContext::SetReadOnly(bool value)
{
    if (value)
        this->Flags |= (uint32) TextAreaFlags::Readonly;
    else
        this->Flags -= ((this->Flags) & (uint32) TextAreaFlags::Readonly);
}
void TextAreaControlContext::SendMsg(Event eventType)
{
    Host->RaiseEvent(eventType);
}
bool TextAreaControlContext::OnEvent(Event eventType, int ID)
{
    if (eventType == Event::Command)
    {
        switch (ID)
        {
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_COPY:
            this->CopyToClipboard();
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_CUT:
            if (Selection.Start != INVALID_SELECTION)
            {
                this->CopyToClipboard();
                OnKeyEvent(Key::Delete, 0);
            }
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_PASTE:
            this->PasteFromClipboard();
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_SELECT_ALL:
            this->SetSelection(0, this->Text.Len());
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_DELETE_SELECTED:
            OnKeyEvent(Key::Delete, 0);
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_TO_UPPER:
            this->ToUpper();
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_TO_LOWER:
            this->ToLower();
            return true;
        }
    }
    return false;
}
//======================================================================================================================================================================
TextArea::~TextArea()
{
    DELETE_CONTROL_CONTEXT(TextAreaControlContext);
}
TextArea::TextArea(const ConstString& caption, string_view layout, TextAreaFlags flags)
    : Control(new TextAreaControlContext(), "", layout, false)
{
    auto Members = reinterpret_cast<TextAreaControlContext*>(this->Context);

    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 3;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) flags;
    // initializam
    ASSERT(Members->Text.Set(caption), "Fail to set text to internal CharactersBuffers object !");
    ASSERT(Members->Lines.Create(128), "Fail to create indexes for line numbers");
    // scroll bars
    if ((uint32) flags & (uint32) TextAreaFlags::ScrollBars)
    {
        Members->Flags |= GATTR_VSCROLL;
        Members->ScrollBars.OutsideControl = (((uint32) flags & (uint32) TextAreaFlags::Border) == 0);
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
bool TextArea::OnEvent(Reference<Control> /*sender*/, Event eventType, int ID)
{
    return WRAPPER->OnEvent(eventType,ID);
}
bool TextArea::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    return WRAPPER->OnKeyEvent(keyCode, UnicodeChar);
}
void TextArea::OnMousePressed(int x, int y, Input::MouseButton button)
{
    WRAPPER->OnMousePressed(x, y, button);
}
void TextArea::OnMouseReleased(int x, int y, Input::MouseButton button)
{
    WRAPPER->OnMouseReleased(x, y, button);
}
bool TextArea::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    return WRAPPER->OnMouseDrag(x, y, button);
}
bool TextArea::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    return WRAPPER->OnMouseWheel(x, y, direction);
}
bool TextArea::OnMouseEnter()
{
    return WRAPPER->OnMouseEnter();
}
bool TextArea::OnMouseLeave()
{
    return WRAPPER->OnMouseLeave();
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
void TextArea::OnAfterSetText()
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
Handlers::TextControl* TextArea::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::TextControl);
}
} // namespace AppCUI