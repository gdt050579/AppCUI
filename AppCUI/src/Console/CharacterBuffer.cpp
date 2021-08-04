#include "AppCUI.hpp"
#include "Internal.hpp"
#include <string.h>

using namespace AppCUI::Console;

struct UnicodeChar
{
    unsigned short Value;
    unsigned int Length;
};

#define VALIDATE_ALLOCATED_SPACE(requiredSpace, returnValue)                                                           \
    if ((requiredSpace) > (Allocated & 0x7FFFFFFF))                                                                    \
    {                                                                                                                  \
        CHECK(Grow(requiredSpace), returnValue, "Fail to allocate space for %d bytes", (requiredSpace));               \
    }

#define COMPUTE_TEXT_SIZE(text, textSize)                                                                              \
    if (textSize == 0xFFFFFFFF)                                                                                        \
    {                                                                                                                  \
        textSize = AppCUI::Utils::String::Len(text);                                                                   \
    }

bool UTF8_to_Unicode(const char8_t* p, const char8_t* end, UnicodeChar& result)
{
    // unicode encoding
    if (((*p) >> 5) == 6) // binary encoding 110xxxxx, followed by 10xxxxxx
    {
        CHECK(p + 1 < end, false, "Invalid unicode sequence (missing one extra character after 110xxxx)");
        CHECK((p[1] >> 6) == 2, false, "Invalid unicode sequence (110xxxx should be followed by 10xxxxxx)");
        result.Value = (((unsigned short) ((*p) & 0x1F)) << 6) | ((unsigned short) ((*(p + 1)) & 63));
        result.Length = 2;
        return true;
    }
    if (((*p) >> 4) == 14) // binary encoding 1110xxxx, followed by 2 bytes with 10xxxxxx
    {
        CHECK(p + 2 < end, false, "Invalid unicode sequence (missing two extra characters after 1110xxxx)");
        CHECK((p[1] >> 6) == 2, false, "Invalid unicode sequence (1110xxxx should be followed by 10xxxxxx)");
        CHECK((p[2] >> 6) == 2, false, "Invalid unicode sequence (10xxxxxx should be followed by 10xxxxxx)");
        result.Value = (((unsigned short) ((*p) & 0x0F)) << 12) | (((unsigned short) ((*(p + 1)) & 63)) << 6) |
                       ((unsigned short) ((*(p + 2)) & 63));
        result.Length = 3;
        return true;
    }
    if (((*p) >> 3) == 30) // binary encoding 11110xxx, followed by 3 bytes with 10xxxxxx
    {
        CHECK(p + 3 < end, false, "Invalid unicode sequence (missing two extra characters after 11110xxx)");
        CHECK((p[1] >> 6) == 2, false, "Invalid unicode sequence (11110xxx should be followed by 10xxxxxx)");
        CHECK((p[2] >> 6) == 2, false, "Invalid unicode sequence (10xxxxxx should be followed by 10xxxxxx)");
        CHECK((p[3] >> 6) == 2, false, "Invalid unicode sequence (10xxxxxx should be followed by 10xxxxxx)");
        result.Value = (((unsigned short) ((*p) & 7)) << 18) | (((unsigned short) ((*(p + 1)) & 63)) << 12) |
                       (((unsigned short) ((*(p + 2)) & 63)) << 6) | ((unsigned short) ((*(p + 3)) & 63));
        result.Length = 4;
        return true;
    }
    // invalid 16 bytes encoding
    RETURNERROR(false, "Invalid UTF-8 encoding ");
}


CharacterBuffer::CharacterBuffer()
{
    Allocated = 0;
    Count     = 0;
    Buffer    = nullptr;
}
void CharacterBuffer::MakeACopy(const CharacterBuffer& obj)
{
    this->Allocated = obj.Allocated;
    this->Count     = obj.Count;
    if (obj.Buffer)
    {
        this->Buffer = new Character[obj.Allocated];
        if (this->Buffer)
        {
            memcpy(this->Buffer, obj.Buffer, sizeof(Character) * obj.Count);
            this->Allocated = obj.Allocated;
            this->Count     = obj.Count;
            return;
        }
    }
    this->Allocated = 0;
    this->Count     = 0;
    this->Buffer    = nullptr;
}
void CharacterBuffer::Swap(CharacterBuffer& obj) noexcept
{
    this->Allocated = obj.Allocated;
    this->Count     = obj.Count;
    this->Buffer    = obj.Buffer;
    obj.Allocated   = 0;
    obj.Count       = 0;
    obj.Buffer      = nullptr;
}


CharacterBuffer::~CharacterBuffer(void)
{
    Destroy();
}



void CharacterBuffer::Destroy()
{
    if (Buffer)
        delete Buffer;
    Buffer = nullptr;
    Count = Allocated = 0;
}
bool CharacterBuffer::Grow(size_t newSize)
{
    newSize = ((newSize | 15) + 1) & 0x7FFFFFFF;
    if (newSize <= (Allocated & 0x7FFFFFFF))
        return true;
    Character* temp = new Character[newSize];
    CHECK(temp, false, "Failed to allocate: %d characters", newSize);
    if (Buffer)
    {
        memcpy(temp, Buffer, sizeof(Character) * this->Count);
        delete Buffer;
    }
    Buffer    = temp;
    Allocated = newSize;
    return true;
}
bool CharacterBuffer::Add(const std::string_view text, const ColorPair color)
{
    CHECK(text.data(), false, "Expecting a valid (non-null) text");
    VALIDATE_ALLOCATED_SPACE(text.size() + this->Count, false);
    Character* ch              = this->Buffer + this->Count;
    const unsigned char* p     = (const unsigned char*) text.data();
    auto textSize              = text.size();

    this->Count += text.size();
    // ASCII-Z format
    while (textSize > 0)
    {
        ch->Color = color;
        ch->Code  = *p;
        ch++;
        p++;
        textSize--;
    }
    return true;
}
bool CharacterBuffer::Add(const std::u8string_view text, const ColorPair color)
{
    CHECK(text.data(), false, "Expecting a valid (non-null) text");
    VALIDATE_ALLOCATED_SPACE(text.size() + this->Count, false);
    Character* ch              = this->Buffer + this->Count;
    const char8_t* p           = (const char8_t*) text.data();
    const char8_t* p_end       = p + text.size();
    auto textSize              = text.size();
    Character* ch_start        = ch;

    UnicodeChar uc;

    while (textSize > 0)
    {
        ch->Color = color;
        if ((*p) < 0x80)
        {
            ch->Code = *p;
            p++;
            ch++;
            textSize--;
        }
        else
        {
            // unicode encoding
            CHECK(UTF8_to_Unicode(p, p_end, uc), false, "Fail to convert to unicode !");
            ch->Code = uc.Value;
            textSize -= uc.Length;
            p += uc.Length;
            ch++;
        }
    }
    // adjust size
    this->Count += (ch - ch_start);

    return true;
}
bool CharacterBuffer::Set(const std::string_view text, const ColorPair color)
{
    this->Count = 0;
    return Add(text, color);
}
bool CharacterBuffer::Set(const std::u8string_view text, const ColorPair color)
{
    this->Count = 0;
    return Add(text, color);
}
bool CharacterBuffer::SetWithHotKey(const std::string_view text, unsigned int& hotKeyCharacterPosition, const ColorPair color)
{
    hotKeyCharacterPosition = 0xFFFFFFFF;
    CHECK(text.data(), false, "Expecting a valid (non-null) text");
    VALIDATE_ALLOCATED_SPACE(text.size() + this->Count, false);
    Character* ch              = this->Buffer;
    const unsigned char* p     = (const unsigned char*) text.data();
    auto textSize              = text.size();

    while (textSize > 0)
    {
        if ((hotKeyCharacterPosition == 0xFFFFFFFF) && ((*p) == '&') && (textSize > 1 /* not the last character*/))
        {
            char tmp = p[1] | 0x20;
            if (((tmp >= 'a') && (tmp <= 'z')) || ((tmp >= '0') && (tmp <= '9')))
            {
                hotKeyCharacterPosition = (unsigned int) (p - (const unsigned char*) text.data());
                p++;
                textSize--;
                continue;
            }
        }
        ch->Color = color;
        ch->Code  = *p;
        ch++;
        p++;
        textSize--;
    }
    this->Count = (unsigned int) (ch - this->Buffer);
    return true;
}
bool CharacterBuffer::SetWithHotKey(const std::u8string_view text, unsigned int& hotKeyCharacterPosition, const ColorPair color)
{
    hotKeyCharacterPosition = 0xFFFFFFFF;
    CHECK(text.data(), false, "Expecting a valid (non-null) text");
    VALIDATE_ALLOCATED_SPACE(text.size() + this->Count, false);
    Character* ch              = this->Buffer;
    const char8_t* p           = text.data();
    const char8_t* p_end       = p + text.size();
    auto textSize              = text.size();
    UnicodeChar uc;


    while (textSize > 0)
    {
        if ((hotKeyCharacterPosition == 0xFFFFFFFF) && ((*p) == '&') && (textSize > 1 /* not the last character*/))
        {
            char8_t tmp = p[1] | 0x20;
            if (((tmp >= 'a') && (tmp <= 'z')) || ((tmp >= '0') && (tmp <= '9')))
            {
                hotKeyCharacterPosition = (unsigned int) (ch - this->Buffer);
                p++;
                textSize--;
                continue;
            }
        }
        ch->Color = color;
        if ((*p) < 0x80)
        {
            ch->Code = *p;
            p++;
            ch++;
            textSize--;
        }
        else
        {
            // unicode encoding
            CHECK(UTF8_to_Unicode(p, p_end, uc), false, "Fail to convert to unicode !");
            ch->Code = uc.Value;
            textSize -= uc.Length;
            p += uc.Length;
            ch++;
        }
    }
    this->Count = (unsigned int) (ch - this->Buffer);
    
    return true;
}
bool CharacterBuffer::SetWithNewLines(const std::u8string_view text, const ColorPair color)
{
    CHECK(text.data(), false, "Expecting a valid (non-null) text");
    VALIDATE_ALLOCATED_SPACE(text.size() + this->Count, false);
    Character* ch              = this->Buffer;
    const char8_t* p           = text.data();
    const char8_t* p_end       = p + text.size();
    auto textSize              = text.size();
    UnicodeChar uc;

    while (textSize > 0)
    {
        if ((*p) == '\r')
        {
            ch->Color = color;
            ch->Code  = NEW_LINE_CODE;
            p++;
            ch++;
            textSize--;
            if ((textSize > 0) && ((*p) == '\n'))
            {
                // skip
                p++;
                textSize--;
            }
            continue;
        }
        if ((*p) == '\n')
        {
            ch->Color = color;
            ch->Code  = NEW_LINE_CODE;
            p++;
            ch++;
            textSize--;
            if ((textSize > 0) && ((*p) == '\r'))
            {
                // skip
                p++;
                textSize--;
            }
            continue;
        }

        ch->Color = color;

        if ((*p) < 0x80)
        {
            ch->Code = *p;
            p++;
            textSize--;
        }
        else
        {
            // unicode encoding
            CHECK(UTF8_to_Unicode(p, p_end, uc), false, "Fail to convert to unicode !");
            ch->Code = uc.Value;
            textSize -= uc.Length;
            p += uc.Length;
        }

        ch++;
    }
    this->Count = (unsigned int) (ch - this->Buffer);
    return true;
}
bool CharacterBuffer::SetWithNewLines(const std::string_view text, const ColorPair color)
{
    CHECK(text.data(), false, "Expecting a valid (non-null) text");
    VALIDATE_ALLOCATED_SPACE(text.size() + this->Count, false);
    Character* ch              = this->Buffer;
    const unsigned char* p     = (const unsigned char*) text.data();
    const unsigned char* p_end = p + text.size();
    auto textSize              = text.size();

    while (textSize > 0)
    {
        if ((*p) == '\r')
        {
            ch->Color = color;
            ch->Code  = NEW_LINE_CODE;
            p++;
            ch++;
            textSize--;
            if ((textSize > 0) && ((*p) == '\n'))
            {
                // skip
                p++;
                textSize--;
            }
            continue;
        }
        if ((*p) == '\n')
        {
            ch->Color = color;
            ch->Code  = NEW_LINE_CODE;
            p++;
            ch++;
            textSize--;
            if ((textSize > 0) && ((*p) == '\r'))
            {
                // skip
                p++;
                textSize--;
            }
            continue;
        }

        ch->Color = color;
        ch->Code = *p;
        p++;
        textSize--;        
        ch++;
    }
    this->Count = (unsigned int) (ch - this->Buffer);
    return true;
}
int  CharacterBuffer::FindAscii(const std::string_view & text, bool ignoreCase) const
{
    const unsigned char* p     = (const unsigned char*) text.data();
    const unsigned char* p_end = p + text.size();
    Character* ch              = this->Buffer;
    Character* ch_end          = this->Buffer + this->Count;
    CHECK(p, -1, "Expecting a valid (non_null) text !");
    CHECK(ch, -1, "Invalid buffer (not set)");
    if (p == p_end)
        return 0; // am empty string is always found
    // the actual search
    //while (ch<ch_end)
    //{
    //
    //}
    return -1;
}
int  CharacterBuffer::FindUTF8(const std::u8string_view& text, bool ignoreCase) const
{
    NOT_IMPLEMENTED(-1);
}
bool CharacterBuffer::Add(const AppCUI::Utils::ConstString& text, const ColorPair color)
{
    if (std::holds_alternative<std::u8string_view>(text))
    {
        return Add(std::get<std::u8string_view>(text), color);
    }
    else
    {
        return Add(std::get<std::string_view>(text), color);
    }
}
bool CharacterBuffer::Set(const AppCUI::Utils::ConstString& text, const ColorPair color)
{
    if (std::holds_alternative<std::u8string_view>(text))
    {
        return Set(std::get<std::u8string_view>(text), color);
    }
    else
    {
        return Set(std::get<std::string_view>(text), color);
    }
}
bool CharacterBuffer::SetWithHotKey(const AppCUI::Utils::ConstString& text, unsigned int& hotKeyCharacterPosition,const ColorPair color)
{
    if (std::holds_alternative<std::u8string_view>(text))
    {
        return SetWithHotKey(std::get<std::u8string_view>(text), hotKeyCharacterPosition, color);
    }
    else
    {
        return SetWithHotKey(std::get<std::string_view>(text), hotKeyCharacterPosition, color);
    }
}
bool CharacterBuffer::SetWithNewLines(const AppCUI::Utils::ConstString& text, const ColorPair color)
{
    if (std::holds_alternative<std::u8string_view>(text))
    {
        return SetWithNewLines(std::get<std::u8string_view>(text), color);
    }
    else
    {
        return SetWithNewLines(std::get<std::string_view>(text), color);
    }
}
void CharacterBuffer::Clear()
{
    this->Count = 0;
}
bool CharacterBuffer::Delete(unsigned int start, unsigned int end)
{
    CHECK(end <= this->Count, false, "Invalid delete offset: %d (should be between 0 and %d)", end, this->Count);
    CHECK(start < end, false, "Start parameter (%d) should be smaller than End parameter (%d)", start, end);
    if (end == this->Count)
    {
        this->Count = start;
    }
    else
    {
        memmove(this->Buffer + start, this->Buffer + end, (this->Count - end) * sizeof(Character));
        this->Count -= (end - start);
    }
    return true;
}
bool CharacterBuffer::DeleteChar(unsigned int position)
{
    CHECK(position < this->Count,
          false,
          "Invalid delete offset: %d (should be between 0 and %d)",
          position,
          this->Count - 1);
    if ((position + 1) < this->Count)
    {
        memmove(
              this->Buffer + position, this->Buffer + position + 1, (this->Count - (position + 1)) * sizeof(Character));
    }
    this->Count--;
    return true;
}
bool CharacterBuffer::Insert(const char* text, unsigned int position, const ColorPair color, unsigned int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text");
    CHECK(position <= this->Count,
          false,
          "Invalid insert offset: %d (should be between 0 and %d)",
          position,
          this->Count);
    COMPUTE_TEXT_SIZE(text, textSize);
    if (textSize == 0)
        return true; // nothing to insert
    VALIDATE_ALLOCATED_SPACE(textSize + this->Count, false);
    if (position < this->Count)
    {
        memmove(
              this->Buffer + position + textSize,
              this->Buffer + position,
              (this->Count - position) * sizeof(Character));
    }
    auto c = this->Buffer + position;
    this->Count += textSize;
    while (textSize > 0)
    {
        c->Code  = *text;
        c->Color = color;
        text++;
        c++;
        textSize--;
    }
    return true;
}
bool CharacterBuffer::Insert(const AppCUI::Utils::String& text, unsigned int position, const ColorPair color)
{
    return Insert(text.GetText(), position, color, text.Len());
}
bool CharacterBuffer::InsertChar(unsigned short characterCode, unsigned int position, const ColorPair color)
{
    VALIDATE_ALLOCATED_SPACE(this->Count + 1, false);
    CHECK(position <= this->Count,
          false,
          "Invalid insert offset: %d (should be between 0 and %d)",
          position,
          this->Count);
    if (position < this->Count)
    {
        memmove(this->Buffer + position + 1, this->Buffer + position, (this->Count - position) * sizeof(Character));
    }
    auto c   = this->Buffer + position;
    c->Code  = characterCode;
    c->Color = color;
    this->Count++;
    return true;
}
bool CharacterBuffer::SetColor(unsigned int start, unsigned int end, const ColorPair color)
{
    if (end > this->Count)
        end = this->Count;
    CHECK(start <= end, false, "Expecting a valid parameter for start (%d) --> should be smaller than %d", start, end);
    Character* ch   = this->Buffer + start;
    unsigned int sz = end - start;
    while (sz)
    {
        ch->Color = color;
        sz--;
        ch++;
    }
    return true;
}
void CharacterBuffer::SetColor(const ColorPair color)
{
    Character* ch = this->Buffer;
    size_t sz     = this->Count;
    while (sz)
    {
        ch->Color = color;
        sz--;
        ch++;
    }
}
bool CharacterBuffer::CopyString(AppCUI::Utils::String& text, unsigned int start, unsigned int end)
{
    CHECK(start < end, false, "Start position (%d) should be smaller than the end position (%d)", start, end);
    CHECK(end <= Count,
          false,
          "Invalid end position (%d), should be smaller or equal to maximum characters in the buffer (%d)",
          end,
          Count);
    CHECK(text.Realloc(end - start), false, "Fail to allocate %d character to be copied", end - start);

    Character* s = this->Buffer + start;
    Character* e = s + (end - start);
    text.Clear();
    while (s < e)
    {
        CHECK(text.AddChar((char) s->Code), false, "Fail to copy character with code: %d to string", s->Code);
        s++;
    }
    return true;
}
bool CharacterBuffer::CopyString(AppCUI::Utils::String& text)
{
    return CopyString(text, 0, this->Count);
}
int  CharacterBuffer::Find(const AppCUI::Utils::ConstString& text, bool ignoreCase) const
{
    if (std::holds_alternative<std::u8string_view>(text))
    {
        return FindUTF8(std::get<std::u8string_view>(text), ignoreCase);
    }
    else
    {
        return FindAscii(std::get<std::string_view>(text), ignoreCase);
    }
}
int CharacterBuffer::CompareWith(const CharacterBuffer& obj, bool ignoreCase) const
{
    Character* s = this->Buffer;
    Character* s_end = s + this->Count;
    Character* d     = obj.Buffer;
    Character* d_end = d + obj.Count;

    // null check
    if ((s) && (!d))
        return 1;
    if ((!s) && (d))
        return -1;
    if ((!s) && (!d))
        return 0;


    if (!ignoreCase)
    {
        while ((s<s_end) && (d<d_end))    
        {
            if (s->Code < d->Code)
                return -1;
            if (s->Code > d->Code)
                return 1;
            s++;
            d++;
        }
    }
    else
    {
        char16_t s_c, d_c;
        while ((s < s_end) && (d < d_end))
        {
            s_c = s->Code;
            d_c = d->Code;
            if ((s_c >= 'A') && (s_c <= 'Z'))
                s_c |= 0x20;
            if ((d_c >= 'A') && (d_c <= 'Z'))
                d_c |= 0x20;
            if (s_c < d_c)
                return -1;
            if (s_c > d_c)
                return 1;
            s++;
            d++;
        }  
    }
    if ((s == s_end) && (d < d_end))
        return -1;
    if ((s < s_end) && (d == d_end))
        return 1;
    return 0;
}