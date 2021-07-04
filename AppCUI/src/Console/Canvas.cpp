#include "AppCUI.h"
#include <string.h>

using namespace AppCUI::Console;


int _special_characters_consolas_unicode_[AppCUI::Console::SpecialChars::Count] = {
       0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551,                      // double line box
       0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502,                      // single line box
       0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                      // arrows
       32, 0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590,  // blocks
       0x25CF, 0x25CB, 0x221A,                                              // symbols
};
int* SpecialCharacters = nullptr;

#define CHECK_CANVAS_INITIALIZE CHECK(this->Characters,false,"Canvas object is not initialized. Have you called Cretae/Resize methods");
//#define SET_CHARACTER_VALUE(ptrCharInfo,value)	    { (ptrCharInfo)->Char.UnicodeChar = (value); }
//#define SET_CHARACTER_COLOR(ptrCharInfo,color)	    { (ptrCharInfo)->Attributes = (color); }
#define SET_CHARACTER(ptrCharInfo,value,color)      { (ptrCharInfo)->Code = (value);(ptrCharInfo)->Color = (color); }
#define SET_CHARACTER_EX(ptrCharInfo,value,color) {\
    if (value>=0) { ptrCharInfo->Code = (value); } \
    if (color<256) { \
        ptrCharInfo->Color = color; \
    } else { \
        if (color != AppCUI::Console::Color::NoColor) { \
            unsigned int temp_color = color; \
            if (color & 256) temp_color = (ptrCharInfo->Color & 0x0F)|(temp_color & 0xFFFFF0); \
            if (color & (256<<4)) temp_color = (ptrCharInfo->Color & 0xF0)|(temp_color & 0xFFFF0F); \
            ptrCharInfo->Color = temp_color; \
        } \
    } \
}


using namespace AppCUI::Console;

#define TRANSLATE_X_COORDONATE(x) x+=this->TranslateX;
#define TRANSLATE_Y_COORDONATE(y) y+=this->TranslateY;
#define TRANSLATE_COORDONATES(x,y) { x += this->TranslateX; y += this->TranslateY; }
#define CHECK_VISIBLE if (this->Clip.Visible == false) return false;

Canvas::Canvas()
{
    TranslateX = TranslateY = 0;
    Width = 0;
    Height = 0;
    Characters = nullptr;
    OffsetRows = nullptr;
    if (SpecialCharacters == nullptr)
        SpecialCharacters = _special_characters_consolas_unicode_;

    this->Clip.Visible = false;
    this->Cursor.Visible = false;
    this->Cursor.X = this->Cursor.Y = 0;
}
Canvas::~Canvas()
{
    Destroy();
}
bool Canvas::Create(unsigned int width, unsigned int height, int fillCharacter, unsigned int color)
{
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    if ((width == this->Width) && (height = this->Height))
    {
        // no re-allocation required
        CHECK(Clear(fillCharacter, color), false, "");
        return true;
    }
    Character * tmp = new Character[width*height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters", width, height);
    Destroy();
    this->Characters = tmp;
    this->Width = width;
    this->Height = height;
    CHECK(Clear(fillCharacter, color), false, "");

    return true;
}
bool Canvas::Resize(unsigned int width, unsigned int height, int fillCharacter, unsigned int color)
{
    if (this->Characters == nullptr)
        return Create(width, height, fillCharacter, color);
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    if ((width == this->Width) && (height = this->Height))
        return true; // nothing to resize
    Character * tmp = new Character[width*height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters", width, height);

    unsigned short col = color & 0xFF; // no transparency allowed
    unsigned short chr = ' ';

    if ((fillCharacter >= 0) && (fillCharacter <= 0xFFFF))
        chr = (unsigned short)(fillCharacter & 0xFFFF);
    // copy from Characters to tmp
    unsigned int min_w = MINVALUE(this->Width, width);
    unsigned int min_h = MINVALUE(this->Height, height);
    for (unsigned int y = 0; y < min_h; y++)
    {
        Character * p_temp = tmp + (y*width);
        Character * p_current = this->Characters + (y*this->Width);
        for (unsigned int x = 0; x < min_w; x++, p_temp++, p_current++)
        {
            *(p_temp) = *(p_current);
        }
    }
    // fill the rest
    if (min_w < width) {
        for (unsigned int y = 0; y < height; y++)
        {
            Character * p_temp = tmp + (y*width);
            for (unsigned int x = min_w; x < width; x++, p_temp++)
            {
                p_temp->Code = chr;
                p_temp->Color = col;
            }
        }
    }
    if (min_h < height)
    {
        for (unsigned int y = min_h; y < height; y++)
        {
            Character * p_temp = tmp + (y*width);
            for (unsigned int x = 0; x < min_w; x++, p_temp++)
            {
                p_temp->Code = chr;
                p_temp->Color = col;
            }
        }
    }
    // destroy old Characters buffer
    Destroy();
    this->Characters = tmp;
    this->Width = width;
    this->Height = height;

    return true;
}


void Canvas::Destroy()
{
    if (Characters)
        delete[] Characters;
    if (OffsetRows)
        delete[] OffsetRows;
    Characters = nullptr;
    OffsetRows = nullptr;
    Width = 0;
    Height = 0;
}


void Canvas::Reset()
{
    this->TranslateX = this->TranslateY = 0;
    this->Clip.Left = this->Clip.Right = 0;
    this->Clip.Right = this->Width - 1;
    this->Clip.Bottom = this->Height - 1;
    this->Clip.Visible = true;
    this->HideCursor();
}


void Canvas::DarkenScreen()
{
    Character * start = this->Characters;
    Character * end = this->Characters + (this->Width * this->Height);
    while (start < end) {
        start->Color = COLOR(Color::Gray, Color::Black);
        start++;
    }
}


void Canvas::SetClip(const AppCUI::Console::Clip & clip)
{
    if (clip.Visible)
    {
        // make sure that clipping coordonates are within screen coordonates
        this->Clip.Left = MAXVALUE(clip.ClipRect.X, 0);
        this->Clip.Top = MAXVALUE(clip.ClipRect.Y, 0);
        this->Clip.Right = clip.ClipRect.X + clip.ClipRect.Width - 1;
        this->Clip.Bottom = clip.ClipRect.Y + clip.ClipRect.Height - 1;
        if (this->Clip.Right >= (int)this->Width)
            this->Clip.Right = (int)this->Width - 1;
        if (this->Clip.Bottom >= (int)this->Height)
            this->Clip.Bottom = (int)this->Height - 1;
        this->Clip.Visible = (Clip.Left <= Clip.Right) && (Clip.Top <= Clip.Bottom);
    }
    else {
        this->Clip.Visible = false;
    }
}
void Canvas::ResetClip()
{
    this->Clip.Left = this->Clip.Right = 0;
    this->Clip.Right = this->Width - 1;
    this->Clip.Bottom = this->Height - 1;
    this->Clip.Visible = true;
}
void Canvas::SetTranslate(int offX, int offY)
{
    this->TranslateX = offX;
    this->TranslateY = offY;
}
void Canvas::HideCursor()
{
    this->Cursor.Visible = false;
}
bool Canvas::ShowCursor(int x, int y)
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
bool Canvas::WriteCharacter(int x, int y, int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);
    if ((x < Clip.Left) || (x > Clip.Right) || (y < Clip.Top) || (y > Clip.Bottom))
        return false;
    Character *p = this->OffsetRows[y] + x;
    if ((charCode > 0) && (color < 256))
    {
        SET_CHARACTER(p, charCode, color);
    }
    else {
        SET_CHARACTER_EX(p, charCode, color);
    }
    return true;
}
bool Canvas::Clear(unsigned int color)
{
    return Clear(' ', color);
}
bool Canvas::Clear(int character, unsigned int color)
{
    CHECK_CANVAS_INITIALIZE;
    Character * s = this->Characters;
    Character * e = s + (this->Width* this->Height);
    Character tmp;
    tmp.Color = color & 0xFF; // no transparency allowed
    tmp.Code = 32;
    if ((character >= 0) && (character <= 0xFFFF))
        tmp.Code = (unsigned short)(character & 0xFFFF);
    while (s < e) {
        s->PackedValue = tmp.PackedValue;
        s++;
    }
    return true;
}
bool Canvas::ClearClipRectangle(int charCode, unsigned int color)
{
    CHECK_VISIBLE;
    if ((Clip.Left == 0) && (Clip.Top == 0) && (Clip.Right + 1 == this->Width) && (Clip.Bottom + 1 == this->Height))
    {
        //optimize for the entire screen
        Character * start = this->Characters;
        Character * end = this->Characters + (this->Width * this->Height);
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
        return FillRect(Clip.Left - TranslateX, Clip.Top - TranslateY, Clip.Right - TranslateX, Clip.Bottom - TranslateY, charCode, color);
    }
}
bool Canvas::FillHorizontalLine(int left, int y, int right, int charCode, unsigned int color)
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
    Character * p = this->OffsetRows[y] + left;
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
bool Canvas::FillVerticalLine(int x, int top, int bottom, int charCode, unsigned int color)
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
    Character * p = this->OffsetRows[top] + x;
    if ((charCode >= 0) && (color < 256))
    {
        while (top <= bottom)
        {
            SET_CHARACTER(p, charCode, color);
            top++;
            p += this->Width;
        }
    }
    else {
        while (top <= bottom)
        {
            SET_CHARACTER_EX(p, charCode, color);
            top++;
            p += this->Width;
        }
    }
    return true;
}
bool Canvas::FillRect(int left, int top, int right, int bottom, int charCode, unsigned int color)
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
    Character *p;
    Character **row = this->OffsetRows;
    row += top;
    if ((charCode >= 0) && (color < 256))
    {
        for (int y = top; y <= bottom; y++, row++)
        {
            p = (*row) + left;
            for (int x = left; x <= right; x++, p++)
            {
                SET_CHARACTER(p, charCode, color);
            }
        }
    }
    else {
        for (int y = top; y <= bottom; y++, row++)
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
bool Canvas::DrawRect(int left, int top, int right, int bottom, unsigned int color, bool doubleLine)
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

    Character *p, *e;
    int char_to_draw;
    // top line
    if (top == orig_top)
    {
        p = this->OffsetRows[top] + left;
        e = this->OffsetRows[top] + right;
        if (doubleLine)
            char_to_draw = SpecialCharacters[SpecialChars::BoxHorizontalDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxHorizontalSingleLine];
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
            char_to_draw = SpecialCharacters[SpecialChars::BoxHorizontalDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxHorizontalSingleLine];
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
            char_to_draw = SpecialCharacters[SpecialChars::BoxVerticalDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxVerticalSingleLine];
        if (color < 256)
        {
            while (p <= e) { SET_CHARACTER(p, char_to_draw, color); p += Width; }
        }
        else {
            while (p <= e) { SET_CHARACTER_EX(p, char_to_draw, color); p += Width; }
        }
    }
    // right line
    if (right == orig_right)
    {
        p = this->OffsetRows[top] + right;
        e = this->OffsetRows[bottom] + right;
        if (doubleLine)
            char_to_draw = SpecialCharacters[SpecialChars::BoxVerticalDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxVerticalSingleLine];
        if (color < 256)
        {
            while (p <= e) { SET_CHARACTER(p, char_to_draw, color); p += Width; }
        }
        else {
            while (p <= e) { SET_CHARACTER_EX(p, char_to_draw, color); p += Width; }
        }
    }
    // corners
    if ((left == orig_left) && (top == orig_top))
    {
        if (doubleLine)
            char_to_draw = SpecialCharacters[SpecialChars::BoxTopLeftCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxTopLeftCornerSingleLine];
        p = this->OffsetRows[top] + left;
        if (color < 256) {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((left == orig_left) && (bottom == orig_bottom))
    {
        if (doubleLine)
            char_to_draw = SpecialCharacters[SpecialChars::BoxBottomLeftCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxBottomLeftCornerSingleLine];
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
            char_to_draw = SpecialCharacters[SpecialChars::BoxBottomRightCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxBottomRightCornerSingleLine];
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
            char_to_draw = SpecialCharacters[SpecialChars::BoxTopRightCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[SpecialChars::BoxTopRightCornerSingleLine];
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
bool Canvas::WriteSingleLineText(int x, int y, const char * text, unsigned int color, int textSize)
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
    Character * c = this->OffsetRows[y] + x;
    while ((s < e) && (x <= Clip.Right))
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
bool Canvas::WriteSingleLineTextWithHotKey(int x, int y, const char * text, unsigned int color, unsigned int hotKeyColor, int textSize)
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
    Character * c = this->OffsetRows[y] + x;
    Character * hotkey = nullptr;
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
bool Canvas::WriteMultiLineText(int x, int y, const char * text, unsigned int color, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    const unsigned char * s = (const unsigned char *)text;
    const unsigned char * e = s + textSize;
    Character * c = this->OffsetRows[y] + x;
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
bool Canvas::WriteMultiLineTextWithHotKey(int x, int y, const char * text, unsigned int color, unsigned int hotKeyColor, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    const unsigned char * s = (const unsigned char *)text;
    const unsigned char * e = s + textSize;
    Character * c = this->OffsetRows[y] + x;
    Character * hotkey = nullptr;
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
bool Canvas::WriteCharacterBuffer_SingleLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end)
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
    Character * p = this->OffsetRows[y] + x;
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
bool Canvas::WriteCharacterBuffer_MultiLine_WithWidth(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end)
{
    TRANSLATE_COORDONATES(x, y);
    if (x > Clip.Right)
        return false;
    if ((params.Width == 0) || (end == start))
        return true; // nothing to draw
    unsigned int nrLines = (end - start - 1) / params.Width;
    if ((y + (int)nrLines < Clip.Top) || (y > Clip.Bottom))
        return false;
    const AppCUI::Console::Character * ch = cb.GetBuffer() + start;
    Character * p = this->OffsetRows[y] + x;
    unsigned int rel_ofs = params.Width;
    int original_x = x;
    //GDT: not efficient - further improvements can be done
    if (params.Flags & WriteCharacterBufferFlags::OVERWRITE_COLORS)
    {
        unsigned int orig_start = start;
        while (start < end)
        {
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top)) {
                SET_CHARACTER_EX(p, ch->Code, params.Color);
            }
            p++; ch++; rel_ofs--; start++; x++;
            if (rel_ofs == 0) {
                rel_ofs = params.Width;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
            }
        }
        if (params.Flags & WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY)
        {
            if ((params.HotKeyPosition < end) && (params.HotKeyPosition >= orig_start))
            {
                ch = cb.GetBuffer() + params.HotKeyPosition;
                p = this->OffsetRows[(params.HotKeyPosition - orig_start) / params.Width] + x + ((params.HotKeyPosition - orig_start) % params.Width);
                SET_CHARACTER_EX(p, ch->Code, params.HotKeyColor);
            }
        }
    }
    else {
        while (start < end)
        {
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top)) {
                SET_CHARACTER_EX(p, ch->Code, ch->Color);
            }
            p++; ch++; rel_ofs--; start++; x++;
            if (rel_ofs == 0) {
                rel_ofs = params.Width;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
            }
        }
    }
    return true;
}
bool Canvas::WriteCharacterBuffer_MultiLine_ProcessNewLine(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params, unsigned int start, unsigned int end)
{
    TRANSLATE_COORDONATES(x, y);
    if (x > Clip.Right)
        return false;
    if ((params.Width == 0) || (end == start))
        return true; // nothing to draw
    unsigned int w;
    if (params.Flags & WriteCharacterBufferFlags::WRAP_TO_WIDTH)
        w = params.Width;
    else
        w = 0xFFFFFFFF;
    const AppCUI::Console::Character * ch = cb.GetBuffer() + start;
    Character * p = this->OffsetRows[y] + x;
    Character * hotkey = nullptr;
    unsigned int rel_ofs = w;
    int original_x = x;
    bool has_hotkey = (params.Flags & WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY) != 0;
    //GDT: not efficient - further improvements can be done
    if (params.Flags & WriteCharacterBufferFlags::OVERWRITE_COLORS)
    {
        unsigned int orig_start = start;
        while (start < end)
        {
            if ((rel_ofs == 0) || (ch->Code == '\n')) {
                rel_ofs = w;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
                if (ch->Code == '\n')
                {
                    ch++; start++;
                    continue;
                }
            }
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top)) {
                SET_CHARACTER_EX(p, ch->Code, params.Color);
                if ((has_hotkey) && (!hotkey) && (start == params.HotKeyPosition))
                    hotkey = p;
            }
            p++; ch++; rel_ofs--; start++; x++;
        }
        if (hotkey) {
            SET_CHARACTER_EX(hotkey, -1, params.HotKeyColor);
        }
    }
    else {
        while (start < end)
        {
            if ((rel_ofs == 0) || (ch->Code == '\n')) {
                rel_ofs = w;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
                if (ch->Code == '\n')
                {
                    ch++; start++;
                    continue;
                }
            }
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top)) {
                SET_CHARACTER_EX(p, ch->Code, ch->Color);
            }
            p++; ch++; rel_ofs--; start++; x++;
        }
    }
    return true;
}

bool Canvas::WriteCharacterBuffer(int x, int y, const AppCUI::Console::CharacterBuffer & cb, const AppCUI::Console::WriteCharacterBufferParams& params)
{
    CHECK_VISIBLE;

    unsigned int start = 0;
    unsigned int end = cb.Len();
    if (params.Flags & WriteCharacterBufferFlags::BUFFER_RANGE)
    {
        CHECK(params.Start < end, false, "Start param (%d) should be smaller than buffer range: %d", params.Start, end);
        start = params.Start;
        if (end > params.End)
            end = params.End;
        CHECK(start < end, false, "Invalid start(%d) and end(%d) params for WriteCharacterBuffer. Start should be smaller than end.", start, end);
    }
    if (end <= start)
        return true; // Nothing to draw
    // single line
    if (params.Flags & WriteCharacterBufferFlags::SINGLE_LINE)
        return WriteCharacterBuffer_SingleLine(x, y, cb, params, start, end);

    // multi line
    if (params.Flags & WriteCharacterBufferFlags::MULTIPLE_LINES)
    {
        // if new line will not be process and there is no width limit, than its like a single line
        if (!(params.Flags & (WriteCharacterBufferFlags::WRAP_TO_WIDTH | WriteCharacterBufferFlags::PROCESS_NEW_LINE)))
            return WriteCharacterBuffer_SingleLine(x, y, cb, params, start, end);
        if (params.Flags & WriteCharacterBufferFlags::PROCESS_NEW_LINE)
            return WriteCharacterBuffer_MultiLine_ProcessNewLine(x, y, cb, params, start, end);
        else
            // implies that WriteCharacterBufferFlags::WRAP_TO_WIDTH is set
            return WriteCharacterBuffer_MultiLine_WithWidth(x, y, cb, params, start, end);
    }
    return true;
}
