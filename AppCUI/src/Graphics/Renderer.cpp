#include "Internal.hpp"

namespace AppCUI::Graphics
{
using namespace Utils;
struct LineTypeChars
{
    char16 TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left;
};
namespace UnicodeSpecialChars
{
    int special_characters[(uint32) Graphics::SpecialChars::Count] = {
        0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551, 0x256C,                         // double line box
        0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502, 0x253C,                         // single line box
        0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                                 // arrows
        32,     0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590, 0x25A0, // blocks
        0x25B2, 0x25BC, 0x25C4, 0x25BA,                                                 // Trangles
        0x25CF, 0x25CB, 0x221A, 0x2261, 0x205E, 0x2026,                                 // symbols
        0x251C, 0x252C, 0x2524, 0x2534                                                  // middle single line box
    };
    struct AppCUI::Graphics::LineTypeChars line_types_chars[] = {
        { 0x250C, 0x2500, 0x2510, 0x2502, 0x2518, 0x2500, 0x2514, 0x2502 }, /* Single Lines */
        { 0x2554, 0x2550, 0x2557, 0x2551, 0x255D, 0x2550, 0x255A, 0x2551 }, /* Double Lines */
        { 0x250F, 0x2501, 0x2513, 0x2503, 0x251B, 0x2501, 0x2517, 0x2503 }, /* Single Thick lines */
        { 0x2584, 0x2584, 0x2584, 0x2588, 0x2580, 0x2580, 0x2580, 0x2588 }, /* Border */
        { '+', '-', '+', '|', '+', '-', '+', '|' },                         /* Ascii */
        { '/', '-', '\\', '|', '/', '-', '\\', '|' },                       /* Ascii Round */
        { 0x256D, 0x2500, 0x256E, 0x2502, 0x256F, 0x2500, 0x2570, 0x2502 }, /* Single Round */
    };
} // namespace UnicodeSpecialChars
namespace LinuxTerminal
{
    int special_characters[(uint32) Graphics::SpecialChars::Count] = {
        0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502, 0x253C,                         // double line box
        0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502, 0x253C,                         // single line box
        0x25B2, 0x25BC, 0x25C0, 0x25B6, '|',    '-',                                    // arrows
        32,     0x2591, 0x2592, 0x2592, 0x2588, 0x25A0, 0x25A0, 0x25A0, 0x25A0, 0x25A0, // blocks
        0x25B2, 0x25BC, 0x25C0, 0x25B6,                                                 // Trangles
        0x25CF, 'o',    'x',    0x2261, ':',    0x2026,                                 // symbols
        0x251C, 0x252C, 0x2524, 0x2534                                                  // middle single line box
    };
    struct AppCUI::Graphics::LineTypeChars line_types_chars[] = {
        { 0x250C, 0x2500, 0x2510, 0x2502, 0x2518, 0x2500, 0x2514, 0x2502 }, /* Single Lines */
        { 0x250C, 0x2500, 0x2510, 0x2502, 0x2518, 0x2500, 0x2514, 0x2502 }, /* Double Lines */
        { 0x250C, 0x2500, 0x2510, 0x2502, 0x2518, 0x2500, 0x2514, 0x2502 }, /* Single Thick lines */
        { 0x250C, 0x2500, 0x2510, 0x2502, 0x2518, 0x2500, 0x2514, 0x2502 }, /* Border */
        { '+', '-', '+', '|', '+', '-', '+', '|' },                         /* Ascii */
        { '/', '-', '\\', '|', '/', '-', '\\', '|' },                       /* Ascii Round */
        { 0x250C, 0x2500, 0x2510, 0x2502, 0x2518, 0x2500, 0x2514, 0x2502 }, /* Single Round */
    };
} // namespace LinuxTerminal
namespace AsciiSpecialChars
{
    int special_characters[(uint32) Graphics::SpecialChars::Count] = {
        '+', '+', '+', '+', '=', '|', '+',                // double line box
        '+', '+', '+', '+', '-', '|', '+',                // single line box
        '^', 'v', '<', '>', '|', '-',                     // arrows
        ' ', ':', '#', '#', '@', '~', '_', '[', ']', '#', // blocks
        '^', 'v', '<', '>',                               // Trangles
        '*', 'o', 'V', '=', ':', '.',                     // symbols
        '+', '+', '+', '+'                                // middle single line box
    };
    struct AppCUI::Graphics::LineTypeChars line_types_chars[] = {
        { '+', '-', '+', '|', '+', '-', '+', '|' },   /* Single Lines */
        { '+', '=', '+', '|', '+', '=', '+', '|' },   /* Double Lines */
        { '+', '-', '+', '|', '+', '-', '+', '|' },   /* Single Thick lines */
        { '+', '-', '+', '|', '+', '-', '+', '|' },   /* Border */
        { '+', '-', '+', '|', '+', '-', '+', '|' },   /* Ascii */
        { '/', '-', '\\', '|', '/', '-', '\\', '|' }, /* Ascii Round */
        { '+', '-', '+', '|', '+', '-', '+', '|' },   /* Single Round */
    };
} // namespace AsciiSpecialChars

int* SpecialCharacters          = nullptr;
LineTypeChars* LineSpecialChars = nullptr;

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
        if (color.Foreground != Graphics::Color::Transparent)                                                          \
        {                                                                                                              \
            ptrCharInfo->Color.Foreground = color.Foreground;                                                          \
        }                                                                                                              \
        if (color.Background != Graphics::Color::Transparent)                                                          \
        {                                                                                                              \
            ptrCharInfo->Color.Background = color.Background;                                                          \
        }                                                                                                              \
    }
#define NO_TRANSPARENCY(color)                                                                                         \
    ((color.Foreground != Graphics::Color::Transparent) && (color.Background != Graphics::Color::Transparent))

using namespace Graphics;

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

struct DrawTextInfo
{
    int X, Y;
    size_t TextStart, TextEnd;
    Character* Start;
    Character* End;
    Character* HotKey;
    Character* FitCharStart;
    Character* LeftMargin;
    Character* RightMargin;
};

//============================================== Write text [single line] ===========================================
template <typename T>
inline void RenderSingleLineString(const T& text, DrawTextInfo& dti, const WriteTextParams& params)
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
    // hot key
    if (dti.HotKey)
    {
        SET_CHARACTER_EX(dti.HotKey, -1, params.HotKeyColor);
    }
    // fit size
    if (dti.FitCharStart)
    {
        for (; dti.FitCharStart < dti.End; dti.FitCharStart++)
        {
            SET_CHARACTER_EX(dti.FitCharStart, '.', NoColorPair);
        }
    }
    // margins
    if (dti.LeftMargin)
    {
        SET_CHARACTER_EX(dti.LeftMargin, ' ', NoColorPair);
    }
    if (dti.RightMargin)
    {
        SET_CHARACTER_EX(dti.RightMargin, ' ', NoColorPair);
    }
}
template <>
inline void RenderSingleLineString<CharacterView>(
      const CharacterView& text, DrawTextInfo& dti, const WriteTextParams& params)
{
    const Character* ch = text.data() + dti.TextStart;
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
    // fit size
    if (dti.FitCharStart)
    {
        for (; dti.FitCharStart < dti.End; dti.FitCharStart++)
        {
            SET_CHARACTER_EX(dti.FitCharStart, '.', NoColorPair);
        }
    }
    // margins
    if (dti.LeftMargin)
    {
        if ((params.Flags & WriteTextFlags::OverwriteColors) != WriteTextFlags::None)
        {
            SET_CHARACTER_EX(dti.LeftMargin, ' ', params.Color);
        }
        else
        {
            SET_CHARACTER_EX(dti.LeftMargin, ' ', NoColorPair);
        }
    }
    if (dti.RightMargin)
    {
        if ((params.Flags & WriteTextFlags::OverwriteColors) != WriteTextFlags::None)
        {
            SET_CHARACTER_EX(dti.RightMargin, ' ', params.Color);
        }
        else
        {
            SET_CHARACTER_EX(dti.RightMargin, ' ', NoColorPair);
        }
    }
}

//============================================== Write text [multi line] ============================================
template <typename T>
inline bool ProcessMultiLinesString(const T& text, const WriteTextParams& params, Renderer& renderer)
{
    auto p         = text.data();
    auto end       = p + text.length();
    auto lineStart = p;
    auto start     = p;
    bool result    = false;
    bool textWrap  = false;

    CHECK(text.length() < 0x00FFFFFF,
          false,
          "Text is too long: %z characters (should be smaller than 0x00FFFFFF)",
          (size_t) text.length());

    WriteTextParams singleLineParams;
    singleLineParams.Flags =
          params.Flags & (WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
                          WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin | WriteTextFlags::HighlightHotKey |
                          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);
    singleLineParams.Flags |= WriteTextFlags::SingleLine;
    singleLineParams.Align       = params.Align;
    singleLineParams.Color       = params.Color;
    singleLineParams.HotKeyColor = params.HotKeyColor;
    singleLineParams.X           = params.X;
    singleLineParams.Y           = params.Y;
    singleLineParams.Width       = params.Width;
    bool showHotKey              = (params.Flags & WriteTextFlags::HighlightHotKey) != WriteTextFlags::None;

    if ((params.Flags & WriteTextFlags::WrapToWidth) != WriteTextFlags::None)
    {
        // Wrap to Width requires ClipToWidth as well for alignament
        singleLineParams.Flags |= WriteTextFlags::ClipToWidth;
        textWrap = true;
    }

    if (textWrap)
    {
        while (p < end)
        {
            lineStart              = p;
            singleLineParams.Width = 0;
            while ((p < end) && ((*p) != '\n') && ((*p) != '\r') && (singleLineParams.Width < params.Width))
            {
                p++;
                singleLineParams.Width++;
            }
            if (showHotKey)
            {
                if (((lineStart - start) <= params.HotKeyPosition) && ((p - start) > params.HotKeyPosition))
                    singleLineParams.HotKeyPosition = params.HotKeyPosition - (uint32) (lineStart - start);
                else
                    singleLineParams.HotKeyPosition = 0xFFFFFFFF;
            }
            result |= renderer.WriteText(T(lineStart, p - lineStart), singleLineParams);
            // skip new line and update Y parameter
            lineStart = p;
            while (p < end)
            {
                if ((*p) == '\n')
                {
                    singleLineParams.Y++;
                    p++;
                    if ((p < end) && ((*p) == '\r')) // skip CRLF
                        p++;
                    continue;
                }
                if ((*p) == '\r')
                {
                    singleLineParams.Y++;
                    p++;
                    if ((p < end) && ((*p) == '\n')) // skip LFCR
                        p++;
                    continue;
                }
                if (p == lineStart)
                    singleLineParams.Y++; // we've break the previous text as a result of text wrapping
                break;
            }
        }
    }
    else
    {
        while (p < end)
        {
            lineStart = p;
            while ((p < end) && ((*p) != '\n') && ((*p) != '\r'))
                p++;
            if (showHotKey)
            {
                if (((lineStart - start) <= params.HotKeyPosition) && ((p - start) > params.HotKeyPosition))
                    singleLineParams.HotKeyPosition = params.HotKeyPosition - (uint32) (lineStart - start);
                else
                    singleLineParams.HotKeyPosition = 0xFFFFFFFF;
            }
            result |= renderer.WriteText(T(lineStart, p - lineStart), singleLineParams);
            // skip new line and update Y parameter
            while (p < end)
            {
                if ((*p) == '\n')
                {
                    singleLineParams.Y++;
                    p++;
                    if ((p < end) && ((*p) == '\r')) // skip CRLF
                        p++;
                    continue;
                }
                if ((*p) == '\r')
                {
                    singleLineParams.Y++;
                    p++;
                    if ((p < end) && ((*p) == '\n')) // skip LFCR
                        p++;
                    continue;
                }
                break; // no CR or LF
            }
        }
    }
    return result;
}

Renderer::Renderer()
{
    TranslateX = TranslateY = 0;
    Width                   = 0;
    Height                  = 0;
    Characters              = nullptr;
    OffsetRows              = nullptr;
    if (SpecialCharacters == nullptr)
        SpecialCharacters = UnicodeSpecialChars::special_characters;
    if (LineSpecialChars == nullptr)
        LineSpecialChars = UnicodeSpecialChars::line_types_chars;

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
    this->Cursor.X       = (uint32) x;
    this->Cursor.Y       = (uint32) y;
    this->Cursor.Visible = true;
    return true;
}
bool Renderer::WriteCharacter(int x, int y, int charCode, ColorPair color)
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

bool Renderer::GetCharacter(int x, int y, Character& c)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, y);

    if ((x < Clip.Left) || (x > Clip.Right) || (y < Clip.Top) || (y > Clip.Bottom))
    {
        return false;
    }

    const Character* p = this->OffsetRows[y] + x;

    c.Code = p->Code;

    if (NO_TRANSPARENCY(c.Color))
    {
        c.Color = p->Color;
    }
    else
    {
        if (c.Color.Foreground != Graphics::Color::Transparent)
        {
            c.Color.Foreground = p->Color.Foreground;
        }
        if (c.Color.Background != Graphics::Color::Transparent)
        {
            c.Color.Background = p->Color.Background;
        }
    }

    return true;
}

bool Renderer::WriteSpecialCharacter(int x, int y, SpecialChars charID, ColorPair color)
{
    return WriteCharacter(x, y, SpecialCharacters[(uint32) charID], color);
}

bool Renderer::_ClearEntireSurface(int character, ColorPair color)
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
        tmp.Code = (uint16) (character & 0xFFFF);
    while (s < e)
    {
        s->PackedValue = tmp.PackedValue;
        s++;
    }
    return true;
}
bool Renderer::Clear(int charCode, ColorPair color)
{
    CHECK_VISIBLE;
    if ((Clip.Left == 0) && (Clip.Top == 0) && (static_cast<unsigned>(Clip.Right + 1) == this->Width) &&
        (static_cast<unsigned>(Clip.Bottom + 1) == this->Height))
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
bool Renderer::ClearWithSpecialChar(SpecialChars charID, ColorPair color)
{
    return Clear(SpecialCharacters[(uint32) charID], color);
}
bool Renderer::FillHorizontalLine(int left, int y, int right, int charCode, ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, y);
    TRANSLATE_X_COORDONATE(right);
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false;
    left  = std::max<>(left, Clip.Left);
    right = std::min<>(right, Clip.Right);
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
bool Renderer::FillHorizontalLineWithSpecialChar(int left, int y, int right, SpecialChars charID, ColorPair color)
{
    return FillHorizontalLine(left, y, right, SpecialCharacters[(uint32) charID], color);
}
bool Renderer::FillHorizontalLineSize(int x, int y, uint32 size, int charCode, ColorPair color)
{
    CHECK(size > 0, false, "");
    return FillHorizontalLine(x, y, x + ((int) size) - 1, charCode, color);
}
bool Renderer::DrawHorizontalLine(int left, int y, int right, ColorPair color, bool singleLine)
{
    if (singleLine)
        return FillHorizontalLine(
              left, y, right, SpecialCharacters[(uint32) Graphics::SpecialChars::BoxHorizontalSingleLine], color);
    else
        return FillHorizontalLine(
              left, y, right, SpecialCharacters[(uint32) Graphics::SpecialChars::BoxHorizontalDoubleLine], color);
}
bool Renderer::FillVerticalLine(int x, int top, int bottom, int charCode, ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(x, top);
    TRANSLATE_Y_COORDONATE(bottom);
    if ((x < Clip.Left) || (x > Clip.Right))
        return false;
    top    = std::max<>(top, Clip.Top);
    bottom = std::min<>(bottom, Clip.Bottom);
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
bool Renderer::FillVerticalLineSize(int x, int y, uint32 size, int charCode, ColorPair color)
{
    CHECK(size > 0, false, "");
    return FillVerticalLine(x, y, y + ((int) size) - 1, charCode, color);
}
bool Renderer::FillVerticalLineWithSpecialChar(int x, int top, int bottom, SpecialChars charID, ColorPair color)
{
    return FillVerticalLine(x, top, bottom, SpecialCharacters[(uint32) charID], color);
}
bool Renderer::DrawVerticalLine(int x, int top, int bottom, ColorPair color, bool singleLine)
{
    if (singleLine)
        return FillVerticalLine(
              x, top, bottom, SpecialCharacters[(uint32) Graphics::SpecialChars::BoxVerticalSingleLine], color);
    else
        return FillVerticalLine(
              x, top, bottom, SpecialCharacters[(uint32) Graphics::SpecialChars::BoxVerticalDoubleLine], color);
}
bool Renderer::FillRect(int left, int top, int right, int bottom, int charCode, ColorPair color)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, top);
    TRANSLATE_COORDONATES(right, bottom);

    left   = std::max<>(left, Clip.Left);
    top    = std::max<>(top, Clip.Top);
    right  = std::min<>(right, Clip.Right);
    bottom = std::min<>(bottom, Clip.Bottom);

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
bool Renderer::DrawRect(int left, int top, int right, int bottom, ColorPair color, LineType lineType)
{
    CHECK_VISIBLE;
    TRANSLATE_COORDONATES(left, top);
    TRANSLATE_COORDONATES(right, bottom);

    int orig_left   = left;
    int orig_right  = right;
    int orig_top    = top;
    int orig_bottom = bottom;

    left   = std::max<>(left, Clip.Left);
    top    = std::max<>(top, Clip.Top);
    right  = std::min<>(right, Clip.Right);
    bottom = std::min<>(bottom, Clip.Bottom);

    if ((left > right) || (top > bottom))
        return false;

    Character *p, *e;
    int char_to_draw;

    const auto& chrs = LineSpecialChars[(uint8) lineType];

    // top line
    if (top == orig_top)
    {
        p            = this->OffsetRows[top] + left;
        e            = this->OffsetRows[top] + right;
        char_to_draw = chrs.Top;
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
        p            = this->OffsetRows[bottom] + left;
        e            = this->OffsetRows[bottom] + right;
        char_to_draw = chrs.Bottom;
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
        p            = this->OffsetRows[top] + left;
        e            = this->OffsetRows[bottom] + left;
        char_to_draw = chrs.Left;
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
        p            = this->OffsetRows[top] + right;
        e            = this->OffsetRows[bottom] + right;
        char_to_draw = chrs.Right;
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
        char_to_draw = chrs.TopLeft;
        p            = this->OffsetRows[top] + left;
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
        char_to_draw = chrs.BottomLeft;
        p            = this->OffsetRows[bottom] + left;
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
        char_to_draw = chrs.BottomRight;
        p            = this->OffsetRows[bottom] + right;
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
        char_to_draw = chrs.TopRight;
        p            = this->OffsetRows[top] + right;
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
bool Renderer::FillRectSize(int x, int y, uint32 width, uint32 height, int charCode, ColorPair color)
{
    CHECK(((width > 0) && (height > 0)), false, "");
    return FillRect(x, y, x + ((int) width) - 1, y + ((int) height) - 1, charCode, color);
}
bool Renderer::DrawRectSize(int x, int y, uint32 width, uint32 height, ColorPair color, LineType lineType)
{
    CHECK(((width > 0) && (height > 0)), false, "");
    return DrawRect(x, y, x + ((int) width) - 1, y + ((int) height) - 1, color, lineType);
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
                Character* d = this->OffsetRows[y] + x;
                Character* s = canvas.OffsetRows[canvas_top] + canvas_left;
                uint32 count = canvas_width;
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
                Character* d = this->OffsetRows[y] + x;
                Character* s = canvas.OffsetRows[canvas_top] + canvas_left;
                uint32 count = canvas_width;
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

    Clip.Left          = ClipCopy.Left + std::max<>(leftMargin, 0);
    Clip.Top           = ClipCopy.Top + std::max<>(topMargin, 0);
    Clip.Right         = ClipCopy.Right - std::max<>(rightMargin, 0);
    Clip.Bottom        = ClipCopy.Bottom - std::max<>(bottomMargin, 0);
    this->Clip.Visible = (Clip.Left <= Clip.Right) && (Clip.Top <= Clip.Bottom);
    return this->Clip.Visible;
}
bool Renderer::SetClipRect(const Rect& r)
{
    if (!this->ClipHasBeenCopied)
    {
        this->ClipCopy          = this->Clip;
        this->ClipHasBeenCopied = true;
    }
    if (!this->Clip.Visible)
        return false;
    Clip.Left          = ClipCopy.Left + std::max<>(r.GetLeft(), 0);
    Clip.Top           = ClipCopy.Top + std::max<>(r.GetTop(), 0);
    Clip.Right         = std::min<>(ClipCopy.Right, ClipCopy.Left + r.GetRight());
    Clip.Bottom        = std::min<>(ClipCopy.Bottom, ClipCopy.Top + r.GetBottom());
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
      const WriteTextParams& params, size_t charactersCount, void* drawTextInfoOutput)
{
    CHECK_VISIBLE;
    // check size
    if (charactersCount == 0)
        return false; // empty text, nothing to draw
    CHECK(charactersCount < 0x00FFFFFF,
          false,
          "String is too large (%z characters) - max allowed size is 0x00FFFFFF !",
          charactersCount);

    // translate coordonates
    int x = params.X + this->TranslateX;
    int y = params.Y + this->TranslateY;
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false; // outside clip rect -> exit

    const auto output    = reinterpret_cast<DrawTextInfo*>(drawTextInfoOutput);
    output->TextStart    = 0;
    output->TextEnd      = charactersCount;
    output->FitCharStart = nullptr;
    bool TextFit         = false;
    // check Text alignament
    switch (params.Align)
    {
    case TextAlignament::Left:
        if ((params.Flags & (WriteTextFlags::FitTextToWidth | WriteTextFlags::ClipToWidth)) != WriteTextFlags::None)
        {
            if (charactersCount > params.Width)
            {
                output->TextEnd = params.Width;
                TextFit         = (params.Flags & WriteTextFlags::FitTextToWidth) != WriteTextFlags::None;
            }
        }
        break;
    case TextAlignament::Right:
        if ((params.Flags & WriteTextFlags::ClipToWidth) != WriteTextFlags::None)
        {
            if (params.Width >= charactersCount)
                x = x + (int) (params.Width - charactersCount); // entire string fits the width
            else
            {
                // x remains the same, move text offset depending on settings
                if ((params.Flags & WriteTextFlags::FitTextToWidth) != WriteTextFlags::None)
                {
                    output->TextEnd = params.Width;
                    TextFit         = true;
                }
                else
                {
                    output->TextStart += (charactersCount - params.Width);
                }
            }
        }
        else
        {
            x -= (int) (charactersCount - 1);
        }
        break;
    case TextAlignament::Center:
        if ((params.Flags & WriteTextFlags::ClipToWidth) != WriteTextFlags::None)
        {
            if (params.Width >= charactersCount)
                x = x + (int) ((params.Width - charactersCount) / 2); // entire string fits the width
            else
            {
                if ((params.Flags & WriteTextFlags::FitTextToWidth) != WriteTextFlags::None)
                {
                    output->TextEnd = params.Width;
                    TextFit         = true;
                }
                else
                {
                    output->TextStart += (charactersCount - params.Width) / 2; // x remains the same, move text
                    output->TextEnd = output->TextStart + params.Width;
                    if (output->TextEnd > charactersCount)
                        output->TextEnd = charactersCount; // sanity check
                }
            }
        }
        else
            x -= (int) (charactersCount / 2);
        break;
    default:
        RETURNERROR(false, "Unknown text align value: %d", params.Align);
    }

    // Check clipping
    if (x > Clip.Right)
        return false; // outside the clipping area
    if (x < Clip.Left)
    {
        if ((x + (int) (output->TextEnd - output->TextStart)) < Clip.Left)
            return false; // outside the clipping area
        output->TextStart += (Clip.Left - x);
        x = Clip.Left;
    }
    if ((x + (int) (output->TextEnd - output->TextStart)) > (Clip.Right + 1))
    {
        output->TextEnd -= ((x + (int) (output->TextEnd - output->TextStart)) - (Clip.Right + 1));
        if (output->TextEnd <= output->TextStart)
            return false; // nothing to draw (sanity check)
    }

    // compute screen buffer pointers
    output->Start = this->OffsetRows[y] + x;
    output->End   = output->Start + (output->TextEnd - output->TextStart);

    // hotkey
    if ((params.Flags & WriteTextFlags::HighlightHotKey) != WriteTextFlags::None)
    {
        if ((params.HotKeyPosition >= output->TextStart) && (params.HotKeyPosition < output->TextEnd))
            output->HotKey = output->Start + (params.HotKeyPosition - output->TextStart);
        else
            output->HotKey = nullptr; // not visible
    }
    else
    {
        output->HotKey = nullptr; // nothing to highlight
    }

    // Text fit
    if (TextFit)
    {
        uint32 sz = (uint32) (output->TextEnd - output->TextStart);
        if (sz > 4)
        {
            sz                   = std::min<>(sz - 3, 3U);
            output->FitCharStart = output->End - sz;
        }
    }

    // margins
    output->LeftMargin  = nullptr;
    output->RightMargin = nullptr;
    if (((params.Flags & WriteTextFlags::LeftMargin) != WriteTextFlags::None) && ((x - 1) >= Clip.Left))
        output->LeftMargin = output->Start - 1;
    if (((params.Flags & WriteTextFlags::RightMargin) != WriteTextFlags::None) &&
        ((x + output->TextEnd - output->TextStart) <= static_cast<unsigned>(Clip.Right)))
        output->RightMargin = output->End;
    // all good
    return true;
}

bool Renderer::WriteText(const ConstString& text, const WriteTextParams& params)
{
    DrawTextInfo dti;
    if ((params.Flags & WriteTextFlags::SingleLine) != WriteTextFlags::None)
    {
        if (std::holds_alternative<string_view>(text))
        {
            if (_Compute_DrawTextInfo_SingleLine_(params, std::get<string_view>(text).length(), &dti) == false)
                return false;
            RenderSingleLineString<string_view>(std::get<string_view>(text), dti, params);
            return true;
        }
        if (std::holds_alternative<CharacterView>(text))
        {
            if (_Compute_DrawTextInfo_SingleLine_(params, std::get<CharacterView>(text).length(), &dti) == false)
                return false;
            RenderSingleLineString<CharacterView>(std::get<CharacterView>(text), dti, params);
            return true;
        }
        if (std::holds_alternative<u16string_view>(text))
        {
            if (_Compute_DrawTextInfo_SingleLine_(params, std::get<u16string_view>(text).length(), &dti) == false)
                return false;
            RenderSingleLineString<u16string_view>(std::get<u16string_view>(text), dti, params);
            return true;
        }
        if (std::holds_alternative<u8string_view>(text))
        {
            LocalUnicodeStringBuilder<1024> tmp(std::get<u8string_view>(text));
            if (_Compute_DrawTextInfo_SingleLine_(params, tmp.Len(), &dti) == false)
                return false;
            RenderSingleLineString<u16string_view>(tmp.ToStringView(), dti, params);
            return true;
        }
        RETURNERROR(false, "Invalid ConstString type (specialized template was not implemented)");
    }
    if ((params.Flags & WriteTextFlags::MultipleLines) != WriteTextFlags::None)
    {
        if (std::holds_alternative<CharacterView>(text))
        {
            return ProcessMultiLinesString<CharacterView>(std::get<CharacterView>(text), params, *this);
        }
        if (std::holds_alternative<string_view>(text))
        {
            return ProcessMultiLinesString<string_view>(std::get<string_view>(text), params, *this);
        }
        if (std::holds_alternative<u16string_view>(text))
        {
            return ProcessMultiLinesString<u16string_view>(std::get<u16string_view>(text), params, *this);
        }
        if (std::holds_alternative<u8string_view>(text))
        {
            LocalUnicodeStringBuilder<2048> tmp(std::get<u8string_view>(text));
            return ProcessMultiLinesString<u16string_view>(std::get<u16string_view>(text), params, *this);
        }
        RETURNERROR(false, "Invalid ConstString type (specialized template was not implemented)");
    }
    RETURNERROR(false, "Missing `WriteTextFlags::MultipleLines` or `WriteTextFlags::SingleLine` from params.Flags !");
}
bool Renderer::WriteSingleLineText(int x, int y, const ConstString& text, ColorPair color)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors);
    params.X     = x;
    params.Y     = y;
    params.Color = color;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, const ConstString& text, ColorPair color, TextAlignament align)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors, align);
    params.X     = x;
    params.Y     = y;
    params.Color = color;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(
      int x, int y, const ConstString& text, ColorPair color, ColorPair hotKeyColor, uint32 hotKeyOffset)
{
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(
      int x,
      int y,
      const ConstString& text,
      ColorPair color,
      ColorPair hotKeyColor,
      uint32 hotKeyOffset,
      TextAlignament align)
{
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey, align);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(int x, int y, uint32 width, const ConstString& text, ColorPair color)
{
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth);
    params.X     = x;
    params.Y     = y;
    params.Color = color;
    params.Width = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(
      int x, int y, uint32 width, const ConstString& text, ColorPair color, TextAlignament align)
{
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth, align);
    params.X     = x;
    params.Y     = y;
    params.Color = color;
    params.Width = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(
      int x, int y, uint32 width, const ConstString& text, ColorPair color, ColorPair hotKeyColor, uint32 hotKeyOffset)
{
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    params.Width          = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineText(
      int x,
      int y,
      uint32 width,
      const ConstString& text,
      ColorPair color,
      ColorPair hotKeyColor,
      uint32 hotKeyOffset,
      TextAlignament align)
{
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
                WriteTextFlags::ClipToWidth,
          align);
    params.X              = x;
    params.Y              = y;
    params.Color          = color;
    params.HotKeyColor    = hotKeyColor;
    params.HotKeyPosition = hotKeyOffset;
    params.Width          = width;
    return WriteText(text, params);
}
bool Renderer::WriteSingleLineCharacterBuffer(int x, int y, CharacterView charView, bool noTransparency)
{
    CHECK_VISIBLE;
    // check size
    auto buf = charView.data();
    auto sz  = charView.size();
    auto end = buf + sz;
    if (sz == 0)
        return false; // empty text, nothing to draw
    CHECK(sz < 0x00FFFFFF, false, "String is too large (%z characters) - max allowed size is 0x00FFFFFF !", sz);

    // translate coordonates
    x += this->TranslateX;
    y += this->TranslateY;

    // vertical clip
    if ((y < Clip.Top) || (y > Clip.Bottom))
        return false; // outside clip rect -> exit

    // horizontal clip
    if (x > Clip.Right)
        return false; // outside clip rect
    if (x < Clip.Left)
    {
        buf += Clip.Left - x;
        x = Clip.Left;
        if (buf >= end)
            return false; // ouside clip rect
        sz = (uint32) (end - buf);
    }
    if ((x + (int) sz) > Clip.Right)
    {
        sz  = (Clip.Right + 1) - x;
        end = buf + sz;
        if (sz == 0)
            return false; // nothing to draw
    }
    // draw
    if (noTransparency)
    {
        // fast draw --> use a memcpy
        memcpy(this->OffsetRows[y] + x, buf, sz * sizeof(Character));
    }
    else
    {
        auto p = this->OffsetRows[y] + x;
        while (buf < end)
        {
            SET_CHARACTER_EX(p, buf->Code, buf->Color);
            buf++;
            p++;
        }
    }

    return true;
}

//=========================================================================[IMAGE]===================
struct _RGB_Color_
{
    uint8 Red, Green, Blue;
    Color c;
};
static const _RGB_Color_ _console_colors_[16] = {
    _RGB_Color_{ 0, 0, 0, Color::Black },        // Black
    _RGB_Color_{ 0, 0, 128, Color::DarkBlue },   // DarkBlue
    _RGB_Color_{ 0, 128, 0, Color::DarkGreen },  // DarkGreen
    _RGB_Color_{ 0, 128, 128, Color::Teal },     // Teal
    _RGB_Color_{ 128, 0, 0, Color::DarkRed },    // DarkRed
    _RGB_Color_{ 128, 0, 128, Color::Magenta },  // Purple
    _RGB_Color_{ 128, 128, 0, Color::Olive },    // Brown
    _RGB_Color_{ 192, 192, 192, Color::Silver }, // LightGray
    _RGB_Color_{ 128, 128, 128, Color::Gray },   // DarkGray
    _RGB_Color_{ 0, 0, 255, Color::Blue },       // Blue
    _RGB_Color_{ 0, 255, 0, Color::Green },      // Green
    _RGB_Color_{ 0, 255, 255, Color::Aqua },     // Aqua
    _RGB_Color_{ 255, 0, 0, Color::Red },        // Red
    _RGB_Color_{ 255, 0, 255, Color::Pink },     // Pink
    _RGB_Color_{ 255, 255, 0, Color::Yellow },   // Yellow
    _RGB_Color_{ 255, 255, 255, Color::White },  // White
};
Color _color_map_16_[] = {
    /* 0*/ Color::Black,     // (0, 0, 0)
    /* 1*/ Color::DarkBlue,  // (0, 0, 1)
    /* 2*/ Color::Blue,      // (0, 0, 2)
    /* 3*/ Color::DarkGreen, // (0, 1, 0)
    /* 4*/ Color::Teal,      // (0, 1, 1)
    /* 5*/ Color::Teal,      // (0, 1, 2) [Aprox]
    /* 6*/ Color::Green,     // (0, 2, 0)
    /* 7*/ Color::Teal,      // (0, 2, 1) [Aprox]
    /* 8*/ Color::Aqua,      // (0, 2, 2)
    /* 9*/ Color::DarkRed,   // (1, 0, 0)
    /*10*/ Color::Magenta,   // (1, 0, 1)
    /*11*/ Color::Magenta,   // (1, 0, 2) [Aprox]
    /*12*/ Color::Olive,     // (1, 1, 0)
    /*13*/ Color::Gray,      // (1, 1, 1)
    /*14*/ Color::Gray,      // (1, 1, 2) [Aprox]
    /*15*/ Color::Olive,     // (1, 2, 0) [Aprox]
    /*16*/ Color::Gray,      // (1, 2, 1) [Aprox]
    /*17*/ Color::Silver,    // (1, 2, 2) [Aprox]
    /*18*/ Color::Red,       // (2, 0, 0)
    /*19*/ Color::Magenta,   // (2, 0, 1) [Aprox]
    /*20*/ Color::Pink,      // (2, 0, 2)
    /*21*/ Color::Olive,     // (2, 1, 0) [Aprox]
    /*22*/ Color::Gray,      // (2, 1, 1) [Aprox]
    /*23*/ Color::Silver,    // (2, 1, 2) [Aprox]
    /*24*/ Color::Yellow,    // (2, 2, 0)
    /*25*/ Color::Silver,    // (2, 2, 1) [Aprox]
    /*26*/ Color::White,     // (2, 2, 2)
};
inline uint32 Channel_To_Index16(uint32 rgbChannelValue)
{
    if (rgbChannelValue <= 64)
        return 0;
    else if (rgbChannelValue < 192)
        return 1;
    return 2;
}
Color RGB_to_16Color(Pixel colorRGB)
{
    uint32 b = Channel_To_Index16(colorRGB.Blue);  // blue channel
    uint32 g = Channel_To_Index16(colorRGB.Green); // green channel
    uint32 r = Channel_To_Index16(colorRGB.Red);   // red channel
    return _color_map_16_[r * 9 + g * 3 + b];
}

inline uint32 Channel_Diff(uint32 v1, uint32 v2)
{
    if (v1 > v2)
        return v1 - v2;
    else
        return v2 - v1;
}
void PixelTo64Color(Pixel colorRGB, ColorPair& c, SpecialChars& ch)
{
    // linear search - not efficient but good enough for the first implementation
    // in the future should be changed to a lookup table
    uint32 R = colorRGB.Red;
    uint32 G = colorRGB.Green;
    uint32 B = colorRGB.Blue;

    uint32 composeR = 0;
    uint32 composeG = 0;
    uint32 composeB = 0;

    uint32 BestDiff = 0xFFFFFFFF;

    for (uint32 c1 = 0; c1 < 16; c1++)
    {
        for (uint32 c2 = 0; c2 < 16; c2++)
        {
            auto& cc1 = _console_colors_[c1];
            auto& cc2 = _console_colors_[c2];

            for (uint32 proc = 1; proc <= 4; proc++) // 25%,50%,75%.100%
            {
                composeR  = ((((uint32) cc1.Red) * proc) + (((uint32) cc2.Red) * (4 - proc))) / 4;
                composeG  = ((((uint32) cc1.Green) * proc) + (((uint32) cc2.Green) * (4 - proc))) / 4;
                composeB  = ((((uint32) cc1.Blue) * proc) + (((uint32) cc2.Blue) * (4 - proc))) / 4;
                uint32 df = 0;
                df += Channel_Diff(R, composeR);
                df += Channel_Diff(G, composeG);
                df += Channel_Diff(B, composeB);
                if (df <= BestDiff)
                {
                    BestDiff = df;
                    c        = ColorPair{ cc1.c, cc2.c };
                    if (proc == 1)
                        ch = SpecialChars::Block25;
                    else if (proc == 2)
                        ch = SpecialChars::Block50;
                    else if (proc == 3)
                        ch = SpecialChars::Block75;
                    else
                        ch = SpecialChars::Block100;
                    if (BestDiff == 0)
                        return; // found a perfect match
                }
            }
        }
    }
}
void PixelToGrayScaleCharacter(Pixel colorRGB, ColorPair& c, SpecialChars& ch)
{
    auto val = colorRGB.ToGrayScale();
    c        = ColorPair{ Color::White, Color::Black };
    if (val < 12)
        ch = SpecialChars::Block0;
    else if (val < 37)
        ch = SpecialChars::Block25;
    else if (val < 62)
        ch = SpecialChars::Block50;
    else if (val < 87)
        ch = SpecialChars::Block75;
    else
        ch = SpecialChars::Block100;
}
void Paint_SmallBlocks(Renderer& r, const Graphics::Image& img, int x, int y, uint32 rap)
{
    const auto w     = img.GetWidth();
    const auto h     = img.GetHeight();
    const auto xStep = rap;
    const auto yStep = rap * 2;
    int px           = 0;
    ColorPair cp     = NoColorPair;
    for (uint32 img_y = 0; img_y < h; img_y += yStep, y++)
    {
        px = x;
        for (uint32 img_x = 0; img_x < w; img_x += xStep, px++)
        {
            if (rap == 1)
                cp = { RGB_to_16Color(img.GetPixel(img_x, img_y)), RGB_to_16Color(img.GetPixel(img_x, img_y + 1)) };
            else
                cp = { RGB_to_16Color(img.ComputeSquareAverageColor(img_x, img_y, rap)),
                       RGB_to_16Color(img.ComputeSquareAverageColor(img_x, img_y + rap, rap)) };

            if (cp.Background == cp.Foreground)
            {
                if (cp.Background == Color::Black)
                    r.WriteCharacter(px, y, ' ', cp);
                else
                    r.WriteSpecialCharacter(px, y, SpecialChars::Block100, cp);
            }
            else
                r.WriteSpecialCharacter(px, y, SpecialChars::BlockUpperHalf, cp);
        }
    }
}
void Paint_LargeBlocks(Renderer& r, const Graphics::Image& img, int x, int y, uint32 rap)
{
    const auto w    = img.GetWidth();
    const auto h    = img.GetHeight();
    int px          = 0;
    ColorPair cp    = NoColorPair;
    SpecialChars sc = SpecialChars::Block100;
    for (uint32 img_y = 0; img_y < h; img_y += rap, y++)
    {
        px = x;
        for (uint32 img_x = 0; img_x < w; img_x += rap, px += 2)
        {
            if (rap == 1)
                PixelTo64Color(img.GetPixel(img_x, img_y), cp, sc);
            else
                PixelTo64Color(img.ComputeSquareAverageColor(img_x, img_y, rap), cp, sc);

            // r.FillHorizontalLineWithSpecialChar(px, y, px + 1, sc, ColorPair{ cp, Color::Black });
            r.FillHorizontalLineWithSpecialChar(px, y, px + 1, sc, cp);
        }
    }
}
void Paint_GrayScale(Renderer& r, const Graphics::Image& img, int x, int y, uint32 rap)
{
    const auto w    = img.GetWidth();
    const auto h    = img.GetHeight();
    int px          = 0;
    ColorPair cp    = NoColorPair;
    SpecialChars sc = SpecialChars::Block100;
    for (uint32 img_y = 0; img_y < h; img_y += rap, y++)
    {
        px = x;
        for (uint32 img_x = 0; img_x < w; img_x += rap, px += 2)
        {
            if (rap == 1)
                PixelToGrayScaleCharacter(img.GetPixel(img_x, img_y), cp, sc);
            else
                PixelToGrayScaleCharacter(img.ComputeSquareAverageColor(img_x, img_y, rap), cp, sc);

            // r.FillHorizontalLineWithSpecialChar(px, y, px + 1, sc, ColorPair{ cp, Color::Black });
            r.FillHorizontalLineWithSpecialChar(px, y, px + 1, sc, cp);
        }
    }
}
Size Renderer::ComputeRenderingSize(const Image& img, ImageRenderingMethod method, ImageScaleMethod scale)
{
    auto rap = static_cast<uint32>(scale);
    uint32 w = img.GetWidth();
    uint32 h = img.GetHeight();

    // sanity check
    CHECK((rap >= 1) && (rap <= 20), Size(), "Invalid scale enum value");

    switch (method)
    {
    case ImageRenderingMethod::PixelTo16ColorsSmallBlock:
        w = w / rap;
        h = h / (2 * rap);
        break;
    case ImageRenderingMethod::PixelTo64ColorsLargeBlock:
    case ImageRenderingMethod::GrayScale:
        w = (w * 2) / rap;
        h = h / rap;
        break;
    case ImageRenderingMethod::AsciiArt:
        NOT_IMPLEMENTED(Size());
    default:
        NOT_IMPLEMENTED(Size());
    };
    w = std::max<>(w, 1U);
    h = std::max<>(h, 1U);
    return Size(w, h);
}
bool Renderer::DrawImage(const Image& img, int x, int y, ImageRenderingMethod method, ImageScaleMethod scale)
{
    auto rap = static_cast<uint32>(scale);
    // sanity check
    CHECK((rap >= 1) && (rap <= 20), false, "Invalid scale enum value");

    switch (method)
    {
    case ImageRenderingMethod::PixelTo16ColorsSmallBlock:
        Paint_SmallBlocks(*this, img, x, y, rap);
        return true;
    case ImageRenderingMethod::PixelTo64ColorsLargeBlock:
        Paint_LargeBlocks(*this, img, x, y, rap);
        return true;
    case ImageRenderingMethod::GrayScale:
        Paint_GrayScale(*this, img, x, y, rap);
        return true;
    case ImageRenderingMethod::AsciiArt:
        NOT_IMPLEMENTED(false);
    default:
        RETURNERROR(false, "Unknwon rendering method (%u) ", static_cast<uint32>(method));
    }
}
} // namespace AppCUI::Graphics

bool AppCUI::Application::SetSpecialCharacterSet(AppCUI::Application::SpecialCharacterSetType charSetType)
{
    if (charSetType == AppCUI::Application::SpecialCharacterSetType::Auto)
    {
        auto app = AppCUI::Application::GetApplication();
        CHECK(app, false, "Application was not initialized");
        CHECK(app->terminal, false, "No terminal was associated/linked to current app");
        // check if support is available for different special character types
        Application::SpecialCharacterSetType order[] = { SpecialCharacterSetType::Unicode,
                                                         SpecialCharacterSetType::LinuxTerminal,
                                                         SpecialCharacterSetType::Ascii };
        bool found                                   = false;
        for (auto index = 0U; index < ARRAY_LEN(order); index++)
        {
            if (app->terminal->HasSupportFor(order[index]))
            {
                charSetType = order[index];
                found       = true;
                break;
            }
        }
        CHECK(found, false, "Current terminal does not support any special character set !");
    }
    switch (charSetType)
    {
    case AppCUI::Application::SpecialCharacterSetType::Unicode:
        AppCUI::Graphics::SpecialCharacters = AppCUI::Graphics::UnicodeSpecialChars::special_characters;
        AppCUI::Graphics::LineSpecialChars  = AppCUI::Graphics::UnicodeSpecialChars::line_types_chars;
        break;
    case AppCUI::Application::SpecialCharacterSetType::LinuxTerminal:
        AppCUI::Graphics::SpecialCharacters = AppCUI::Graphics::LinuxTerminal::special_characters;
        AppCUI::Graphics::LineSpecialChars  = AppCUI::Graphics::LinuxTerminal::line_types_chars;
        break;
    case AppCUI::Application::SpecialCharacterSetType::Ascii:
        AppCUI::Graphics::SpecialCharacters = AppCUI::Graphics::AsciiSpecialChars::special_characters;
        AppCUI::Graphics::LineSpecialChars  = AppCUI::Graphics::AsciiSpecialChars::line_types_chars;
        break;
    default:
        RETURNERROR(false, "Unknwon special character set --> this is a fallback case, it should not be reached !");
        break;
    }
    return true;
}

#undef COMPUTE_RGB