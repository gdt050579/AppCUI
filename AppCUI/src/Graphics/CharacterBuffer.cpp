#include <AppCUI.hpp>
#include "Internal.hpp"
#include <string.h>

namespace AppCUI
{
using namespace Graphics;
using namespace Utils;

// 24 bits for max size
// it is important that maxSize (24 bits) x sizeof(Character) (4) cand always be repesented on 32 bits
#define MAX_ALLOCATION_SIZE 0x00FFFFFF
#define ALOCATION_MASK      0x00FFFFFF

#define VALIDATE_ALLOCATED_SPACE(requiredSpace, returnValue)                                                           \
    if ((requiredSpace) > (Allocated & ALOCATION_MASK))                                                                \
    {                                                                                                                  \
        CHECK(Grow(requiredSpace), returnValue, "Fail to allocate space for %z bytes", (size_t) (requiredSpace));      \
    }

constexpr bool IgnoreCaseEquals(char16 code1, char16 code2);

template <typename T>
size_t CopyStringToCharBuffer(Character* dest, const T* source, size_t sourceCharactersCount, ColorPair col)
{
    // it is iassume that dest, source and sourceCharactersCount are valid values
    // all new-line formats will be converted into a single separator
    const T* end        = source + sourceCharactersCount;
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
        dest->Code = (char16) (*source);
        dest++;
        source++;
    }
    return dest - ch_start;
}
template <typename T>
size_t CopyStringToCharBufferWidthHotKey(
      Character* dest, const T* source, size_t sourceCharactersCount, ColorPair col, unsigned int& hotKeyPos)
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
            if (((source[1] >= 'A') && (source[1] <= 'Z')) || ((source[1] >= 'a') && (source[1] <= 'z')) ||
                ((source[1] >= '0') && (source[1] <= '9')))
            {
                // found a hotkey
                hotKeyPos = (unsigned int) (source - start);
                source++; // skip `&` character
                dest->Code = (char16) (*source);
                dest++;
                source++;
                hotKeyFound = true;
                continue;
            }
        }
        // no new_line --> direct copy
        dest->Code = (char16) (*source);
        dest++;
        source++;
    }
    return dest - ch_start;
}
template <typename T>
int FindInCharacterBuffer(const T& sv, const CharacterView& charView, bool ignoreCase)
{
    auto p                  = sv.data();
    auto p_end              = p + sv.length();
    const Character* ch     = charView.data();
    const Character* ch_end = ch + charView.length();
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

constexpr bool IgnoreCaseEquals(char16 code1, char16 code2)
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
    // LOG_INFO("Default ctor for %p", this);
    Allocated = 0;
    Count     = 0;
    Buffer    = nullptr;
}

CharacterBuffer::~CharacterBuffer(void)
{
    // LOG_INFO("DTOR for %p [Buffer=%p,Count=%d,Allocated=%d]", this, Buffer, (int) Count, (int) Allocated);
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
    // LOG_INFO("Destroy Character Buffer from (%p) with [Buffer = %p, Count = %d, Allocated = %d]",this, Buffer,
    // (int)Count, (int)Allocated);
    if (Buffer)
        delete[] Buffer;
    Buffer = nullptr;
    Count = Allocated = 0;
}
bool CharacterBuffer::Grow(size_t newSize)
{
    // make sure that the original size is a 32 bytes value (smaller than 0x00FFFFFF)
    CHECK(newSize <= MAX_ALLOCATION_SIZE, false, "Size must be smaller than 0x00FFFFFF");
    if (newSize <= (size_t) (Allocated & ALOCATION_MASK))
        return true;
    size_t alingSize = ((newSize | 15) + 1);
    CHECK(alingSize >= newSize, false, "Integer overflow (x86 case) for %z size!", newSize);
    CHECK(alingSize <= MAX_ALLOCATION_SIZE, false, "Size must be smaller than 0x00FFFFFF");
    Character* temp;
    try
    {
        temp = new Character[alingSize];
    }
    catch (...)
    {
        RETURNERROR(false, "Failed to allocate: %z characters", alingSize);
    }
    if (Buffer)
    {
        memcpy(temp, Buffer, sizeof(Character) * this->Count);
        delete[] Buffer;
    }
    Buffer    = temp;
    Allocated = (unsigned int) alingSize;
    return true;
}
bool CharacterBuffer::Resize(unsigned int size, char16 character, const ColorPair color)
{
    if (size == this->Count)
        return true; // nothing to do
    if (size < this->Count)
    {
        // truncate
        this->Count = size;
        return true;
    }
    CHECK(Grow(size), false, "Fail to allocate %d characters", size);
    auto s = this->Buffer + this->Count;
    auto e = this->Buffer + size;
    Character c;
    c.Code  = character;
    c.Color = color;
    while (s < e)
    {
        s->PackedValue = c.PackedValue;
        s++;
    }
    this->Count = size;
    return true;
}
bool CharacterBuffer::Fill(char16 character, unsigned int size, const ColorPair color)
{
    Character c;

    if (size != this->Count)
    {
        CHECK(Grow(size), false, "Fail to allocate %d characters", size);
    }
    this->Count = size;
    auto s      = this->Buffer;
    auto e      = s + this->Count;
    c.Code      = character;
    c.Color     = color;
    while (s < e)
    {
        s->PackedValue = c.PackedValue;
        s++;
    }
    return true;
}
bool CharacterBuffer::Set(const CharacterBuffer& obj)
{
    this->Count = 0; // we overwrite the buffer anyway - don't need to recopy it if the allocated space is too small
    CHECK(Grow(obj.Count), false, "Fail to allocate %d bytes ", (int) obj.Count);
    if (obj.Count > 0)
    {
        memcpy(this->Buffer, obj.Buffer, sizeof(Character) * obj.Count);
    }
    this->Count = obj.Count;
    return true;
}
bool CharacterBuffer::Add(const ConstString& text, const ColorPair color)
{
    ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;

    if (textObj.Length == 0)
        return true; // nothing to do
    CHECK(textObj.Data, false, "Expecting a valid (non-null) string");
    CHECK(Grow(this->Count + textObj.Length),
          false,
          "Fail to allocate space for the character buffer: %z",
          this->Count + textObj.Length);

    size_t sz;

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        sz = CopyStringToCharBuffer<char>(
              this->Buffer + this->Count, (const char*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::CharacterBuffer:
        sz = CopyStringToCharBuffer<Character>(
              this->Buffer + this->Count, (const Character*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::Unicode16:
        sz = CopyStringToCharBuffer<char16>(
              this->Buffer + this->Count, (const char16*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), false, "Fail to convert UTF-8 to current internal format !");
        sz = CopyStringToCharBuffer<char16>(this->Buffer + this->Count, ub.GetString(), ub.Len(), color);
        break;
    default:
        RETURNERROR(false, "Unknwon string encoding type: %d", textObj.Encoding);
    }
    CHECK(sz <= textObj.Length, false, "Internal error --> possible buffer overwrite !");
    // sz can be fitted in 32 bits
    this->Count += (unsigned int) sz;
    return true;
}
bool CharacterBuffer::Set(const ConstString& text, const ColorPair color)
{
    this->Count = 0;
    return Add(text, color);
}
bool CharacterBuffer::SetWithHotKey(
      const ConstString& text,
      unsigned int& hotKeyCharacterPosition,
      Input::Key& hotKey,
      Input::Key hotKeyModifier,
      const ColorPair color)
{
    ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;
    hotKeyCharacterPosition = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    hotKey                  = Input::Key::None;

    if (textObj.Length == 0)
        return true; // nothing to do
    CHECK(textObj.Data, false, "Expecting a valid (non-null) string");
    CHECK(Grow(textObj.Length), false, "Fail to allocate space for the character buffer: %z", textObj.Length);

    size_t sz;

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        sz = CopyStringToCharBufferWidthHotKey<char>(
              this->Buffer, (const char*) textObj.Data, textObj.Length, color, hotKeyCharacterPosition);
        break;
    case StringEncoding::CharacterBuffer:
        sz = CopyStringToCharBufferWidthHotKey<Character>(
              this->Buffer, (const Character*) textObj.Data, textObj.Length, color, hotKeyCharacterPosition);
        break;
    case StringEncoding::Unicode16:
        sz = CopyStringToCharBufferWidthHotKey<char16>(
              this->Buffer, (const char16*) textObj.Data, textObj.Length, color, hotKeyCharacterPosition);
        break;
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), false, "Fail to convert UTF-8 to current internal format !");
        sz = CopyStringToCharBufferWidthHotKey<char16>(
              this->Buffer, ub.GetString(), ub.Len(), color, hotKeyCharacterPosition);
        break;
    default:
        RETURNERROR(false, "Unknwon string encoding type: %d", textObj.Encoding);
    }
    if (sz > textObj.Length)
    {
        // reset values
        hotKeyCharacterPosition = CharacterBuffer::INVALID_HOTKEY_OFFSET;
        hotKey                  = Input::Key::None;
        RETURNERROR(false, "Internal error --> possible buffer overwrite !");
    }
    this->Count = (unsigned int) sz;
    // sanity check
    if ((hotKeyCharacterPosition != CharacterBuffer::INVALID_HOTKEY_OFFSET) && (hotKeyCharacterPosition >= this->Count))
    {
        // reset values
        hotKeyCharacterPosition = CharacterBuffer::INVALID_HOTKEY_OFFSET;
        hotKey                  = Input::Key::None;
    }
    if (hotKeyCharacterPosition != CharacterBuffer::INVALID_HOTKEY_OFFSET)
    {
        hotKey = Utils::KeyUtils::CreateHotKey(this->Buffer[hotKeyCharacterPosition].Code, hotKeyModifier);
        if (hotKey == Input::Key::None)
            hotKeyCharacterPosition = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    }

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
bool CharacterBuffer::Insert(const ConstString& text, unsigned int position, const ColorPair color)
{
    ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;

    if (textObj.Length == 0)
        return true; // nothing to do

    CHECK(textObj.Data, false, "Expecting a valid (non-null) string");
    CHECK(position <= this->Count,
          false,
          "Invalid insert offset: %d (should be between 0 and %d)",
          position,
          this->Count);
    CHECK(Grow(this->Count + textObj.Length),
          false,
          "Fail to allocate space for the character buffer: %z",
          this->Count + textObj.Length);

    // make space for new data
    if (position < this->Count)
    {
        memmove(
              this->Buffer + position + textObj.Length,
              this->Buffer + position,
              (this->Count - position) * sizeof(Character));
    }

    size_t writtenChars;

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        writtenChars =
              CopyStringToCharBuffer<char>(this->Buffer + position, (const char*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::CharacterBuffer:
        writtenChars = CopyStringToCharBuffer<Character>(
              this->Buffer + position, (const Character*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::Unicode16:
        writtenChars = CopyStringToCharBuffer<char16>(
              this->Buffer + position, (const char16*) textObj.Data, textObj.Length, color);
        break;
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), false, "Fail to convert UTF-8 to current internal format !");
        writtenChars = CopyStringToCharBuffer<char16>(this->Buffer + position, ub.GetString(), ub.Len(), color);
        break;
    default:
        RETURNERROR(false, "Unknwon string encoding type: %d", textObj.Encoding);
    }
    CHECK(writtenChars <= textObj.Length, false, "Internal error => possible buffer overwrite !");
    if (writtenChars < textObj.Length)
    {
        // fewer chars were actually writtem
        memmove(
              this->Buffer + position + writtenChars,
              this->Buffer + position + textObj.Length,
              (this->Count - position) * sizeof(Character));
    }
    this->Count += (unsigned int) writtenChars;
    return true;
}

bool CharacterBuffer::InsertChar(uint16 characterCode, unsigned int position, const ColorPair color)
{
    VALIDATE_ALLOCATED_SPACE(((size_t) this->Count) + 1, false);
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
bool CharacterBuffer::ConvertToUpper(unsigned int start, unsigned int end)
{
    if (end > this->Count)
        end = this->Count;
    CHECK(start <= end, false, "Expecting a valid parameter for start (%d) --> should be smaller than %d", start, end);
    Character* ch   = this->Buffer + start;
    unsigned int sz = end - start;
    while (sz)
    {
        // GDT: upper/lower case needs to be redesigned
        if ((ch->Code >= 'a') && (ch->Code <= 'z'))
            ch->Code -= 32;
        sz--;
        ch++;
    }
    return true;
}
bool CharacterBuffer::ConvertToLower(unsigned int start, unsigned int end)
{
    if (end > this->Count)
        end = this->Count;
    CHECK(start <= end, false, "Expecting a valid parameter for start (%d) --> should be smaller than %d", start, end);
    Character* ch   = this->Buffer + start;
    unsigned int sz = end - start;
    while (sz)
    {
        // GDT: upper/lower case needs to be redesigned
        if ((ch->Code >= 'A') && (ch->Code <= 'Z'))
            ch->Code += 32;
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
bool CharacterBuffer::CopyString(Utils::String& text, unsigned int start, unsigned int end)
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
bool CharacterBuffer::CopyString(Utils::String& text)
{
    return CopyString(text, 0, this->Count);
}
int CharacterBuffer::Find(const ConstString& text, bool ignoreCase) const
{
    ConstStringObject textObj(text);
    LocalUnicodeStringBuilder<1024> ub;

    CHECK(textObj.Data, -1, "Expecting a valid (non-null) string");
    if (textObj.Length == 0)
        return 0; // nothing to do

    switch (textObj.Encoding)
    {
    case StringEncoding::Ascii:
        return FindInCharacterBuffer<string_view>(std::get<string_view>(text), *this, ignoreCase);
    case StringEncoding::CharacterBuffer:
        return FindInCharacterBuffer<CharacterView>(std::get<CharacterView>(text), *this, ignoreCase);
    case StringEncoding::Unicode16:
        return FindInCharacterBuffer<u16string_view>(std::get<u16string_view>(text), *this, ignoreCase);
    case StringEncoding::UTF8:
        CHECK(ub.Set(text), -1, "Fail to convert UTF-8 to current internal format !");
        return FindInCharacterBuffer<u16string_view>(ub.ToStringView(), *this, ignoreCase);
    default:
        RETURNERROR(-1, "Unknwon string encoding type: %d", textObj.Encoding);
    }
}
int CharacterBuffer::CompareWith(const CharacterBuffer& obj, bool ignoreCase) const
{
    Character* s     = this->Buffer;
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
        while ((s < s_end) && (d < d_end))
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
        char16 s_c, d_c;
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

optional<unsigned int> CharacterBuffer::FindNext(
      unsigned int startOffset, bool (*shouldSkip)(unsigned int offset, Character ch)) const
{
    CHECK(this->Buffer, std::nullopt, "Object not initialized ");
    CHECK(shouldSkip, std::nullopt, "shouldSkip parameter must be valid (non-null)");
    if (startOffset >= this->Count)
        return this->Count;
    auto* p = this->Buffer + startOffset;
    while ((startOffset < this->Count) && (shouldSkip(startOffset, *p)))
    {
        p++;
        startOffset++;
    }
    return startOffset;
}
optional<unsigned int> CharacterBuffer::FindPrevious(
      unsigned int startOffset, bool (*shouldSkip)(unsigned int offset, Character ch)) const
{
    CHECK(this->Buffer, std::nullopt, "Object not initialized ");
    CHECK(shouldSkip, std::nullopt, "shouldSkip parameter must be valid (non-null)");
    if (this->Count == 0)
        return 0;
    if (startOffset >= this->Count)
        return this->Count - 1;
    auto* p = this->Buffer + startOffset;
    while ((startOffset > 0) && (shouldSkip(startOffset, *p)))
    {
        p--;
        startOffset--;
    }
    return startOffset;
}

bool CharacterBuffer::ToString(std::string& output) const
{
    CHECK(this->Buffer, false, "");
    const Character* p = this->Buffer;
    const Character* e = p + this->Count;
    output.reserve(this->Count);
    output = "";
    while (p < e)
    {
        if (p->Code < 256)
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
    char16 tmp[1] = { 0 };
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
} // namespace AppCUI