#include "AppCUI.hpp"
#include <string.h>

using namespace AppCUI::Graphics;
using namespace AppCUI::Utils;

int _special_characters_consolas_unicode_[(unsigned int) AppCUI::Graphics::SpecialChars::Count] = {
    0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551, 0x256C,                         // double line box
    0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502, 0x253C,                         // single line box
    0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                                 // arrows
    32,     0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590, 0x25A0, // blocks
    0x25B2, 0x25BC, 0x25C4, 0x25BA,                                                 // Trangles
    0x25CF, 0x25CB, 0x221A,                                                         // symbols
};
int* SpecialCharacters = nullptr;

#define CHECK_CANVAS_INITIALIZE                                                                                        \
    CHECK(this->Characters, false, "Canvas object is not initialized. Have you called Cretae/Resize methods");
#define SET_CHARACTER(ptrCharInfo, value, color)                                                                       \
    {                                                                                                                  \
        (ptrCharInfo)->Code  = (value);                                                                                \
        (ptrCharInfo)->Color = (color);                                                                                \
    }
#define SET_CHARACTER_EX(ptrCharInfo, value, color)                                                                    \
    {                                                                                                                  \
        if (value >= 0)                                                                                                \
        {                                                                                                              \
            ptrCharInfo->Code = (value);                                                                               \
        }                                                                                                              \
        if (color.Foreground != AppCUI::Graphics::Color::Transparent)                                                  \
        {                                                                                                              \
            ptrCharInfo->Color.Foreground = color.Foreground;                                                          \
        }                                                                                                              \
        if (color.Background != AppCUI::Graphics::Color::Transparent)                                                  \
        {                                                                                                              \
            ptrCharInfo->Color.Background = color.Background;                                                          \
        }                                                                                                              \
    }
#define NO_TRANSPARENCY(color)                                                                                         \
    ((color.Foreground != AppCUI::Graphics::Color::Transparent) &&                                                     \
     (color.Background != AppCUI::Graphics::Color::Transparent))

using namespace AppCUI::Graphics;

#define TRANSLATE_X_COORDONATE(x) x += this->TranslateX;
#define TRANSLATE_Y_COORDONATE(y) y += this->TranslateY;
#define TRANSLATE_COORDONATES(x, y)                                                                                    \
    {                                                                                                                  \
        x += this->TranslateX;                                                                                         \
        y += this->TranslateY;                                                                                         \
    }
#define CHECK_VISIBLE                                                                                                  \
    if (this->Clip.Visible == false)                                                                                   \
        return false;

Renderer::Renderer()
{
    TranslateX = TranslateY = 0;
    Width                   = 0;
    Height                  = 0;
    Characters              = nullptr;
    OffsetRows              = nullptr;
    if (SpecialCharacters == nullptr)
        SpecialCharacters = _special_characters_consolas_unicode_;

    this->Clip.Visible   = false;
    this->Cursor.Visible = false;
    this->Cursor.X = this->Cursor.Y = 0;
    this->ClipCopy.Bottom = this->ClipCopy.Top = this->ClipCopy.Left = this->ClipCopy.Right = -1;
    this->ClipCopy.Visible                                                                  = false;
    this->ClipHasBeenCopied                                                                 = false;
}
Renderer::~Renderer()
{
    _Destroy();
}

void Renderer::_Destroy()
{
    if (Characters)
        delete[] Characters;
    if (OffsetRows)
        delete[] OffsetRows;
    Characters = nullptr;
    OffsetRows = nullptr;
    Width      = 0;
    Height     = 0;
}

void Renderer::HideCursor()
{
    this->Cursor.Visible = false;
}
bool Renderer::SetCursor(int x, int y)
{
    TRANSLATE_COORDONATES(x, y);
    if ((x < Clip.Left) || (x > Clip.Right) || (y < Clip.Top) || (y > Clip.Bottom))
    {
        this->Cursor.Visible = false;
        return false;
    }
    this->Cursor.X       = (unsigned int) x;
    this->Cursor.Y       = (unsigned int) y;
    this->Cursor.Visible = true;
    return true;
}
bool Renderer::WriteCharacter(int x, int y, int charCode, const ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);
    if ((x < Clip.Left) || (x > Clip.Right) || (y < Clip.Top) || (y > Clip.Bottom))
        return false;
    Character* p = this->OffsetRows[y] + x;
    if ((charCode > 0) && (NO_TRANSPARENCY(color)))
    {
        SET_CHARACTER(p, charCode, color);
    }
    else
    {
        SET_CHARACTER_EX(p, charCode, color);
    }
    return true;
}
bool Renderer::WriteSpecialCharacter(int x, int y, SpecialChars charID, const ColorPair color)
{
    return WriteCharacter(x, y, SpecialCharacters[(unsigned int) charID], color);
}

bool Renderer::_ClearEntireSurface(int character, const ColorPair color)
{
    CHECK_CANVAS_INITIALIZE;
    Character* s = this->Characters;
    Character* e = s + (this->Width * this->Height);
    Character tmp;
    tmp.Color = DefaultColorPair;
    if (color.Background != Color::Transparent)
        tmp.Color.Background = color.Background;
    if (color.Foreground != Color::Transparent)
        tmp.Color.Foreground = color.Foreground;
    tmp.Code = 32;
    if ((character >= 0) && (character <= 0xFFFF))
        tmp.Code = (unsigned short) (character & 0xFFFF);
    while (s < e)
    {
        s->PackedValue = tmp.PackedValue;
        s++;
    }
    return true;
}
bool Renderer::Clear(int charCode, const ColorPair color)
{
    CHECK_VISIBLE;
    if ((Clip.Left == 0) && (Clip.Top == 0) && (Clip.Right + 1 == this->Width) && (Clip.Bottom + 1 == this->Height))
    {
        return _ClearEntireSurface(charCode, color);
    }
    else
    {
        return FillRect(
              Clip.Left - TranslateX,
              Clip.Top - TranslateY,
              Clip.Right - TranslateX,
              Clip.Bottom - TranslateY,
              charCode,
              color);
    }
}
bool Renderer::ClearWithSpecialChar(SpecialChars charID, const ColorPair color)
{
    return Clear(SpecialCharacters[(unsigned int) charID], color);
}
bool Renderer::DrawHorizontalLine(int left, int y, int right, int charCode, const ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, y);
    TRANSLATE_X_COORDONATE(right);
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    left  = MAXVALUE(left, Clip.Left);
    right = MINVALUE(right, Clip.Right);
    if (left > right)
        return false;
    Character* p = this->OffsetRows[y] + left;
    if ((charCode >= 0) && (NO_TRANSPARENCY(color)))
    {
        while (left <= right)
        {
            SET_CHARACTER(p, charCode, color);
            left++;
            p++;
        }
    }
    else
    {
        while (left <= right)
        {
            SET_CHARACTER_EX(p, charCode, color);
            left++;
            p++;
        }
    }
    return true;
}
bool Renderer::DrawHorizontalLineWithSpecialChar(int left, int y, int right, SpecialChars charID, const ColorPair color)
{
    return DrawHorizontalLine(left, y, right, SpecialCharacters[(unsigned int) charID], color);
}
bool Renderer::DrawHorizontalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color)
{
    CHECK(size > 0, false, "");
    return DrawHorizontalLine(x, y, x + ((int) size) - 1, charCode, color);
}
bool Renderer::DrawVerticalLine(int x, int top, int bottom, int charCode, const ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, top);
    TRANSLATE_Y_COORDONATE(bottom);
    if ((x < Clip.Left) || (x > Clip.Right))
        return false;
    top    = MAXVALUE(top, Clip.Top);
    bottom = MINVALUE(bottom, Clip.Bottom);
    if (top > bottom)
        return false;
    Character* p = this->OffsetRows[top] + x;
    if ((charCode >= 0) && (NO_TRANSPARENCY(color)))
    {
        while (top <= bottom)
        {
            SET_CHARACTER(p, charCode, color);
            top++;
            p += this->Width;
        }
    }
    else
    {
        while (top <= bottom)
        {
            SET_CHARACTER_EX(p, charCode, color);
            top++;
            p += this->Width;
        }
    }
    return true;
}
bool Renderer::DrawVerticalLineSize(int x, int y, unsigned int size, int charCode, const ColorPair color)
{
    CHECK(size > 0, false, "");
    return DrawVerticalLine(x, y, y + ((int) size) - 1, charCode, color);
}
bool Renderer::DrawVerticalLineWithSpecialChar(int x, int top, int bottom, SpecialChars charID, const ColorPair color)
{
    return DrawVerticalLine(x, top, bottom, SpecialCharacters[(unsigned int) charID], color);
}
bool Renderer::FillRect(int left, int top, int right, int bottom, int charCode, const ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, top);
    TRANSLATE_COORDONATES(right, bottom);

    left   = MAXVALUE(left, Clip.Left);
    top    = MAXVALUE(top, Clip.Top);
    right  = MINVALUE(right, Clip.Right);
    bottom = MINVALUE(bottom, Clip.Bottom);

    if ((left > right) || (top > bottom))
        return false;
    Character* p;
    Character** row = this->OffsetRows;
    row += top;
    if ((charCode >= 0) && (NO_TRANSPARENCY(color)))
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
    else
    {
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
bool Renderer::DrawRect(int left, int top, int right, int bottom, const ColorPair color, bool doubleLine)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, top);
    TRANSLATE_COORDONATES(right, bottom);

    int orig_left   = left;
    int orig_right  = right;
    int orig_top    = top;
    int orig_bottom = bottom;

    left   = MAXVALUE(left, Clip.Left);
    top    = MAXVALUE(top, Clip.Top);
    right  = MINVALUE(right, Clip.Right);
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
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxHorizontalDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxHorizontalSingleLine];
        if (NO_TRANSPARENCY(color))
        {
            while (p <= e)
            {
                SET_CHARACTER(p, char_to_draw, color);
                p++;
            }
        }
        else
        {
            while (p <= e)
            {
                SET_CHARACTER_EX(p, char_to_draw, color);
                p++;
            }
        }
    }
    // bottom line
    if (bottom == orig_bottom)
    {
        p = this->OffsetRows[bottom] + left;
        e = this->OffsetRows[bottom] + right;
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxHorizontalDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxHorizontalSingleLine];
        if (NO_TRANSPARENCY(color))
        {
            while (p <= e)
            {
                SET_CHARACTER(p, char_to_draw, color);
                p++;
            }
        }
        else
        {
            while (p <= e)
            {
                SET_CHARACTER_EX(p, char_to_draw, color);
                p++;
            }
        }
    }
    // left line
    if (left == orig_left)
    {
        p = this->OffsetRows[top] + left;
        e = this->OffsetRows[bottom] + left;
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxVerticalDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxVerticalSingleLine];
        if (NO_TRANSPARENCY(color))
        {
            while (p <= e)
            {
                SET_CHARACTER(p, char_to_draw, color);
                p += Width;
            }
        }
        else
        {
            while (p <= e)
            {
                SET_CHARACTER_EX(p, char_to_draw, color);
                p += Width;
            }
        }
    }
    // right line
    if (right == orig_right)
    {
        p = this->OffsetRows[top] + right;
        e = this->OffsetRows[bottom] + right;
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxVerticalDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxVerticalSingleLine];
        if (NO_TRANSPARENCY(color))
        {
            while (p <= e)
            {
                SET_CHARACTER(p, char_to_draw, color);
                p += Width;
            }
        }
        else
        {
            while (p <= e)
            {
                SET_CHARACTER_EX(p, char_to_draw, color);
                p += Width;
            }
        }
    }
    // corners
    if ((left == orig_left) && (top == orig_top))
    {
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxTopLeftCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxTopLeftCornerSingleLine];
        p = this->OffsetRows[top] + left;
        if (NO_TRANSPARENCY(color))
        {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else
        {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((left == orig_left) && (bottom == orig_bottom))
    {
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxBottomLeftCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxBottomLeftCornerSingleLine];
        p = this->OffsetRows[bottom] + left;
        if (NO_TRANSPARENCY(color))
        {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else
        {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((right == orig_right) && (bottom == orig_bottom))
    {
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxBottomRightCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxBottomRightCornerSingleLine];
        p = this->OffsetRows[bottom] + right;
        if (NO_TRANSPARENCY(color))
        {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else
        {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    if ((right == orig_right) && (top == orig_top))
    {
        if (doubleLine)
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxTopRightCornerDoubleLine];
        else
            char_to_draw = SpecialCharacters[(unsigned int) SpecialChars::BoxTopRightCornerSingleLine];
        p = this->OffsetRows[top] + right;
        if (NO_TRANSPARENCY(color))
        {
            SET_CHARACTER(p, char_to_draw, color);
        }
        else
        {
            SET_CHARACTER_EX(p, char_to_draw, color);
        }
    }
    return true;
}
bool Renderer::FillRectSize(int x, int y, unsigned int width, unsigned int height, int charCode, const ColorPair color)
{
    CHECK(((width > 0) && (height > 0)), false, "");
    return FillRect(x, y, x + ((int) width) - 1, y + ((int) height) - 1, charCode, color);
}
bool Renderer::DrawRectSize(
      int x, int y, unsigned int width, unsigned int height, const ColorPair color, bool doubleLine)
{
    CHECK(((width > 0) && (height > 0)), false, "");
    return DrawRect(x, y, x + ((int) width) - 1, y + ((int) height) - 1, color, doubleLine);
}
bool Renderer::WriteSingleLineText(int x, int y, const char* text, const ColorPair color, int textSize)
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
    const unsigned char* s = (const unsigned char*) text;
    const unsigned char* e = s + textSize;
    // needs more optimizations
    Character* c = this->OffsetRows[y] + x;
    if (NO_TRANSPARENCY(color))
    {
        while ((s < e) && (x <= Clip.Right))
        {
            if (x >= Clip.Left)
            {
                SET_CHARACTER(c, *s, color);
            }
            c++;
            s++;
            x++;
        }
    }
    else
    {
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
    }
    return true;
}
bool Renderer::WriteSingleLineText(
      int x, int y, const char* text, unsigned int width, const ColorPair color, TextAlignament align, int textSize)
{
    if (width == 0)
        return false; // nothing to print
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    if (x > Clip.Right)
        return false;
    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);

    if (x + (int) width < Clip.Left)
        return false;
    const unsigned char* s;
    const unsigned char* e;
    switch ((TextAlignament)(
          ((unsigned int) align) &
          (unsigned int) (TextAlignament::Left | TextAlignament::Right | TextAlignament::Center)))
    {
    case TextAlignament::Left:
        s = (const unsigned char*) text;
        e = s + MINVALUE((unsigned int) textSize, width);
        break;
    case TextAlignament::Right:
        e = (const unsigned char*) text + ((unsigned int) textSize);
        s = e - MINVALUE((unsigned int) textSize, width);
        x += width;
        x -= MINVALUE((unsigned int) textSize, width);
        break;
    case TextAlignament::Center:
        s = (const unsigned char*) text;
        e = s + MINVALUE((unsigned int) textSize, width);
        if ((unsigned int) textSize < width)
            x += ((width - (unsigned int) textSize) >> 1);
        break;
    default:
        RETURNERROR(false, "Invalid text align method (it can only be Left,Righ or Center)");
    }
    Character* c = this->OffsetRows[y] + x;
    if (NO_TRANSPARENCY(color))
    {
        while ((s < e) && (x <= Clip.Right))
        {
            if (x >= Clip.Left)
            {
                SET_CHARACTER(c, *s, color);
            }
            c++;
            s++;
            x++;
        }
    }
    else
    {
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
    }
    return true;
}
bool Renderer::WriteSingleLineTextWithHotKey(
      int x,
      int y,
      const char* text,
      unsigned int width,
      const ColorPair color,
      const ColorPair hotKeyColor,
      unsigned int hotKeyOffset,
      TextAlignament align,
      int textSize)
{
    if (width == 0)
        return false; // nothing to print
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    if (x > Clip.Right)
        return false;
    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);

    if (x + (int) width < Clip.Left)
        return false;
    const unsigned char* s;
    const unsigned char* e;
    int hotKeyX;
    switch ((TextAlignament)(
          ((unsigned int) align) &
          (unsigned int) (TextAlignament::Left | TextAlignament::Right | TextAlignament::Center)))
    {
    case TextAlignament::Left:
        s       = (const unsigned char*) text;
        e       = s + MINVALUE((unsigned int) textSize, width);
        hotKeyX = x + hotKeyOffset;
        break;
    case TextAlignament::Right:
        e = (const unsigned char*) text + ((unsigned int) textSize);
        s = e - MINVALUE((unsigned int) textSize, width);
        x += width;
        x -= MINVALUE((unsigned int) textSize, width);
        hotKeyX = x + hotKeyOffset - (int) (s - (const unsigned char*) text);
        break;
    case TextAlignament::Center:
        s = (const unsigned char*) text;
        e = s + MINVALUE((unsigned int) textSize, width);
        if ((unsigned int) textSize < width)
            x += ((width - (unsigned int) textSize) >> 1);
        hotKeyX = x + hotKeyOffset;
        break;
    default:
        RETURNERROR(false, "Invalid text align method (it can only be Left,Righ or Center)");
    }

    int startX   = x;
    Character* c = this->OffsetRows[y] + x;
    if (NO_TRANSPARENCY(color))
    {
        while ((s < e) && (x <= Clip.Right))
        {
            if (x >= Clip.Left)
            {
                SET_CHARACTER(c, *s, color);
            }
            c++;
            s++;
            x++;
        }
    }
    else
    {
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
    }
    // hotkey
    if ((hotKeyX >= Clip.Left) && (hotKeyX <= Clip.Right) && (hotKeyX >= startX))
    {
        c = this->OffsetRows[y] + hotKeyX;
        SET_CHARACTER_EX(c, -1, hotKeyColor);
    }
    return true;
}

bool Renderer::WriteSingleLineTextWithHotKey(
      int x, int y, const char* text, const ColorPair color, const ColorPair hotKeyColor, int textSize)
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
    const unsigned char* s = (const unsigned char*) text;
    const unsigned char* e = s + textSize;
    // needs more optimizations
    Character* c      = this->OffsetRows[y] + x;
    Character* hotkey = nullptr;
    while ((s < e) && (x <= Clip.Right))
    {
        if (x >= Clip.Left)
        {
            if ((!hotkey) && ((*s) == '&'))
            {
                hotkey = c;
                x--;
                c--;
            }
            else
            {
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
bool Renderer::WriteMultiLineText(int x, int y, const char* text, const ColorPair color, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    const unsigned char* s = (const unsigned char*) text;
    const unsigned char* e = s + textSize;
    Character* c           = this->OffsetRows[y] + x;
    int orig_x             = x;
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
        else
        {
            y++;
            x = orig_x;
            c = this->OffsetRows[y] + x;
        }
        s++;
    }
    return true;
}
bool Renderer::WriteMultiLineTextWithHotKey(
      int x, int y, const char* text, const ColorPair color, const ColorPair hotKeyColor, int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text ");
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if (textSize < 0)
        textSize = AppCUI::Utils::String::Len(text);
    const unsigned char* s = (const unsigned char*) text;
    const unsigned char* e = s + textSize;
    Character* c           = this->OffsetRows[y] + x;
    Character* hotkey      = nullptr;
    int orig_x             = x;
    while ((s < e) && (y <= Clip.Bottom))
    {
        if ((*s) != '\n')
        {
            if ((x >= Clip.Left) && (x <= Clip.Right))
            {
                if ((!hotkey) && ((*s) == '&'))
                {
                    hotkey = c;
                    x--;
                    c--;
                }
                else
                {
                    SET_CHARACTER_EX(c, *s, color);
                }
            }
            c++;
            x++;
        }
        else
        {
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
bool Renderer::_WriteCharacterBuffer_SingleLine(
      int x,
      int y,
      const AppCUI::Graphics::CharacterBuffer& cb,
      const AppCUI::Graphics::WriteCharacterBufferParams& params,
      unsigned int start,
      unsigned int end)
{
    TRANSLATE_COORDONATES(x, y);
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    if (x > Clip.Right)
        return false;
    if (x < Clip.Left)
    {
        start += (unsigned int) (Clip.Left - x);
        x = Clip.Left;
        if (start >= end)
            return false;
    }
    if ((Clip.Right + 1 - x) <= (int) (end - start))
        end = start + (unsigned int) (Clip.Right + 1 - x);
    if ((params.Flags & WriteCharacterBufferFlags::WRAP_TO_WIDTH) != WriteCharacterBufferFlags::NONE)
    {
        if ((end - start) > params.Width)
            end = start + params.Width;
    }
    const AppCUI::Graphics::Character* ch = cb.GetBuffer() + start;
    Character* p                         = this->OffsetRows[y] + x;
    if ((params.Flags & WriteCharacterBufferFlags::OVERWRITE_COLORS) != WriteCharacterBufferFlags::NONE)
    {
        unsigned int position = start;
        if (NO_TRANSPARENCY(params.Color))
        {
            while (position < end)
            {
                SET_CHARACTER(p, ch->Code, params.Color);
                p++;
                ch++;
                position++;
            }
        }
        else
        {
            while (position < end)
            {
                SET_CHARACTER_EX(p, ch->Code, params.Color);
                p++;
                ch++;
                position++;
            }
        }
        if ((params.Flags & WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY) != WriteCharacterBufferFlags::NONE)
        {
            if ((params.HotKeyPosition < end) && (params.HotKeyPosition >= start))
            {
                ch = cb.GetBuffer() + params.HotKeyPosition;
                p  = this->OffsetRows[y] + x + (params.HotKeyPosition - start);
                SET_CHARACTER_EX(p, ch->Code, params.HotKeyColor);
            }
        }
    }
    else
    {
        while (start < end)
        {
            SET_CHARACTER_EX(p, ch->Code, ch->Color);
            p++;
            ch++;
            start++;
        }
    }
    return true;
}
bool Renderer::_WriteCharacterBuffer_MultiLine_WithWidth(
      int x,
      int y,
      const AppCUI::Graphics::CharacterBuffer& cb,
      const AppCUI::Graphics::WriteCharacterBufferParams& params,
      unsigned int start,
      unsigned int end)
{
    TRANSLATE_COORDONATES(x, y);
    if (x > Clip.Right)
        return false;
    if ((params.Width == 0) || (end == start))
        return true; // nothing to draw
    unsigned int nrLines = (end - start - 1) / params.Width;
    if ((y + (int) nrLines < Clip.Top) || (y > Clip.Bottom))
        return false;
    const AppCUI::Graphics::Character* ch = cb.GetBuffer() + start;
    Character* p                         = this->OffsetRows[y] + x;
    unsigned int rel_ofs                 = params.Width;
    int original_x                       = x;
    // GDT: not efficient - further improvements can be done
    if ((params.Flags & WriteCharacterBufferFlags::OVERWRITE_COLORS) != WriteCharacterBufferFlags::NONE)
    {
        unsigned int orig_start = start;
        while (start < end)
        {
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top))
            {
                SET_CHARACTER_EX(p, ch->Code, params.Color);
            }
            p++;
            ch++;
            rel_ofs--;
            start++;
            x++;
            if (rel_ofs == 0)
            {
                rel_ofs = params.Width;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
            }
        }
        if ((params.Flags & WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY) != WriteCharacterBufferFlags::NONE)
        {
            if ((params.HotKeyPosition < end) && (params.HotKeyPosition >= orig_start))
            {
                ch = cb.GetBuffer() + params.HotKeyPosition;
                p  = this->OffsetRows[(params.HotKeyPosition - orig_start) / params.Width] + x +
                    ((params.HotKeyPosition - orig_start) % params.Width);
                SET_CHARACTER_EX(p, ch->Code, params.HotKeyColor);
            }
        }
    }
    else
    {
        while (start < end)
        {
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top))
            {
                SET_CHARACTER_EX(p, ch->Code, ch->Color);
            }
            p++;
            ch++;
            rel_ofs--;
            start++;
            x++;
            if (rel_ofs == 0)
            {
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
bool Renderer::_WriteCharacterBuffer_MultiLine_ProcessNewLine(
      int x,
      int y,
      const AppCUI::Graphics::CharacterBuffer& cb,
      const AppCUI::Graphics::WriteCharacterBufferParams& params,
      unsigned int start,
      unsigned int end)
{
    TRANSLATE_COORDONATES(x, y);
    if (x > Clip.Right)
        return false;
    if ((params.Width == 0) || (end == start))
        return true; // nothing to draw
    unsigned int w;
    if ((params.Flags & WriteCharacterBufferFlags::WRAP_TO_WIDTH) != WriteCharacterBufferFlags::NONE)
        w = params.Width;
    else
        w = 0xFFFFFFFF;
    const AppCUI::Graphics::Character* ch = cb.GetBuffer() + start;
    Character* p                         = this->OffsetRows[y] + x;
    Character* hotkey                    = nullptr;
    unsigned int rel_ofs                 = w;
    int original_x                       = x;
    bool has_hotkey = (params.Flags & WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY) != WriteCharacterBufferFlags::NONE;
    // GDT: not efficient - further improvements can be done
    if ((params.Flags & WriteCharacterBufferFlags::OVERWRITE_COLORS) != WriteCharacterBufferFlags::NONE)
    {
        unsigned int orig_start = start;
        while (start < end)
        {
            if ((rel_ofs == 0) || (ch->Code == '\n'))
            {
                rel_ofs = w;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
                if (ch->Code == '\n')
                {
                    ch++;
                    start++;
                    continue;
                }
            }
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top))
            {
                SET_CHARACTER_EX(p, ch->Code, params.Color);
                if ((has_hotkey) && (!hotkey) && (start == params.HotKeyPosition))
                    hotkey = p;
            }
            p++;
            ch++;
            rel_ofs--;
            start++;
            x++;
        }
        if (hotkey)
        {
            SET_CHARACTER_EX(hotkey, -1, params.HotKeyColor);
        }
    }
    else
    {
        while (start < end)
        {
            if ((rel_ofs == 0) || (ch->Code == '\n'))
            {
                rel_ofs = w;
                y++;
                if (y > Clip.Bottom)
                    break;
                x = original_x;
                p = this->OffsetRows[y] + x;
                if (ch->Code == '\n')
                {
                    ch++;
                    start++;
                    continue;
                }
            }
            if ((x >= Clip.Left) && (x <= Clip.Right) && (y >= Clip.Top))
            {
                SET_CHARACTER_EX(p, ch->Code, ch->Color);
            }
            p++;
            ch++;
            rel_ofs--;
            start++;
            x++;
        }
    }
    return true;
}

bool Renderer::WriteCharacterBuffer(
      int x,
      int y,
      const AppCUI::Graphics::CharacterBuffer& cb,
      const AppCUI::Graphics::WriteCharacterBufferParams& params)
{
    CHECK_VISIBLE;

    unsigned int start = 0;
    unsigned int end   = cb.Len();
    if ((params.Flags & WriteCharacterBufferFlags::BUFFER_RANGE) != WriteCharacterBufferFlags::NONE)
    {
        CHECK(params.Start < end, false, "Start param (%d) should be smaller than buffer range: %d", params.Start, end);
        start = params.Start;
        if (end > params.End)
            end = params.End;
        CHECK(start < end,
              false,
              "Invalid start(%d) and end(%d) params for WriteCharacterBuffer. Start should be smaller than end.",
              start,
              end);
    }
    if (end <= start)
        return true; // Nothing to draw
    // single line
    if ((params.Flags & WriteCharacterBufferFlags::SINGLE_LINE) != WriteCharacterBufferFlags::NONE)
        return _WriteCharacterBuffer_SingleLine(x, y, cb, params, start, end);

    // multi line
    if ((params.Flags & WriteCharacterBufferFlags::MULTIPLE_LINES) != WriteCharacterBufferFlags::NONE)
    {
        // if new line will not be process and there is no width limit, than its like a single line
        if ((params.Flags & (WriteCharacterBufferFlags::WRAP_TO_WIDTH | WriteCharacterBufferFlags::PROCESS_NEW_LINE)) ==
            WriteCharacterBufferFlags::NONE)
            return _WriteCharacterBuffer_SingleLine(x, y, cb, params, start, end);
        if ((params.Flags & WriteCharacterBufferFlags::PROCESS_NEW_LINE) != WriteCharacterBufferFlags::NONE)
            return _WriteCharacterBuffer_MultiLine_ProcessNewLine(x, y, cb, params, start, end);
        else
            // implies that WriteCharacterBufferFlags::WRAP_TO_WIDTH is set
            return _WriteCharacterBuffer_MultiLine_WithWidth(x, y, cb, params, start, end);
    }
    return true;
}
bool Renderer::WriteCharacterBuffer(
      int x,
      int y,
      unsigned int width,
      const AppCUI::Graphics::CharacterBuffer& cb,
      const ColorPair textColor,
      TextAlignament align)
{
    return WriteCharacterBuffer(x, y, width, cb, textColor, NoColorPair, 0xFFFFFFFF, align);
}
bool Renderer::WriteCharacterBuffer(
      int x,
      int y,
      unsigned int width,
      const AppCUI::Graphics::CharacterBuffer& cb,
      const ColorPair textColor,
      const ColorPair hotKeyColor,
      unsigned int hotKeyOffset,
      TextAlignament align)
{
    WriteCharacterBufferParams params(
          WriteCharacterBufferFlags::OVERWRITE_COLORS | WriteCharacterBufferFlags::WRAP_TO_WIDTH |
          WriteCharacterBufferFlags::SINGLE_LINE);
    params.Color = textColor;
    if (hotKeyOffset != 0xFFFFFFFF)
    {
        params.Flags |= WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY;
        params.HotKeyColor    = hotKeyColor;
        params.HotKeyPosition = hotKeyOffset;
    }
    //bool hasPadding = ((align & TextAlignament::Padding) == TextAlignament::Padding);
    bool hasPadding = false;
    if (hasPadding)
    {
        CHECK(width >= 2, false, "width parameter must be bigger than 2 to support padding !");
        width -= 2;
    }
    params.Width        = width;
    unsigned int txSize = cb.Len();
    txSize              = MINVALUE(txSize, width);

    if ((align & TextAlignament::Right) == TextAlignament::Right)
    {
        // align on right
        x = (x - (int) txSize);
    }
    else if ((align & TextAlignament::Center) == TextAlignament::Center)
    {
        // align on right
        x = (x - ((int) txSize >> 1));
    }
    else
    {
        // align on left
        x++;
    }
    if (hasPadding)
    {
        CHECK(WriteCharacter(x - 1, y, ' ', textColor), false, "Fail to write left padding !");
        CHECK(WriteCharacter(x + txSize, y, ' ', textColor), false, "Fail to write write padding !");
    }
    CHECK(WriteCharacterBuffer(x, y, cb, params), false, "Fail to write text !");
    return true;
}

bool Renderer::DrawCanvas(int x, int y, const Canvas& canvas, const ColorPair overwriteColor)
{
    if ((!Clip.Visible) || (canvas.Characters == nullptr))
        return false;
    if ((x == 0) && (y == 0) && (this->Width == canvas.Width) && (this->Height == canvas.Height))
    {
        // optimize --> copy the entire buffer
        memcpy(this->Characters, canvas.Characters, sizeof(Character) * canvas.Height * canvas.Width);
        return true;
    }
    TRANSLATE_COORDONATES(x, y);
    int canvas_left = 0;
    int canvas_top  = 0;
    int canvas_width_memory_size;
    int canvas_width;
    int canvas_height;

    if (x < Clip.Left)
    {
        canvas_left = Clip.Left - x;
        x           = Clip.Left;
    }
    if (y < Clip.Top)
    {
        canvas_top = Clip.Top - y;
        y          = Clip.Top;
    }

    if ((x > Clip.Right) || (y > Clip.Bottom))
        return false;
    if (canvas_left >= (int) canvas.Width)
        return false;
    if (canvas_top >= (int) canvas.Height)
        return false;

    canvas_height = (Clip.Bottom - y) + 1;
    if (canvas_height > ((int) canvas.Height - canvas_top))
        canvas_height = (int) canvas.Height - canvas_top;
    if (canvas_height <= 0)
        return false;

    canvas_width = (Clip.Right - x) + 1;
    if (canvas_width > ((int) canvas.Width - canvas_left))
        canvas_width = (int) canvas.Width - canvas_left;
    if (canvas_width <= 0)
        return false;

    canvas_width_memory_size = canvas_width * sizeof(Character);

    if ((overwriteColor.Background == Color::Transparent) && (overwriteColor.Foreground == Color::Transparent))
    {
        // copy memory
        while (canvas_height > 0)
        {
            memcpy(this->OffsetRows[y] + x, canvas.OffsetRows[canvas_top] + canvas_left, canvas_width_memory_size);
            canvas_top++;
            y++;
            canvas_height--;
        }
    }
    else
    {
        // write character by character, changing the color
        if ((overwriteColor.Background != Color::Transparent) && (overwriteColor.Foreground != Color::Transparent))
        {
            while (canvas_height > 0)
            {
                Character* d       = this->OffsetRows[y] + x;
                Character* s       = canvas.OffsetRows[canvas_top] + canvas_left;
                unsigned int count = canvas_width;
                while (count)
                {
                    SET_CHARACTER(d, s->Code, overwriteColor);
                    d++;
                    s++;
                    count--;
                }
                canvas_top++;
                y++;
                canvas_height--;
            }
        }
        else
        {
            while (canvas_height > 0)
            {
                Character* d       = this->OffsetRows[y] + x;
                Character* s       = canvas.OffsetRows[canvas_top] + canvas_left;
                unsigned int count = canvas_width;
                while (count)
                {
                    SET_CHARACTER_EX(d, s->Code, overwriteColor);
                    d++;
                    s++;
                    count--;
                }

                canvas_top++;
                y++;
                canvas_height--;
            }
        }
    }
    return true;
}

bool Renderer::SetClipMargins(int leftMargin, int topMargin, int rightMargin, int bottomMargin)
{
    if (!this->ClipHasBeenCopied)
    {
        this->ClipCopy          = this->Clip;
        this->ClipHasBeenCopied = true;
    }
    if (!this->Clip.Visible)
        return false;

    Clip.Left          = ClipCopy.Left + MAXVALUE(leftMargin, 0);
    Clip.Top           = ClipCopy.Top + MAXVALUE(topMargin, 0);
    Clip.Right         = ClipCopy.Right - MAXVALUE(rightMargin, 0);
    Clip.Bottom        = ClipCopy.Bottom - MAXVALUE(bottomMargin, 0);
    this->Clip.Visible = (Clip.Left <= Clip.Right) && (Clip.Top <= Clip.Bottom);
    return this->Clip.Visible;
}
bool Renderer::ResetClip()
{
    CHECK(this->ClipHasBeenCopied, false, "Call to 'ResetClip' method wihout calling 'SetClip' first !");
    this->Clip = this->ClipCopy;
    return true;
}


bool Renderer::_Compute_DrawTextInfo_SingleLine_(
      const WriteTextParams& params, unsigned int charactersCount, DrawTextInfo& output)
{
    CHECK_VISIBLE;
    // check size
    if (charactersCount == 0)
        return false; // empty text, nothing to draw
    // translate coordonates
    int x = params.X + this->TranslateX;
    int y = params.Y + this->TranslateY;
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false; // outside clip rect -> exit

    output.TextStart = 0;
    output.TextEnd   = charactersCount;
    // check Text alignament
    switch (params.Align)
    {
    case TextAlignament::Left:
        if ((params.Flags & WriteTextFlags::ClipToWidth) != WriteTextFlags::None)
        {
            if (charactersCount>params.Width)
                output.TextEnd = params.Width;            
        }
        break;
    case TextAlignament::Right:
        if ((params.Flags & WriteTextFlags::ClipToWidth) != WriteTextFlags::None)
        {
            if (params.Width >= charactersCount)
                x = x + (int) (params.Width - charactersCount); // entire string fits the width
            else
                output.TextStart += (charactersCount - params.Width); // x remains the same, move text offset
            
        }            
        else
            x -= (int) (charactersCount-1);
        break;
    case TextAlignament::Center:
        if ((params.Flags & WriteTextFlags::ClipToWidth) != WriteTextFlags::None)
        {
            if (params.Width >= charactersCount)
                x = x + (int) ((params.Width - charactersCount)/2); // entire string fits the width
            else
            {
                output.TextStart += (charactersCount - params.Width) / 2; // x remains the same, move text
                output.TextEnd = output.TextStart + params.Width;
                if (output.TextEnd > charactersCount)
                    output.TextEnd = charactersCount; // sanity check
            }
                
        }
        else
            x -= (int) (charactersCount/2);
        break;
    default:
        RETURNERROR(false, "Unknown text align value: %d", params.Align);
    }
    
    // Check clipping
    if (x > Clip.Right)
        return false; // outside the clipping area
    if (x<Clip.Left)
    {
        if ((x + (int) (output.TextEnd - output.TextStart)) < Clip.Left)
            return false; // outside the clipping area
        output.TextStart += (unsigned int)(Clip.Left - x);
        x = Clip.Left;
    }
    if ((x + (int) (output.TextEnd - output.TextStart)) > Clip.Right)
    {
        output.TextEnd -= (unsigned int) ((x + (int) (output.TextEnd - output.TextStart)) - Clip.Right);
        if (output.TextEnd <= output.TextStart)
            return false; // nothing to draw (sanity check)
    }
    
    // compute screen buffer pointers
    output.Start = this->OffsetRows[y] + x;
    output.End   = output.Start + (output.TextEnd - output.TextStart);
    
    // hotkey
    if ((params.Flags & WriteTextFlags::HighlightHotKey) != WriteTextFlags::None)
    {
        if ((params.HotKeyPosition >= output.TextStart) && (params.HotKeyPosition < output.TextEnd))
            output.HotKey = output.Start + (params.HotKeyPosition - output.TextStart);
        else
            output.HotKey = nullptr; // not visible
    }
    else
    {
        output.HotKey = nullptr; // nothing to highlight
    }

    // all good 
    return true;
}
bool Renderer::_WriteText_SingleLine_(const CharacterBuffer& text, const WriteTextParams& params)
{
    DrawTextInfo dti;
    if (_Compute_DrawTextInfo_SingleLine_(params, text.Len(), dti) == false)
        return false;
    Character* ch = text.GetBuffer() + dti.TextStart;
    if ((params.Flags & WriteTextFlags::OverwriteColors) != WriteTextFlags::None)
    {
        if (NO_TRANSPARENCY(params.Color))
        {
            while (dti.Start < dti.End)
            {
                SET_CHARACTER(dti.Start, ch->Code, params.Color);
                dti.Start++;
                ch++;
            }
        }
        else
        {
            while (dti.Start < dti.End)
            {
                SET_CHARACTER_EX(dti.Start, ch->Code, params.Color);
                dti.Start++;
                ch++;
            }
        }
    }
    else
    {
        while (dti.Start < dti.End)
        {
            SET_CHARACTER_EX(dti.Start, ch->Code, ch->Color);
            dti.Start++;
            ch++;
        }
    }
    if (dti.HotKey)
    {
        SET_CHARACTER_EX(dti.HotKey, -1, params.HotKeyColor);
    }
    return true;
}
template <typename T>
inline void RenderSingleLineString(
      const T& text, AppCUI::Graphics::Renderer::DrawTextInfo& dti, const WriteTextParams& params)
{
    auto* ch = text.data() + dti.TextStart;

    if (NO_TRANSPARENCY(params.Color))
    {
        while (dti.Start < dti.End)
        {
            SET_CHARACTER(dti.Start, *ch, params.Color);
            dti.Start++;
            ch++;
        }
    }
    else
    {
        while (dti.Start < dti.End)
        {
            SET_CHARACTER_EX(dti.Start, *ch, params.Color);
            dti.Start++;
            ch++;
        }
    }

    if (dti.HotKey)
    {
        SET_CHARACTER_EX(dti.HotKey, -1, params.HotKeyColor);
    }
}

bool Renderer::WriteText(const CharacterBuffer& text, const WriteTextParams& params)
{
    if ((params.Flags & WriteTextFlags::SingleLine) != WriteTextFlags::None)
        return _WriteText_SingleLine_(text, params);
    NOT_IMPLEMENTED(false);
}
bool Renderer::WriteText(const AppCUI::Utils::ConstString& text, const WriteTextParams& params)
{
    DrawTextInfo dti;
    if ((params.Flags & WriteTextFlags::SingleLine) != WriteTextFlags::None)
    {
        if (std::holds_alternative<std::string_view>(text))
        {
            if (_Compute_DrawTextInfo_SingleLine_(params, std::get<std::string_view>(text).length(), dti) == false)
                return false;
            RenderSingleLineString<std::string_view>(std::get<std::string_view>(text), dti, params);
            return true;
        }
        if (std::holds_alternative<std::u16string_view>(text))
        {
            if (_Compute_DrawTextInfo_SingleLine_(params, std::get<std::u16string_view>(text).length(), dti) == false)
                return false;
            RenderSingleLineString<std::u16string_view>(std::get<std::u16string_view>(text), dti, params);
            return true;
        }
        if (std::holds_alternative<std::u8string_view>(text))
        {
            LocalUnicodeStringBuilder<1024> tmp(std::get<std::u8string_view>(text));
            if (_Compute_DrawTextInfo_SingleLine_(params, tmp.Len(), dti) == false)
                return false;
            RenderSingleLineString<std::u16string_view>(tmp.ToStringView(), dti, params);
            return true;
        }
        RETURNERROR(false, "Invalid ConstString type (specialized template was not implemented)");
    }
    NOT_IMPLEMENTED(false);
}
bool Renderer::WriteSingleLineText(int x, int y, const AppCUI::Utils::ConstString& text, ColorPair color)
{
    WriteTextParams params(WriteTextFlags::SingleLine);
    params.X = x;
    params.Y = y;
    params.Color = color;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, const AppCUI::Utils::ConstString& text, ColorPair color, TextAlignament align)
{
    WriteTextParams params(WriteTextFlags::SingleLine, align);
    params.X     = x;
    params.Y     = y;
    params.Color = color;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, const AppCUI::Utils::ConstString& text, ColorPair color, ColorPair hotKeyColor, unsigned int hotKeyOffset)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, const AppCUI::Utils::ConstString& text, ColorPair color, ColorPair hotKeyColor, unsigned int hotKeyOffset, TextAlignament align)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey, align);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, unsigned int width, const AppCUI::Utils::ConstString& text, ColorPair color)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth);
    params.X = x;
    params.Y = y;
    params.Color = color;
    params.Width = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, unsigned int width, const AppCUI::Utils::ConstString& text, ColorPair color, TextAlignament align)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth, align);
    params.X     = x;
    params.Y     = y;
    params.Color = color;
    params.Width = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, unsigned int width, const AppCUI::Utils::ConstString& text, ColorPair color, ColorPair hotKeyColor, unsigned int hotKeyOffset)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey | WriteTextFlags::ClipToWidth);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    params.Width          = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, unsigned int width, const AppCUI::Utils::ConstString& text, ColorPair color, ColorPair hotKeyColor, unsigned int hotKeyOffset, TextAlignament align)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey | WriteTextFlags::ClipToWidth, align);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    params.Width          = width;
    return WriteText(text, params);
}