#include "Internal.h"

using namespace AppCUI::Internal;
using namespace AppCUI::Console;

#define TRANSLATE_X_COORDONATE(x) x+=this->TranslateX;
#define TRANSLATE_Y_COORDONATE(y) y+=this->TranslateY;
#define TRANSLATE_COORDONATES(x,y) { x += this->TranslateX; y += this->TranslateY; }
#define CHECK_VISIBLE if (this->Clip.Visible == false) return false;

ConsoleRenderer::ConsoleRenderer()
{
    TranslateX = TranslateY = 0;
    ConsoleSize.Set(0, 0);
    WorkingBuffer = nullptr;
    OffsetRows = nullptr;
    SpecialCharacters = nullptr;
    this->Clip.Visible = false;
    this->Cursor.Visible = false;
    this->Cursor.X = this->Cursor.Y = 0;
    this->LastUpdateCursor.Visible = false;
    this->LastUpdateCursor.X = 0xFFFFFFFF;
    this->LastUpdateCursor.Y = 0xFFFFFFFF;
}
bool ConsoleRenderer::CreateScreenBuffers(unsigned int width, unsigned int height)
{
    this->Clip.Visible = false;
    CHECK(width > 0, false, "width must be bigger than 0");
    CHECK(height > 0, false, "height must be bigger than 0");
    this->ConsoleSize.Set(width, height);
    if (this->WorkingBuffer) {
        delete []this->WorkingBuffer;
        this->WorkingBuffer = nullptr;
    }
    this->WorkingBuffer = new CHARACTER_INFORMATION[width*height];
    CHECK(this->WorkingBuffer, false, "Fail to allocate a screen of size: %d x %d", width, height);
    if (this->OffsetRows)
    {
        delete[]this->OffsetRows;
        this->OffsetRows = nullptr;
    }
    this->OffsetRows = new CHARACTER_INFORMATION*[height];
    CHECK(this->OffsetRows, false, "Fail to allocate offset rows lookups for %d rows", height);
    // all good --> associate the rows to the screen
    CHARACTER_INFORMATION* p = this->WorkingBuffer;
    for (unsigned int tr = 0; tr < height; tr++, p += width)
        this->OffsetRows[tr] = p;
    
    // clear current buffer
    CHARACTER_INFORMATION* e = this->WorkingBuffer + width * height;
    p = this->WorkingBuffer;
    while (p < e) {
        SET_CHARACTER(p, ' ', 0);
        p++;
    }

    // copy data from 
    unsigned int h = MINVALUE(height, this->BeforeInitConfig.consoleSize.Height);
    unsigned int w = MINVALUE(width, this->BeforeInitConfig.consoleSize.Width);

    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            *(this->WorkingBuffer + y * width + x) = *(this->BeforeInitConfig.screenBuffer + y * this->BeforeInitConfig.consoleSize.Width + x);
        }
    }

    return true;
}
void ConsoleRenderer::SetClip(const AppCUI::Console::Clip & clip)
{
    if (clip.Visible)
    {
        // make sure that clipping coordonates are within screen coordonates
        this->Clip.Left = MAXVALUE(clip.ClipRect.X, 0);
        this->Clip.Top = MAXVALUE(clip.ClipRect.Y, 0);
        this->Clip.Right = clip.ClipRect.X + clip.ClipRect.Width - 1;
        this->Clip.Bottom = clip.ClipRect.Y + clip.ClipRect.Height - 1;
        if (this->Clip.Right >= (int)this->ConsoleSize.Width)
            this->Clip.Right = (int)this->ConsoleSize.Width - 1;
        if (this->Clip.Bottom >= (int)this->ConsoleSize.Height)
            this->Clip.Bottom = (int)this->ConsoleSize.Height - 1;
        this->Clip.Visible = (Clip.Left <= Clip.Right) && (Clip.Top <= Clip.Bottom);
    }
    else {
        this->Clip.Visible = false;
    }
}
void ConsoleRenderer::ResetClip()
{
    this->Clip.Left = this->Clip.Right = 0;
    this->Clip.Right = this->ConsoleSize.Width - 1;
    this->Clip.Bottom = this->ConsoleSize.Height - 1;
    this->Clip.Visible = true;
}
void ConsoleRenderer::SetTranslate(int offX, int offY)
{
    this->TranslateX = offX;
    this->TranslateY = offY;
}
bool ConsoleRenderer::SetSize(unsigned int width, unsigned int height)
{
    CHECK(CreateScreenBuffers(width, height), false, "Fail to create a screen buffer !");
    return true;
}
void ConsoleRenderer::Prepare()
{
    this->TranslateX = this->TranslateY = 0;
    this->Clip.Left = this->Clip.Right = 0;
    this->Clip.Right = this->ConsoleSize.Width - 1;
    this->Clip.Bottom = this->ConsoleSize.Height - 1;
    this->Clip.Visible = true;
    this->HideCursor();
}
void ConsoleRenderer::UpdateScreen()
{
    this->FlushToScreen();
    if ((this->Cursor.Visible != this->LastUpdateCursor.Visible) ||
        (this->Cursor.X != this->LastUpdateCursor.X) || 
        (this->Cursor.Y != this->LastUpdateCursor.Y))
    {
        if (this->UpdateCursor())
        {
            // update last cursor information
            this->LastUpdateCursor = this->Cursor;
        }
    }
    
}
void ConsoleRenderer::HideCursor()
{
    this->Cursor.Visible = false;
}
bool ConsoleRenderer::ShowCursor(int x, int y)
{
    TRANSLATE_COORDONATES(x, y);
    if ((x < Clip.Left) || (x > Clip.Right) || (y < Clip.Top) || (y > Clip.Bottom))
    {
        this->Cursor.Visible = false;
        return false;
    }
    this->Cursor.X = (unsigned int)x;
    this->Cursor.Y = (unsigned int)y;
    this->Cursor.Visible = true;
    return true;
}
bool ConsoleRenderer::WriteCharacter(int x, int y, int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);
    if ((x < Clip.Left) || (x > Clip.Right) || (y < Clip.Top) || (y > Clip.Bottom))
        return false;
    CHARACTER_INFORMATION *p = this->OffsetRows[y] + x;
    if ((charCode > 0) && (color < 256))
    {
        SET_CHARACTER(p, charCode, color);
    }
    else {
        SET_CHARACTER_EX(p, charCode, color);
    }
    return true;
}
bool ConsoleRenderer::ClearClipRectangle(int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    if ((Clip.Left == 0) && (Clip.Top == 0) && (Clip.Right+1 == this->ConsoleSize.Width) && (Clip.Bottom+1 == this->ConsoleSize.Height))
    {
        //optimize for the entire screen
        CHARACTER_INFORMATION * start = this->WorkingBuffer;
        CHARACTER_INFORMATION * end = this->WorkingBuffer + (this->ConsoleSize.Width * this->ConsoleSize.Height);
        if ((charCode >= 0) && (color < 256))
        {
            while (start < end) {
                SET_CHARACTER(start, charCode, color);
                start++;
            }
        }
        else {
            while (start < end) {
                SET_CHARACTER_EX(start, charCode, color);
                start++;
            }
        }
        return true;
    }
    else {
        return FillRect(Clip.Left-TranslateX, Clip.Top-TranslateY, Clip.Right-TranslateX, Clip.Bottom-TranslateY, charCode, color);
    }
}
bool ConsoleRenderer::FillHorizontalLine(int left, int y, int right, int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, y);
    TRANSLATE_X_COORDONATE(right);
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    left = MAXVALUE(left, Clip.Left);
    right = MINVALUE(right, Clip.Right);
    if (left > right)
        return false;
    CHARACTER_INFORMATION * p = this->OffsetRows[y] + left;
    if ((charCode >= 0) && (color < 256))
    {
        while (left <= right)
        {
            SET_CHARACTER(p, charCode, color);
            left++;
            p++;
        }
    }
    else {
        while (left <= right)
        {
            SET_CHARACTER_EX(p, charCode, color);
            left++;
            p++;
        }
    }
    return true;
}
bool ConsoleRenderer::FillVerticalLine(int x, int top, int bottom, int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, top);
    TRANSLATE_Y_COORDONATE(bottom);
    if ((x < Clip.Left) || (x > Clip.Right))
        return false;
    top = MAXVALUE(top, Clip.Top);
    bottom = MINVALUE(bottom, Clip.Bottom);
    if (top > bottom)
        return false;
    CHARACTER_INFORMATION * p = this->OffsetRows[top] + x;
    if ((charCode >= 0) && (color < 256))
    {
        while (top <= bottom)
        {
            SET_CHARACTER(p, charCode, color);
            top++;
            p += this->ConsoleSize.Width;
        }
    }
    else {
        while (top <= bottom)
        {
            SET_CHARACTER_EX(p, charCode, color);
            top++;
            p += this->ConsoleSize.Width;
        }
    }
    return true;
}
bool ConsoleRenderer::FillRect(int left, int top, int right, int bottom, int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, top);
    TRANSLATE_COORDONATES(right, bottom);

    left = MAXVALUE(left, Clip.Left);
    top = MAXVALUE(top, Clip.Top);
    right = MINVALUE(right, Clip.Right);
    bottom = MINVALUE(bottom, Clip.Bottom);

    if ((left > right) || (top > bottom))
        return false;
    CHARACTER_INFORMATION *p;
    CHARACTER_INFORMATION **row = this->OffsetRows;
    row += top;
    if ((charCode >= 0) && (color < 256))
    {
        for (int y = top; y <= bottom; y++,row++)
        {
            p = (*row) + left;
            for (int x = left; x <= right; x++, p++)
            {
                SET_CHARACTER(p, charCode, color);
            }
        }
    }
    else {
        for (int y = top; y <= bottom; y++,row++)
        {
            p = (*row) + left;
            for (int x = left; x <= right; x++, p++)
            {
                SET_CHARACTER_EX(p, charCode, color);
            }
        }
    }
    return true;
}
bool ConsoleRenderer::DrawRect(int left, int top, int right, int bottom, unsigned int color, bool doubleLine)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, top);
    TRANSLATE_COORDONATES(right, bottom);

    int orig_left = left;
    int orig_right = right;
    int orig_top = top;
    int orig_bottom = bottom;

    left = MAXVALUE(left, Clip.Left);
    top = MAXVALUE(top, Clip.Top);
    right = MINVALUE(right, Clip.Right);
    bottom = MINVALUE(bottom, Clip.Bottom);

    if ((left > right) || (top > bottom))
        return false;

    CHARACTER_INFORMATION *p, *e;
    int char_to_draw;
    // top line
    if (top == orig_top)
    {
        p = this->OffsetRows[top] + left;
        e = this->OffsetRows[top] + right;
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxHorizontalDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxHorizontalSingleLine];
        if (color < 256)
        {
            while (p <= e) { SET_CHARACTER(p, char_to_draw, color); p++; }
        }
        else {
            while (p <= e) { SET_CHARACTER_EX(p, char_to_draw, color); p++; }
        }
    }
    // bottom line
    if (bottom == orig_bottom)
    {
        p = this->OffsetRows[bottom] + left;
        e = this->OffsetRows[bottom] + right;
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxHorizontalDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxHorizontalSingleLine];
        if (color < 256)
        {
            while (p <= e) { SET_CHARACTER(p, char_to_draw, color); p++; }
        }
        else {
            while (p <= e) { SET_CHARACTER_EX(p, char_to_draw, color); p++; }
        }
    }
    // left line
    if (left == orig_left)
    {
        p = this->OffsetRows[top] + left;
        e = this->OffsetRows[bottom] + left;
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxVerticalDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxVerticalSingleLine];
        if (color < 256)
        {
            while (p <= e) { SET_CHARACTER(p, char_to_draw, color); p+=ConsoleSize.Width; }
        }
        else {
            while (p <= e) { SET_CHARACTER_EX(p, char_to_draw, color); p += ConsoleSize.Width; }
        }
    }
    // right line
    if (right == orig_right)
    {
        p = this->OffsetRows[top] + right;
        e = this->OffsetRows[bottom] + right;
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxVerticalDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxVerticalSingleLine];
        if (color < 256)
        {
            while (p <= e) { SET_CHARACTER(p, char_to_draw, color); p += ConsoleSize.Width; }
        }
        else {
            while (p <= e) { SET_CHARACTER_EX(p, char_to_draw, color); p += ConsoleSize.Width; }
        }
    }
    // corners
    if ((left == orig_left) && (top == orig_top))
    {
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxTopLeftCornerDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxTopLeftCornerSingleLine];
        p = this->OffsetRows[top] + left;
        if (color < 256) {
            SET_CHARACTER(p, char_to_draw, color);
        } else {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((left == orig_left) && (bottom == orig_bottom))
    {
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxBottomLeftCornerDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxBottomLeftCornerSingleLine];
        p = this->OffsetRows[bottom] + left;
        if (color < 256) {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((right == orig_right) && (bottom == orig_bottom))
    {
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxBottomRightCornerDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxBottomRightCornerSingleLine];
        p = this->OffsetRows[bottom] + right;
        if (color < 256) {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((right == orig_right) && (top == orig_top))
    {
        if (doubleLine)
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxTopRightCornerDoubleLine];
        else
            char_to_draw = this->SpecialCharacters[SpecialChars::BoxTopRightCornerSingleLine];
        p = this->OffsetRows[top] + right;
        if (color < 256) {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    return true;
}
bool ConsoleRenderer::WriteSingleLineText(int x, int y, const char * text, unsigned int color, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    if (x > Clip.Right)
        return false;
    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    if (x + textSize < Clip.Left)
        return false;
    const unsigned char * s = (const unsigned char *)text;
    const unsigned char * e = s + textSize;
    // needs more optimizations
    CHARACTER_INFORMATION * c = this->OffsetRows[y] + x;
    while ((s < e) && (x<=Clip.Right))
    {
        if (x >= Clip.Left)
        {
            SET_CHARACTER_EX(c, *s, color);
        }
        c++;
        s++;
        x++;
    }    
    return true;
}
bool ConsoleRenderer::WriteSingleLineTextWithHotKey(int x, int y, const char * text, unsigned int color, unsigned int hotKeyColor, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    if (x > Clip.Right)
        return false;
    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    if (x + textSize < Clip.Left)
        return false;
    const unsigned char * s = (const unsigned char *)text;
    const unsigned char * e = s + textSize;
    // needs more optimizations
    CHARACTER_INFORMATION * c = this->OffsetRows[y] + x;
    CHARACTER_INFORMATION * hotkey = nullptr;
    while ((s < e) && (x <= Clip.Right))
    {
        if (x >= Clip.Left)
        {
            if ((!hotkey) && ((*s) == '&')) {
                hotkey = c;
                x--;
                c--;
            }
            else {
                SET_CHARACTER_EX(c, *s, color);
            }
        }
        c++;
        s++;
        x++;
    }
    if (hotkey)
        SET_CHARACTER_EX(hotkey, -1, hotKeyColor);
    return true;
}
bool ConsoleRenderer::WriteMultiLineText(int x, int y, const char * text, unsigned int color, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    const unsigned char * s = (const unsigned char *)text;
    const unsigned char * e = s + textSize;
    CHARACTER_INFORMATION * c = this->OffsetRows[y] + x;
    int orig_x = x;
    while ((s < e) && (y <= Clip.Bottom))
    {
        if ((*s) != '\n')
        {
            if ((x >= Clip.Left) && (x <= Clip.Right))
            {
                SET_CHARACTER_EX(c, *s, color);
            }
            c++;
            x++;
        }
        else {
            y++;
            x = orig_x;
            c = this->OffsetRows[y] + x;
        }
        s++;
    }
    return true;
}
bool ConsoleRenderer::WriteMultiLineTextWithHotKey(int x, int y, const char * text, unsigned int color, unsigned int hotKeyColor, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    const unsigned char * s = (const unsigned char *)text;
    const unsigned char * e = s + textSize;
    CHARACTER_INFORMATION * c = this->OffsetRows[y] + x;
    CHARACTER_INFORMATION * hotkey = nullptr;
    int orig_x = x;
    while ((s < e) && (y <= Clip.Bottom))
    {
        if ((*s) != '\n')
        {
            if ((x >= Clip.Left) && (x <= Clip.Right))
            {
                if ((!hotkey) && ((*s) == '&')) {
                    hotkey = c;
                    x--;
                    c--;
                }
                else {
                    SET_CHARACTER_EX(c, *s, color);
                }
            }
            c++;
            x++;
        }
        else {
            y++;
            x = orig_x;
            c = this->OffsetRows[y] + x;
        }
        s++;
    }
    if (hotkey)
        SET_CHARACTER_EX(hotkey, -1, hotKeyColor);
    return true;
}
bool ConsoleRenderer::WriteCharacterBuffer_SingleLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end)
{
    TRANSLATE_COORDONATES(x, y);
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    if (x > Clip.Right)
        return false;
    if (x < Clip.Left)
    {
        start += (unsigned int)(Clip.Left - x);
        x = Clip.Left;
        if (start >= end)
            return false;
    }
    if ((Clip.Right + 1 - x) <= (int)(end - start))
        end = start + (unsigned int)(Clip.Right + 1 - x);
    if (params.Flags & WriteCharacterBufferFlags::WRAP_TO_WIDTH)
    {
        if ((end - start) > params.Width)
            end = start + params.Width;
    }
    const AppCUI::Console::Character * ch = cb.GetBuffer() + start;
    CHARACTER_INFORMATION * p = this->OffsetRows[y] + x;
    if (params.Flags & WriteCharacterBufferFlags::OVERWRITE_COLORS)
    {
        unsigned int position = start;
        if (params.Color < 256)
        {
            while (position < end)
            {
                SET_CHARACTER(p, ch->Code, params.Color);
                p++; ch++; position++;
            }
        }
        else {
            while (position < end)
            {
                SET_CHARACTER_EX(p, ch->Code, params.Color);
                p++; ch++; position++;
            }
        }
        if (params.Flags & WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY)
        {
            if ((params.HotKeyPosition < end) && (params.HotKeyPosition >= start))
            {
                ch = cb.GetBuffer() + params.HotKeyPosition;
                p = this->OffsetRows[y] + x + (params.HotKeyPosition - start);
                SET_CHARACTER_EX(p, ch->Code, params.HotKeyColor);
            }
        }
    }
    else {
        while (start < end)
        {
            SET_CHARACTER_EX(p, ch->Code, ch->Color);
            p++; ch++; start++;
        }
    }
    return true;
}
bool ConsoleRenderer::WriteCharacterBuffer_MultiLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end)
{
    NOT_IMPLEMENTED(false);
}
bool ConsoleRenderer::WriteCharacterBuffer_MultiLine_ProcessNewLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end)
{
    NOT_IMPLEMENTED(false);
}

bool ConsoleRenderer::WriteCharacterBuffer(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params)
{
    CHECK_VISIBLE;
    
    unsigned int start = 0;
    unsigned int end = cb.Len();
    if (params.Flags & WriteCharacterBufferFlags::BUFFER_RANGE)
    {
        CHECK(params.Start < end, false, "Start param (%d) should be smaller than buffer range: %d", params.Start, end);
        start = params.Start;
        if (end>params.End)
            end = params.End;
        CHECK(start < end, false, "Invalid start(%d) and end(%d) params for WriteCharacterBuffer. Start should be smaller than end.",start,end);        
    }

    // single line
    if (params.Flags & WriteCharacterBufferFlags::SINGLE_LINE)
        return WriteCharacterBuffer_SingleLine(x, y, cb, params, start, end);

    // multi line
    if (params.Flags & WriteCharacterBufferFlags::MULTIPLE_LINES)
    {
        // if new line will not be process and there is no width limit, than its like a single line
        if (!(params.Flags & (WriteCharacterBufferFlags::WRAP_TO_WIDTH| WriteCharacterBufferFlags::PROCESS_NEW_LINE)))
            return WriteCharacterBuffer_SingleLine(x, y, cb, params, start, end);
        if (params.Flags & WriteCharacterBufferFlags::PROCESS_NEW_LINE)
            return WriteCharacterBuffer_MultiLine_ProcessNewLine(x, y, cb, params, start, end);
        else
            return WriteCharacterBuffer_MultiLine(x, y, cb, params, start, end);
    }
    return true;
}