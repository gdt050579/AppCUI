#include "ControlContext.hpp"

#define C_WIDTH ((Members->Layout.Width - 2) * Members->Layout.Height)
#define EXIT_IF_READONLY()                                                                                             \
    if ((Members->Flags & TextFieldFlags::Readonly) != TextFieldFlags::None)                                           \
    {                                                                                                                  \
        return;                                                                                                        \
    };

namespace AppCUI::Controls
{
Internal::TextControlDefaultMenu* textFieldContexMenu = nullptr;

void UninitTextFieldDefaultMenu()
{
    if (textFieldContexMenu)
        delete textFieldContexMenu;
    textFieldContexMenu = nullptr;
}

void TextField_SendTextChangedEvent(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if ((Members->Flags & TextFieldFlags::SyntaxHighlighting) != TextFieldFlags::None)
    {
        if (Members->handlers != nullptr)
        {
            auto t_h = (Controls::Handlers::TextControl*) Members->handlers.get();
            if (t_h->OnTextColor.obj)
            {
                t_h->OnTextColor.obj->OnTextColor(control, Members->Text.GetBuffer(), Members->Text.Len());
            }
        }
    }
    control->RaiseEvent(Event::TextChanged);
}

void TextField_MoveSelTo(TextField* control, int poz)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    Members->FullSelectionDueToOnFocusEvent = false;
    if (Members->Selection.Start == -1)
        return;
    if (poz == Members->Selection.Origin)
    {
        control->ClearSelection();
        Members->Modified = true;
        return;
    }
    if (poz < Members->Selection.Origin)
    {
        Members->Selection.Start = poz;
        Members->Selection.End   = Members->Selection.Origin - 1;
    }
    else if (poz > Members->Selection.Origin)
    {
        Members->Selection.End   = poz - 1;
        Members->Selection.Start = Members->Selection.Origin;
    }
    Members->Modified = true;
    // if (poz<SelStart) SelStart=poz; else SelEnd=poz-1;
}
void TextField_MoveTo(TextField* control, int newPoz, bool selected)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    int c_width                             = C_WIDTH;
    Members->FullSelectionDueToOnFocusEvent = false;
    if ((!selected) && (Members->Selection.Start != -1))
        control->ClearSelection();
    if ((static_cast<unsigned>(Members->Cursor.Pos) == Members->Text.Len()) && (newPoz > Members->Cursor.Pos))
        return;

    if ((selected) && (Members->Selection.Start == -1))
    {
        Members->Selection.Start = Members->Selection.End = Members->Selection.Origin = Members->Cursor.Pos;
    }
    while (Members->Cursor.Pos != newPoz)
    {
        if (Members->Cursor.Pos > newPoz)
            Members->Cursor.Pos--;
        else if (Members->Cursor.Pos < newPoz)
            Members->Cursor.Pos++;
        if (Members->Cursor.Pos < 0)
        {
            Members->Cursor.Pos = newPoz = 0;
        }
        if (static_cast<unsigned>(Members->Cursor.Pos) == Members->Text.Len())
            newPoz = Members->Cursor.Pos;
        if (Members->Cursor.Pos < Members->Cursor.StartOffset)
            Members->Cursor.StartOffset = Members->Cursor.Pos;
        if (Members->Cursor.Pos > Members->Cursor.StartOffset + c_width)
            Members->Cursor.StartOffset = Members->Cursor.Pos - c_width;
    }
    if (selected)
        TextField_MoveSelTo(control, Members->Cursor.Pos);
}
bool __is_op__(uint32, Character ch)
{
    if ((ch.Code < 33) || (ch.Code >= 127))
        return false;
    if ((ch.Code >= '0') && (ch.Code <= '9'))
        return false;
    if ((ch.Code >= 'A') && (ch.Code <= 'Z'))
        return false;
    if ((ch.Code >= 'a') && (ch.Code <= 'z'))
        return false;
    return true;
}
bool __is_not_op__(uint32, Character ch)
{
    if ((ch == '\n') || (ch == '\r') || (ch == ' ') || (ch == '\t'))
        return false;
    return !__is_op__(0, ch);
}
void TextField_MoveToNextWord(TextField* control, bool selected, bool skipSpacesAfterWord = true)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Cursor.Pos >= (int) Members->Text.Len())
        return;
    auto currentChar     = Members->Text.GetBuffer()[Members->Cursor.Pos];
    optional<uint32> res = std::nullopt;

    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = Members->Text.FindNext(
              Members->Cursor.Pos, [](uint32, Character ch) { return (ch == ' ') || (ch == '\t'); });
    }
    else if (__is_op__(0, currentChar))
    {
        res = Members->Text.FindNext(Members->Cursor.Pos, __is_op__);
    }
    else
    {
        res = Members->Text.FindNext(Members->Cursor.Pos, __is_not_op__);
    }
    // skip spaces if exists
    if (skipSpacesAfterWord)
    {
        if (res.has_value())
            res = Members->Text.FindNext(res.value(), [](uint32, Character ch) { return (ch == ' ') || (ch == '\t'); });
    }
    if (res.has_value())
        TextField_MoveTo(control, res.value(), selected);
}
void TextField_MoveToPreviousWord(TextField* control, bool selected)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Cursor.Pos == 0)
        return;
    auto startPoz        = Members->Cursor.Pos - 1;
    auto currentChar     = Members->Text.GetBuffer()[startPoz];
    optional<uint32> res = std::nullopt;

    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = Members->Text.FindPrevious(startPoz, [](uint32, Character ch) { return (ch == ' ') || (ch == '\t'); });
        if (res.has_value())
            startPoz = res.value();
    }

    currentChar = Members->Text.GetBuffer()[startPoz];
    if (__is_op__(0, currentChar))
        res = Members->Text.FindPrevious(startPoz, __is_op__);
    else
        res = Members->Text.FindPrevious(startPoz, __is_not_op__);

    // set new pos
    if (res.has_value())
    {
        if (res.value() > 0)
            TextField_MoveTo(control, res.value() + 1, selected);
        else
            TextField_MoveTo(control, 0, selected);
    }
}
void TextField_DeleteSelected(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    int ss, se;
    if (Members->Selection.Start == -1)
        return;
    ss = Members->Selection.Start;
    se = Members->Selection.End;
    if (Members->Selection.Start < Members->Selection.Origin)
    {
        Members->Selection.Start++;
    }
    if (Members->Cursor.Pos >= Members->Selection.Start)
    {
        TextField_MoveTo(control, Members->Cursor.Pos - (se - ss + 1), false);
    }
    Members->Text.Delete(ss, se + 1);
    control->ClearSelection();
    Members->Modified = true;
}
void TextField_AddChar(TextField* control, char16 ch)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    if (Members->handlers)
    {
        auto h = (Handlers::TextControl*) (Members->handlers.get());
        if ((h->OnValidateCharacter.obj) && (h->OnValidateCharacter.obj->OnValidateCharacter(control, ch) == false))
            return;
    }
    TextField_DeleteSelected(control);
    if (Members->Cursor.Pos > (int) Members->Text.Len())
        Members->Text.InsertChar(ch, Members->Text.Len());
    else
        Members->Text.InsertChar(ch, (uint32) (Members->Cursor.Pos));
    TextField_MoveTo(control, Members->Cursor.Pos + 1, false);
    TextField_SendTextChangedEvent(control);
    Members->Modified = true;
}
void TextField_KeyBack(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    if (Members->Selection.Start != -1)
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
void TextField_KeyDelete(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    if (Members->Selection.Start != -1)
    {
        TextField_DeleteSelected(control);
        Members->Modified = true;
        return;
    }
    Members->Text.DeleteChar(Members->Cursor.Pos);
    TextField_SendTextChangedEvent(control);
    Members->Modified = true;
}
bool TextField_HasSelection(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, false);
    return (
          (Members->Selection.Start >= 0) && (Members->Selection.End >= 0) &&
          (Members->Selection.End >= Members->Selection.Start));
}
void TextField_SetSelection(TextField* control, int start, int end)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if ((start >= 0) && (start <= end))
    {
        Members->Selection.Start = Members->Selection.Origin = start;
        Members->Selection.End                               = end;
        Members->Modified                                    = true;
    }
}
void TextField_CopyToClipboard(TextField* control, bool deleteSelectionAfterCopy)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );

    if (!TextField_HasSelection(control))
        return;
    if (!OS::Clipboard::SetText(Members->Text.SubString(Members->Selection.Start, (size_t) Members->Selection.End + 1)))
    {
        LOG_WARNING("Fail to copy string to the clipboard");
    }
    if (deleteSelectionAfterCopy)
    {
        TextField_KeyDelete(control);
    }
}
void TextField_PasteFromClipboard(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    LocalUnicodeStringBuilder<2048> temp;
    if (OS::Clipboard::GetText(temp) == false)
    {
        LOG_WARNING("Fail to retrive a text from the clipboard.");
        return;
    }
    TextField_DeleteSelected(control);
    if (Members->Text.Insert(temp.ToStringView(), Members->Cursor.Pos))
    {
        TextField_MoveTo(control, Members->Cursor.Pos + temp.Len(), false);
    }
    TextField_SendTextChangedEvent(control);
    Members->Modified = true;
}
int TextField_MouseToTextPos(TextField* control, int x, int y)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, -1);
    // for sizes too small --> it can not be computed (sanity check)
    if (Members->Layout.Width <= 2)
        return -1;
    int poz = y * (Members->Layout.Width - 2) + (x - 1); // -1 as the first character is space
    if (poz >= (int) Members->Text.Len())
        poz = (int) Members->Text.Len();
    if (poz < 0)
        poz = 0;
    return poz;
}
void TextField_SelectWorld(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    control->ClearSelection();
    TextField_MoveToPreviousWord(control, false);
    TextField_MoveToNextWord(control, true, false);
}
void TextField_ToUpper(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Selection.Start < 0)
        return;
    Members->Text.ConvertToUpper((uint32) Members->Selection.Start, (uint32) (Members->Selection.End + 1));
}
void TextField_ToLower(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Selection.Start < 0)
        return;
    Members->Text.ConvertToLower((uint32) Members->Selection.Start, (uint32) (Members->Selection.End + 1));
}
//============================================================================
TextField::~TextField()
{
    DELETE_CONTROL_CONTEXT(TextFieldControlContext);
}
TextField::TextField(const ConstString& caption, string_view layout, TextFieldFlags flags)
    : Control(new TextFieldControlContext(), caption, layout, false)
{
    auto Members                            = reinterpret_cast<TextFieldControlContext*>(this->Context);
    Members->Layout.MinWidth                = 3;
    Members->Layout.MinHeight               = 1;
    Members->Flags                          = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) flags;
    Members->Modified                       = true;
    Members->FullSelectionDueToOnFocusEvent = false;

    this->ClearSelection();
    Members->Cursor.Pos = Members->Cursor.StartOffset = 0;
    TextField_MoveTo(this, 0xFFFF, false);
}

void TextField::SelectAll()
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    Members->Selection.Start = 0;
    Members->Selection.End   = Members->Text.Len() - 1;
    if (Members->Selection.End < 0)
        ClearSelection();
    Members->Modified                       = true;
    Members->FullSelectionDueToOnFocusEvent = false;
}
void TextField::CopyToClipboard(bool deleteSelectionAfterCopy)
{
    TextField_CopyToClipboard(this, deleteSelectionAfterCopy);
}
void TextField::PasteFromClipboard()
{
    TextField_PasteFromClipboard(this);
}
void TextField::ClearSelection()
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    Members->Selection.Start = Members->Selection.End = Members->Selection.Origin = -1;
    Members->Modified                                                             = true;
    Members->FullSelectionDueToOnFocusEvent                                       = false;
}
bool TextField::HasSelection() const
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, false);
    return Members->Selection.Start >= 0;
}
bool TextField::GetSelection(uint32& start, uint32& size) const
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, false);
    if ((Members->Selection.Start < 0) || (Members->Selection.End <= Members->Selection.Start))
        return false;
    start = (uint32) Members->Selection.Start;
    size  = (uint32) (Members->Selection.End - Members->Selection.Start);
    return true;
}
bool TextField::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
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
    case Key::Ctrl | Key::Left:
        TextField_MoveToPreviousWord(this, false);
        return true;
    case Key::Ctrl | Key::Right:
        TextField_MoveToNextWord(this, false);
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
    case Key::Shift | Key::Ctrl | Key::Left:
        TextField_MoveToPreviousWord(this, true);
        return true;
    case Key::Shift | Key::Ctrl | Key::Right:
        TextField_MoveToNextWord(this, true);
        return true;

    case Key::Backspace:
        TextField_KeyBack(this);
        return true;
    case Key::Delete:
        TextField_KeyDelete(this);
        return true;

    case Key::Ctrl | Key::A:
        SelectAll();
        return true;

    case Key::Ctrl | Key::Shift | Key::U:
        TextField_ToUpper(this);
        return true;
    case Key::Ctrl | Key::U:
        TextField_ToLower(this);
        return true;

    case Key::Ctrl | Key::Insert:
    case Key::Ctrl | Key::C:
        TextField_CopyToClipboard(this, false);
        return true;
    case Key::Ctrl | Key::V:
    case Key::Shift | Key::Insert:
        TextField_PasteFromClipboard(this);
        return true;
    case Key::Ctrl | Key::X:
        TextField_CopyToClipboard(this, true);
        return true;

    case Key::Enter:
        if ((Members->Flags & TextFieldFlags::ProcessEnter) != TextFieldFlags::None)
        {
            RaiseEvent(Event::TextFieldValidate);
            return true;
        }
        return false;
    }

    if (UnicodeChar != 0)
    {
        TextField_AddChar(this, UnicodeChar);
        return true;
    }

    return false;
}
void TextField::OnAfterSetText()
{
    // reposition cursor at the end of the newly set text
    TextField_MoveTo(this, 0, false);
    TextField_MoveTo(this, 0xFFFF, false);
}

void TextField::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );

    WriteTextParams params(WriteTextFlags::WrapToWidth | WriteTextFlags::ClipToWidth, TextAlignament::Left);
    params.Width = Members->Layout.Width - 2;
    params.X     = 1;
    params.Y     = 0;
    params.Color = Members->Cfg->Editor.GetColor(Members->GetControlState(ControlStateFlags::ProcessHoverStatus));

    if (Members->Layout.Height == 1)
        params.Flags |= WriteTextFlags::SingleLine;
    else
        params.Flags |= WriteTextFlags::MultipleLines;

    renderer.Clear(' ', params.Color);

    if (Members->Focused)
    {
        if (Members->Modified)
        {
            if ((Members->Flags & TextFieldFlags::SyntaxHighlighting) != TextFieldFlags::None)
            {
                if (Members->handlers != nullptr)
                {
                    auto t_h = (Controls::Handlers::TextControl*) Members->handlers.get();
                    if (t_h->OnTextColor.obj)
                    {
                        t_h->OnTextColor.obj->OnTextColor(this, Members->Text.GetBuffer(), Members->Text.Len());
                    }
                    else
                    {
                        Members->Text.SetColor(params.Color);
                    }
                }
                else
                {
                    Members->Text.SetColor(params.Color);
                }
            }
            else
                Members->Text.SetColor(params.Color);
            if ((Members->Selection.Start >= 0) && (Members->Selection.End >= 0) &&
                (Members->Selection.End >= Members->Selection.Start))
                Members->Text.SetColor(
                      Members->Selection.Start, Members->Selection.End + 1, Members->Cfg->Selection.Editor);
            Members->Modified = false;
        }
        else
        {
            // if no selection is present and no syntax highlighting --> use overwrite colors as it is faster
            if ((Members->Selection.Start < 0) &&
                ((Members->Flags & TextFieldFlags::SyntaxHighlighting) == TextFieldFlags::None))
                params.Flags |= WriteTextFlags::OverwriteColors;
        }
    }
    else
    {
        params.Flags |= WriteTextFlags::OverwriteColors;
    }
    renderer.WriteText(
          CharacterView(
                Members->Text.GetBuffer() + Members->Cursor.StartOffset,
                Members->Text.Len() - Members->Cursor.StartOffset),
          params);
    if (Members->Focused)
    {
        int y = (Members->Cursor.Pos - Members->Cursor.StartOffset) / params.Width;
        int x = (Members->Cursor.Pos - Members->Cursor.StartOffset) % params.Width;
        if ((x == 0) && (y == Members->Layout.Height))
        {
            x = params.Width;
            y--;
        }
        renderer.SetCursor(x + 1, y);
    }
}
void TextField::OnAfterResize(int /*newWidth*/, int /*newHeight*/)
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    int sz            = Members->Text.Len();
    auto visibleChars = C_WIDTH;
    if (sz < visibleChars)
    {
        Members->Cursor.StartOffset = 0;
    }
    else
    {
        if (Members->Cursor.Pos >= visibleChars)
            Members->Cursor.StartOffset = Members->Cursor.Pos - visibleChars;
        else
            Members->Cursor.StartOffset = 0;
    }
}
void TextField::OnFocus()
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, this, Members, );
    if (!(Members->Flags && TextFieldFlags::DisableAutoSelectOnFocus))
    {
        SelectAll();
        Members->FullSelectionDueToOnFocusEvent = Members->Text.Len() > 0;
    }
}
bool TextField::OnMouseEnter()
{
    return true;
}
bool TextField::OnMouseLeave()
{
    return true;
}
void TextField::OnMousePressed(int x, int y, Input::MouseButton button)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, this, Members, );
    if (button == (MouseButton::DoubleClicked | MouseButton::Left))
    {
        TextField_MoveTo(this, TextField_MouseToTextPos(this, x, y), false);
        TextField_SelectWorld(this);
        return;
    }

    if ((button & MouseButton::Left) != MouseButton::None)
    {
        if (Members->FullSelectionDueToOnFocusEvent)
        {
            Members->FullSelectionDueToOnFocusEvent = false;
            return;
        }
        ClearSelection();
        TextField_MoveTo(this, TextField_MouseToTextPos(this, x, y), false);
    }
    Members->FullSelectionDueToOnFocusEvent = false;
    if (button == MouseButton::Right)
    {
        if ((Members->handlers) &&
            ((reinterpret_cast<Handlers::TextControl*>(Members->handlers.get()))->OnTextRightClick.obj))
        {
            (reinterpret_cast<Handlers::TextControl*>(Members->handlers.get()))
                  ->OnTextRightClick.obj->OnTextRightClick(this, x, y);
        }
        else
        {
            if (textFieldContexMenu == nullptr)
            {
                textFieldContexMenu = new Internal::TextControlDefaultMenu();
            }
            textFieldContexMenu->Show(this, x, y + 1, Members->Selection.Start >= 0);
        }
    }
}
bool TextField::OnEvent(Reference<Control> /*sender*/, Event eventType, int controlID)
{
    if (eventType == Event::Command)
    {
        switch (controlID)
        {
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_COPY:
            this->CopyToClipboard(false);
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_CUT:
            this->CopyToClipboard(true);
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_PASTE:
            this->PasteFromClipboard();
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_SELECT_ALL:
            this->SelectAll();
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_DELETE_SELECTED:
            OnKeyEvent(Key::Delete, 0);
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_TO_UPPER:
            TextField_ToUpper(this);
            return true;
        case Internal::TextControlDefaultMenu::TEXTCONTROL_CMD_TO_LOWER:
            TextField_ToLower(this);
            return true;
        }
    }
    return false;
}
bool TextField::OnMouseDrag(int x, int y, Input::MouseButton /*button*/)
{
    TextField_MoveTo(this, TextField_MouseToTextPos(this, x, y), true);
    return true;
}
void TextField::OnMouseReleased(int /*x*/, int /*y*/, Input::MouseButton /*button*/)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, this, Members, );
}
Handlers::TextControl* TextField::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::TextControl);
}
} // namespace AppCUI::Controls