#include "ControlContext.hpp"

namespace AppCUI
{
constexpr uint32 INVALID_CHARACTER         = 0xFFFFFFFF;
constexpr int32 CHARACTER_X_OFFSET         = 7;
constexpr int32 OFFSET_X_RIGHT_MARGIN      = 5;
constexpr int32 CHARACTER_HEX_VALUE_OFFSET = 4;

void CharacterTableContext::MoveTo(uint32 newCharCode)
{
    newCharCode   = std::min<uint32>(0xFFFF, newCharCode);
    const auto sz = (uint32) this->GetCharPerWidth() * (uint32) (this->Layout.Height - 2);

    // if already in current view --> exit
    if ((newCharCode >= this->startView) && (newCharCode < this->startView + sz))
    {
        this->character = newCharCode;
        return;
    }
    // move up
    if (newCharCode < this->startView)
    {
        if (this->character >= this->startView)
        {
            auto dif = this->character - this->startView;
            if (dif <= newCharCode)
                this->startView = newCharCode - dif;
            else
                this->startView = 0;
            this->character = newCharCode;
            // sanity check
            if (this->character > this->startView + sz)
                this->startView = this->character; // reset
        }
        else
        {
            this->character = newCharCode;
            this->startView = newCharCode;
        }
        return;
    }
    // move down
    if (this->character >= this->startView)
    {
        auto dif = this->character - this->startView;
        if (dif <= newCharCode)
            this->startView = newCharCode - dif;
        else
            this->startView = 0;
        this->character = newCharCode;
        // sanity check
        if (this->character > this->startView + sz)
            this->startView = this->character; // reset
    }
    else
    {
        this->character = newCharCode;
        if (newCharCode + 1 >= sz)
            this->startView = newCharCode + 1 - sz;
        else
            this->startView = 0;
        // sanity check
        if (this->character > this->startView + sz)
            this->startView = this->character; // reset
    }
}
bool CharacterTableContext::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    const uint32 w = (uint32) this->GetCharPerWidth();
    const auto sz  = (uint32) this->GetCharPerWidth() * (uint32) (this->Layout.Height - 2);
    switch (keyCode)
    {
    case Key::Up:
        if (this->character > w)
            MoveTo(this->character - w);
        else
            MoveTo(0);
        return true;
    case Key::Down:
        MoveTo(this->character + w);
        return true;
    case Key::Left:
        if (this->character > 0)
            MoveTo(this->character - 1);
        else
            MoveTo(0);
        return true;
    case Key::Right:
        MoveTo(this->character + 1);
        return true;
    case Key::Home:
        MoveTo(0);
        return true;
    case Key::End:
        MoveTo(0xFFFF);
        return true;
    case Key::PageUp:
        if (this->character > sz)
            MoveTo(this->character - sz);
        else
            MoveTo(0);
        return true;
    case Key::PageDown:
        MoveTo(this->character + sz);
        return true;
    case Key::Escape:
        if (editMode)
        {
            editMode = false;
            return true;
        }
        break;
    case Key::Backspace:
        editMode = true;
        MoveTo(this->character >> 4);
        return true;
    }
    if ((UnicodeChar >= '0') && (UnicodeChar <= '9'))
    {
        editMode = true;
        MoveTo((this->character << 4) + (UnicodeChar - '0'));
        return true;
    }
    if ((UnicodeChar >= 'A') && (UnicodeChar <= 'F'))
    {
        editMode = true;
        MoveTo((this->character << 4) + (UnicodeChar - 'A' + 10));
        return true;
    }
    if ((UnicodeChar >= 'a') && (UnicodeChar <= 'f'))
    {
        editMode = true;
        MoveTo((this->character << 4) + (UnicodeChar - 'a' + 10));
        return true;
    }
    return false;
}
uint32 CharacterTableContext::MousePosToChar(int x, int y)
{
    if ((y <= 0) || (y >= (this->Layout.Height - 1)))
        return INVALID_CHARACTER;
    if ((x < CHARACTER_X_OFFSET) || (x >= this->Layout.Width))
        return INVALID_CHARACTER;
    x -= CHARACTER_X_OFFSET;
    x = x >> 1;
    if (x >= this->GetCharPerWidth())
        return INVALID_CHARACTER;
    auto c_code = (uint32) ((y - 1) * this->GetCharPerWidth()) + this->startView + (uint32) x;
    if (c_code > 0xFFFF)
        return INVALID_CHARACTER;
    return c_code;
}
void CharacterTableContext::OnMousePressed(int x, int y, Input::MouseButton /*button*/)
{
    if (y == (this->Layout.Height - 1))
    {
        this->editMode = true;
        return;
    }
    auto code = MousePosToChar(x, y);
    if (code != INVALID_CHARACTER)
    {
        this->editMode = false;
        MoveTo(code);
        return;
    }
}
bool CharacterTableContext::OnMouseWheel(Input::MouseWheel direction)
{
    const auto sz = (uint32) this->GetCharPerWidth() * (uint32) (this->Layout.Height - 2);
    switch (direction)
    {
    case MouseWheel::Up:
        if (this->character > sz)
            MoveTo(this->character - sz);
        else
            MoveTo(0);
        return true;
    case MouseWheel::Down:
        MoveTo(this->character + sz);
        return true;
    }
    return false;
}
bool CharacterTableContext::OnMouseOver(int x, int y, uint32& code, int& toolTipX)
{
    code = MousePosToChar(x, y);
    if (code == hoverChar)
        return false;
    hoverChar = code;
    toolTipX  = ((code - this->startView) % ((uint32) this->GetCharPerWidth())) * 2 + 7;
    return true;
}
void CharacterTableContext::Paint(Graphics::Renderer& renderer)
{
    NumericFormatter n;
    auto* col = &this->Cfg->CharacterTable.Normal;

    if (!(this->Flags & GATTR_ENABLE))
        col = &this->Cfg->CharacterTable.Inactive;
    else if (this->Focused)
        col = &this->Cfg->CharacterTable.Focus;
    else if (this->MouseIsOver)
        col = &this->Cfg->CharacterTable.Hover;

    renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, col->Border, LineType::Single);
    renderer.DrawVerticalLine(OFFSET_X_RIGHT_MARGIN, 1, this->Layout.Height - 2, col->Border, false);
    renderer.FillRect(1, 1, OFFSET_X_RIGHT_MARGIN - 1, this->Layout.Height - 2, ' ', col->Offset);
    if (Focused)
    {
        renderer.WriteSingleLineText(1, this->Layout.Height - 1, " 0x     ", Cfg->SearchBar.Inactive);
        auto v = n.ToHex(this->character);
        if (editMode)
        {
            renderer.WriteSingleLineText(CHARACTER_HEX_VALUE_OFFSET, this->Layout.Height - 1, v, Cfg->SearchBar.Focused);
            renderer.SetCursor(CHARACTER_HEX_VALUE_OFFSET + (int32) v.length(), this->Layout.Height - 1);
        }
        else
        {
            renderer.WriteSingleLineText(CHARACTER_HEX_VALUE_OFFSET, this->Layout.Height - 1, v, Cfg->SearchBar.Normal);
        }
    }
    renderer.SetClipMargins(1, 1, 1, 1);

    // paint all chars
    auto w        = this->GetCharPerWidth();
    auto charCode = startView;
    for (auto y = 0; y < this->Layout.Height - 2; y++)
    {
        // write the offset
        renderer.WriteSingleLineText(
              OFFSET_X_RIGHT_MARGIN - 1, y + 1, n.ToHex(charCode), col->Offset, TextAlignament::Right);
        for (auto x = 0; (x < w) && (charCode < 0x10000); x++, charCode++)
        {
            renderer.WriteCharacter(x * 2 + CHARACTER_X_OFFSET, y + 1, charCode, col->Text);
            if (charCode == character)
            {
                renderer.FillHorizontalLine(
                      x * 2 + CHARACTER_X_OFFSET - 1, y + 1, x * 2 + CHARACTER_X_OFFSET + 1, -1, col->Cursor);
                if ((this->Focused) && (!editMode))
                    renderer.SetCursor(x * 2 + CHARACTER_X_OFFSET, y + 1);
            }
        }

        if (charCode > 0xFFFF)
            break;
    }
}
CharacterTable::CharacterTable(string_view layout) : Control(new CharacterTableContext(), "", layout, false)
{
    auto Members              = reinterpret_cast<CharacterTableContext*>(this->Context);
    Members->Layout.MinWidth  = 10;
    Members->Layout.MinHeight = 3;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_VSCROLL;
    Members->character        = 0;
    Members->startView        = 0;
    Members->editMode         = false;
    Members->hoverChar        = INVALID_CHARACTER;
}
CharacterTable::~CharacterTable()
{
}
void CharacterTable::Paint(Graphics::Renderer& renderer)
{
    reinterpret_cast<CharacterTableContext*>(this->Context)->Paint(renderer);
}
bool CharacterTable::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    return reinterpret_cast<CharacterTableContext*>(this->Context)->OnKeyEvent(keyCode, UnicodeChar);
}
bool CharacterTable::OnMouseWheel(int /*x*/, int /*y*/, Input::MouseWheel direction)
{
    return reinterpret_cast<CharacterTableContext*>(this->Context)->OnMouseWheel(direction);
}
bool CharacterTable::OnMouseLeave()
{
    return true;
}
bool CharacterTable::OnMouseEnter()
{
    return true;
}
bool CharacterTable::OnMouseOver(int x, int y)
{
    uint32 code;
    int32 toolTipX;
    if (reinterpret_cast<CharacterTableContext*>(this->Context)->OnMouseOver(x, y, code, toolTipX) == false)
        return false;
    if (code == INVALID_CHARACTER)
    {
        this->HideToolTip();
    }
    else
    {
        NumericFormatter n;
        LocalUnicodeStringBuilder<128> tmpUnicode;
        tmpUnicode.Add("Chr: ");
        if (code > 0)
            tmpUnicode.AddChar((char16) code);
        tmpUnicode.Add("\nHex: 0x");
        tmpUnicode.Add(n.ToHex(code));
        tmpUnicode.Add("\nDec: ");
        tmpUnicode.Add(n.ToDec(code));
        tmpUnicode.Add("\nOct: ");
        tmpUnicode.Add(n.ToOct(code));
        this->ShowToolTip(tmpUnicode, toolTipX, y);
    }
    return true;
}
void CharacterTable::OnMousePressed(int x, int y, Input::MouseButton button)
{
    reinterpret_cast<CharacterTableContext*>(this->Context)->OnMousePressed(x, y, button);
}
void CharacterTable::OnUpdateScrollBars()
{
    UpdateVScrollBar(reinterpret_cast<CharacterTableContext*>(this->Context)->character, 0xFFFF);
}
void CharacterTable::SetCharacter(char16 character)
{
    reinterpret_cast<CharacterTableContext*>(this->Context)->MoveTo(character);
}
char16 CharacterTable::GetCharacter()
{
    return reinterpret_cast<CharacterTableContext*>(this->Context)->character;
}

} // namespace AppCUI