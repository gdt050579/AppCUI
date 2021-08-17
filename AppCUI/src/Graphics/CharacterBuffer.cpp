#include "AppCUI.hpp"
#include "Internal.hpp"
#include <string.h>

using namespace AppCUI::Graphics;
using namespace AppCUI::Utils;



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

constexpr bool IgnoreCaseEquals(char16_t code1, char16_t code2);

template<typename T>
size_t CopyStringToCharBuffer(Character* dest, const T* source, size_t sourceCharactersCount, ColorPair col)
{
    // it is iassume that dest, source and sourceCharactersCount are valid values
    // all new-line formats will be converted into a single separator 
    const T* end = source + sourceCharactersCount;
    Character* ch_start = dest;
    while (source < end)
    {
        dest->Color = col;
        if ((*source) == '\r')
        {
            dest->Code = NEW_LINE_CODE;
            dest++;
            source++;
            if ((source < end) && ((*source) == '\n'))
                source++;
            continue;
        }
        if ((*source) == '\n')
        {
            dest->Code = NEW_LINE_CODE;
            dest++;
            source++;
            if ((source < end) && ((*source) == '\r'))
                source++;
            continue;
        }
        // no new_line --> direct copy
        dest->Code = (char16_t) (*source);
        dest++;
        source++;
    }
    return dest - ch_start;
}
template <typename T>
size_t CopyStringToCharBufferWidthHotKey(Character* dest, const T* source, size_t sourceCharactersCount, ColorPair col, unsigned int &hotKeyPos)
{
    // it is iassume that dest, source and sourceCharactersCount are valid values
    // all new-line formats will be converted into a single separator
    const T* end        = source + sourceCharactersCount;
    const T* start      = source;
    Character* ch_start = dest;
    bool hotKeyFound    = false;
    while (source < end)
    {
        dest->Color = col;
        if ((*source) == '\r')
        {
            dest->Code = NEW_LINE_CODE;
            dest++;
            source++;
            if ((source < end) && ((*source) == '\n'))
                source++;
            continue;
        }
        if ((*source) == '\n')
        {
            dest->Code = NEW_LINE_CODE;
            dest++;
            source++;
            if ((source < end) && ((*source) == '\r'))
                source++;
            continue;
        }
        if ((!hotKeyFound) && ((*source) == '&') && ((source + 1) < end))
        {
            if (((source[1] >= 'A') && (source[1] <='Z')) ||
                ((source[1] >= 'a') && (source[1] <='z')) ||
                ((source[1] >= '0') && (source[1] <='9')))
            {
                // found a hotkey
                hotKeyPos = (unsigned int) (source - start);
                source++; // skip `&` character
                dest->Code = (char16_t) (*source);
                dest++;
                source++;
                hotKeyFound = true;
                continue;
            }
        }
        // no new_line --> direct copy
        dest->Code = (char16_t) (*source);
        dest++;
        source++;
    }
    return dest - ch_start;
}
template <typename T>
int FindInCharacterBuffer(const T& sv, const CharacterView &charView, bool ignoreCase)
{
    auto p                     = sv.data();
    auto p_end                 = p + sv.length();
    const Character* ch        = charView.data();
    const Character* ch_end    = ch + charView.length();
    CHECK(p, -1, "Expecting a valid (non_null) text !");
    CHECK(ch, -1, "Invalid buffer (not set)");
    if (p == p_end)
        return 0; // am empty string is always found at the first position
    if (sv.size() > charView.length())
        return -1;
    const Character* ch_max_search = ch_end - sv.length();
    // the actual search
    if (ignoreCase)
    {
        while (ch <= ch_max_search)
        {
            auto s = p;
            auto c = ch;
            while ((s < p_end) && (c < ch_end) && (IgnoreCaseEquals(*s, c->Code)))
            {
                c++;
                s++;
            }
            if (s >= p_end)
                return (int) (ch - charView.data());
            ch++;
        }
    }
    else
    {
        while (ch <= ch_max_search)
        {
            auto s = p;
            auto c = ch;
            while ((s < p_end) && (c < ch_end) && ((*s) == c->Code))
            {
                c++;
                s++;
            }
            if (s >= p_end)
                return (int) (ch - charView.data());
            ch++;
        }
    }
    return -1;
}


constexpr bool IgnoreCaseEquals(char16_t code1, char16_t code2)
{
    // GDT: temporary implementation ==> should be addapted for UNICODE characters as well
    if ((code1 >= 'A') && (code1 <= 'Z'))
        code1 |= 0x20;
    if ((code2 >= 'A') && (code2 <= 'Z'))
        code2 |= 0x20;
    return code1 == code2;
}

CharacterBuffer::CharacterBuffer()
{
    //LOG_INFO("Default ctor for %p", this);
    Allocated = 0;
    Count     = 0;
    Buffer    = nullptr;
}

CharacterBuffer::~CharacterBuffer(void)
{
    //LOG_INFO("DTOR for %p [Buffer=%p,Count=%d,Allocated=%d]", this, Buffer, (int) Count, (int) Allocated);
    Destroy();
}


void CharacterBuffer::Swap(CharacterBuffer& obj) noexcept
{
    // LOG_INFO("Swap %p with %p", this, &obj);
    std::swap(this->Buffer, obj.Buffer);
    std::swap(this->Allocated, obj.Allocated);
    std::swap(this->Count, obj.Count);
}
void CharacterBuffer::Destroy()
{
    //LOG_INFO("Destroy Character Buffer from (%p) with [Buffer = %p, Count = %d, Allocated = %d]",this, Buffer, (int)Count, (int)Allocated);
    if (Buffer)
        delete []Buffer;
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
        delete []Buffer;
    }
    Buffer    = temp;
    Allocated = newSize;
    return true;
}

bool CharacterBuffer::Set(const CharacterBuffer& obj)
{
    this->Count = 0; // we overwrite the buffer anyway - don't need to recopy it if the allocated space is too small
    CHECK(Grow(obj.Count), false, "Fail to allocate %d bytes ", (int) obj.Count);
    if (obj.Count>0)
    {
        memcpy(this->Buffer, obj.Buffer, sizeof(Character) * obj.Count);   
    }
    this->Count = obj.Count;
    return true;
}
bool CharacterBuffer::Add(const AppCUI::Utils::ConstString& text, const ColorPair color)
{
    AppCUI::Utils::ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;
    
    if (textObj.Length == 0)
        return true; // nothing to do
    CHECK(textObj.Data, false, "Expecting a valid (non-null) string");
    CHECK(Grow(this->Count + textObj.Length),false, "Fail to allocate space for the character buffer: %z", this->Count + textObj.Length);

    size_t sz;

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        sz = CopyStringToCharBuffer<char>(this->Buffer + this->Count, (const char*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::CharacterBuffer:
        sz = CopyStringToCharBuffer<Character>(this->Buffer + this->Count, (const Character*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::Unicode16:
        sz = CopyStringToCharBuffer<char16_t>(this->Buffer + this->Count, (const char16_t*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), false, "Fail to convert UTF-8 to current internal format !");
        sz = CopyStringToCharBuffer<char16_t>(this->Buffer + this->Count, ub.GetString(), ub.Len(), color);
        break;
    default:
        RETURNERROR(false, "Unknwon string encoding type: %d", textObj.Encoding);
    }
    CHECK(sz <= textObj.Length, false, "Internal error --> possible buffer overwrite !");
    this->Count += sz;
    return true;
}
bool CharacterBuffer::Set(const AppCUI::Utils::ConstString& text, const ColorPair color)
{
    this->Count = 0;
    return Add(text, color);
}
bool CharacterBuffer::SetWithHotKey(const AppCUI::Utils::ConstString& text, unsigned int& hotKeyCharacterPosition,const ColorPair color)
{
    AppCUI::Utils::ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;
    hotKeyCharacterPosition = CharacterBuffer::INVALID_HOTKEY_OFFSET;

    if (textObj.Length == 0)
        return true; // nothing to do
    CHECK(textObj.Data, false, "Expecting a valid (non-null) string");
    CHECK(Grow(textObj.Length), false, "Fail to allocate space for the character buffer: %z", textObj.Length);

    size_t sz;

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        sz = CopyStringToCharBufferWidthHotKey<char>(this->Buffer + this->Count, (const char*) textObj.Data, textObj.Length, color, hotKeyCharacterPosition);
        break;
    case StringEncoding::CharacterBuffer:
        sz = CopyStringToCharBufferWidthHotKey<Character>(this->Buffer + this->Count, (const Character*) textObj.Data, textObj.Length, color, hotKeyCharacterPosition);
        break;
    case StringEncoding::Unicode16:
        sz = CopyStringToCharBufferWidthHotKey<char16_t>(this->Buffer + this->Count, (const char16_t*) textObj.Data, textObj.Length, color, hotKeyCharacterPosition);
        break;
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), false, "Fail to convert UTF-8 to current internal format !");
        sz = CopyStringToCharBufferWidthHotKey<char16_t>(this->Buffer + this->Count, ub.GetString(), ub.Len(), color, hotKeyCharacterPosition);
        break;
    default:
        RETURNERROR(false, "Unknwon string encoding type: %d", textObj.Encoding);
    }
    CHECK(sz <= textObj.Length, false, "Internal error --> possible buffer overwrite !");
    this->Count += sz;
    return true;
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
bool CharacterBuffer::Insert(const AppCUI::Utils::ConstString& text, unsigned int position, const ColorPair color)
{
    AppCUI::Utils::ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;

    if (textObj.Length == 0)
        return true; // nothing to do

    CHECK(textObj.Data, false, "Expecting a valid (non-null) string");
    CHECK(position <= this->Count, false, "Invalid insert offset: %d (should be between 0 and %d)", position, this->Count);
    CHECK(Grow(this->Count + textObj.Length), false, "Fail to allocate space for the character buffer: %z", this->Count + textObj.Length);

    // make space for new data
    if (position < this->Count)
    {
        memmove(this->Buffer + position + textObj.Length, this->Buffer + position, (this->Count - position) * sizeof(Character));
    }

    size_t writtenChars;

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        writtenChars = CopyStringToCharBuffer<char>(this->Buffer + position, (const char*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::CharacterBuffer:
        writtenChars = CopyStringToCharBuffer<Character>(this->Buffer + position, (const Character*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::Unicode16:
        writtenChars = CopyStringToCharBuffer<char16_t>(this->Buffer + position, (const char16_t*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), false, "Fail to convert UTF-8 to current internal format !");
        writtenChars = CopyStringToCharBuffer<char16_t>(this->Buffer + position, ub.GetString(), ub.Len(), color);
        break;
    default:
        RETURNERROR(false, "Unknwon string encoding type: %d", textObj.Encoding);
    }
    CHECK(writtenChars <= textObj.Length, false, "Internal error => possible buffer overwrite !");
    if (writtenChars<textObj.Length)
    {
        // fewer chars were actually writtem
        memmove(
              this->Buffer + position + writtenChars,
              this->Buffer + position + textObj.Length,
              (this->Count - position) * sizeof(Character));
    }
    this->Count += writtenChars;
    return true;
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
    AppCUI::Utils::ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;

    CHECK(textObj.Data, -1, "Expecting a valid (non-null) string");
    if (textObj.Length == 0)
        return 0; // nothing to do

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        return FindInCharacterBuffer<std::string_view>(std::get<std::string_view>(text), *this, ignoreCase);
    case StringEncoding::CharacterBuffer:
        return FindInCharacterBuffer<CharacterView>(std::get<CharacterView>(text), *this, ignoreCase);
    case StringEncoding::Unicode16:
        return FindInCharacterBuffer<std::u16string_view>(std::get<std::u16string_view>(text), *this, ignoreCase);
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), -1, "Fail to convert UTF-8 to current internal format !");
        return FindInCharacterBuffer<std::u16string_view>(ub.ToStringView(), *this, ignoreCase);
    default:
        RETURNERROR(-1, "Unknwon string encoding type: %d", textObj.Encoding);
    }
}
int  CharacterBuffer::CompareWith(const CharacterBuffer& obj, bool ignoreCase) const
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


bool CharacterBuffer::ToString(std::string& output) const
{
    CHECK(this->Buffer, false, "");
    const Character* p = this->Buffer;
    const Character* e = p + this->Count;
    output.reserve(this->Count);
    output = "";
    while (p<e)
    {
        if (p->Code<256)
            output.append(1, (char) p->Code);
        else
            output.append(1, '?');
        p++;
    }
    return true;
}
bool CharacterBuffer::ToString(std::u16string& output) const
{
    CHECK(this->Buffer, false, "");
    const Character* p = this->Buffer;
    const Character* e = p + this->Count;
    output.reserve(this->Count);
    char16_t tmp[1] = { 0 };
    output          = tmp;
    while (p < e)
    {
        output.append(1, p->Code);
        p++;
    }
    return true;
}
bool CharacterBuffer::ToPath(std::filesystem::path& output) const
{
    CHECK(this->Buffer, false, "");
    const Character* p = this->Buffer;
    const Character* e = p + this->Count;
    output             = "";
    while (p < e)
    {
        output += p->Code;
        p++;
    }
    return true;
}