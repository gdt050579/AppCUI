#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define C_WIDTH ((Members->Layout.Width - 2) * Members->Layout.Height)
#define EXIT_IF_READONLY()                                                                                             \
    if ((Members->Flags & TextFieldFlags::Readonly) != TextFieldFlags::None)                                           \
    {                                                                                                                  \
        return;                                                                                                        \
    };

#define DEFAULT_TEXT_COLOR 0xFFFFFFFF

void TextField_SendTextChangedEvent(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if ((Members->Flags & TextFieldFlags::SyntaxHighlighting) != TextFieldFlags::None)
    {
        Members->Syntax.Handler(control, Members->Text.GetBuffer(), Members->Text.Len(), Members->Syntax.Context);
    }
    control->RaiseEvent(Event::TextChanged);
}

void TextField_MoveSelTo(TextField* control, int poz)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
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
    int c_width = C_WIDTH;
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
bool __is_op__(unsigned int, Character ch)
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
bool __is_not_op__(unsigned int, Character ch)
{
    if ((ch == '\n') || (ch == '\r') || (ch == ' ') || (ch == '\t'))
        return false;
    return !__is_op__(0, ch);
}
void TextField_MoveToNextWord(TextField* control, bool selected)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Cursor.Pos >= (int)Members->Text.Len())
        return;
    auto currentChar                = Members->Text.GetBuffer()[Members->Cursor.Pos];
    std::optional<unsigned int> res = std::nullopt;

    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = Members->Text.FindNext(
              Members->Cursor.Pos, [](unsigned int, Character ch) { return (ch == ' ') || (ch == '\t'); });
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
    if (res.has_value())
        res = Members->Text.FindNext(
              res.value(), [](unsigned int, Character ch) { return (ch == ' ') || (ch == '\t'); });
    if (res.has_value())
        TextField_MoveTo(control, res.value(), selected);
}
void TextField_MoveToPreviousWord(TextField* control, bool selected)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    if (Members->Cursor.Pos == 0)
        return;
    auto startPoz                   = Members->Cursor.Pos - 1;
    auto currentChar                = Members->Text.GetBuffer()[startPoz];
    std::optional<unsigned int> res = std::nullopt;

    if ((currentChar == ' ') || (currentChar == '\t'))
    {
        res = Members->Text.FindPrevious(
              startPoz, [](unsigned int, Character ch) { return (ch == ' ') || (ch == '\t'); });
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
        if (res.value()>0)
            TextField_MoveTo(control, res.value()+1, selected);
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
void TextField_AddChar(TextField* control, char16_t ch)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    TextField_DeleteSelected(control);
    if (Members->Cursor.Pos > (int) Members->Text.Len())
        Members->Text.InsertChar(ch, Members->Text.Len());
    else
        Members->Text.InsertChar(ch, (unsigned int) (Members->Cursor.Pos));
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
void TextField_CopyToClipboard(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );

    if (!TextField_HasSelection(control))
        return;
    if (!AppCUI::OS::Clipboard::SetText(Members->Text.SubString(Members->Selection.Start, (size_t)Members->Selection.End + 1)))
    {
        LOG_WARNING("Fail to copy string to the clipboard");
    }
}
void TextField_PasteFromClipboard(TextField* control)
{
    CREATE_TYPE_CONTEXT(TextFieldControlContext, control, Members, );
    EXIT_IF_READONLY();
    LocalUnicodeStringBuilder<2048> temp;
    if (AppCUI::OS::Clipboard::GetText(temp) == false)
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
//============================================================================
TextField::~TextField()
{
    DELETE_CONTROL_CONTEXT(TextFieldControlContext);
}
std::unique_ptr<TextField> TextField::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      TextFieldFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    INIT_CONTROL(TextField, TextFieldControlContext);
    Members->Layout.MinWidth  = 3;
    Members->Layout.MinHeight = 1;
    Members->Syntax.Handler   = nullptr;
    Members->Syntax.Context   = nullptr;

    CHECK(me->Init(caption, layout, false), nullptr, "Failed to create text field !");

    Members->Flags    = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int) flags;
    Members->Modified = true;
    ClearSelection();
    Members->Cursor.Pos = Members->Cursor.StartOffset = 0;
    TextField_MoveTo(this, 0xFFFF, false);
    if ((Members->Flags & TextFieldFlags::SyntaxHighlighting) != TextFieldFlags::None) 
    {
        Members->Syntax.Handler = handler;
        Members->Syntax.Context = handlerContext;
        CHECK(handler,
              nullptr,
              "if 'TextFieldFlags::SyntaxHighlighting` is set a syntaxt highligh handler must be provided");
    }
    return me;
}

TextField* TextField::Create(
      Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      TextFieldFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    auto me = TextField::Create(caption, layout, flags, handler, handlerContext);
    CHECK(me, nullptr, "Fail to create a TextField control !");
    return parent.AddControl<TextField>(std::move(me));
}

void TextField::SelectAll()
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    Members->Selection.Start = 0;
    Members->Selection.End   = Members->Text.Len() - 1;
    if (Members->Selection.End < 0)
        ClearSelection();
    Members->Modified = true;
}
void TextField::ClearSelection()
{
    CREATE_TYPECONTROL_CONTEXT(TextFieldControlContext, Members, );
    Members->Selection.Start = Members->Selection.End = Members->Selection.Origin = -1;
    Members->Modified                                                             = true;
}

bool TextField::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t UnicodeChar)
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
    case Key::Ctrl | Key::Insert:
    case Key::Ctrl | Key::C:
        TextField_CopyToClipboard(this);
        return true;
    case Key::Ctrl | Key::V:
    case Key::Shift | Key::Insert:
        TextField_PasteFromClipboard(this);
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
void TextField::OnAfterSetText(const AppCUI::Utils::ConstString&)
{
    // repozitionez cursorul
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
    if (Members->Layout.Height == 1)
        params.Flags |= WriteTextFlags::SingleLine;
    else
        params.Flags |= WriteTextFlags::MultipleLines;

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
            if ((Members->Flags & TextFieldFlags::SyntaxHighlighting) != TextFieldFlags::None) 
            {
                Members->Syntax.Handler(this, Members->Text.GetBuffer(), Members->Text.Len(), Members->Syntax.Context);
            }
            else
                Members->Text.SetColor(params.Color);
            if ((Members->Selection.Start >= 0) && (Members->Selection.End >= 0) &&
                (Members->Selection.End >= Members->Selection.Start))
                Members->Text.SetColor(
                      Members->Selection.Start, Members->Selection.End + 1, Members->Cfg->Text.SelectionColor);
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
