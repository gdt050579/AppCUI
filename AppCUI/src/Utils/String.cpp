#include <AppCUI.hpp>
#include "Internal.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

using namespace AppCUI;

const uint8 string_lowercase_table[256] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
    22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,
    44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  97,
    98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
    132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
    154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
    198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
    242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};
constexpr uint32 STRING_FLAG_STACK_BUFFER = 0x80000000;

constexpr uint32 STRING_FLAG_STATIC_BUFFER    = 1;
constexpr uint32 STRING_FLAG_CONSTANT         = 2;
constexpr uint32 STRING_FLAG_STATIC_WITH_GROW = 4;

#define COMPUTE_TEXT_SIZE(text, textSize)                                                                              \
    if (textSize == 0xFFFFFFFF)                                                                                        \
    {                                                                                                                  \
        textSize = Len(text);                                                                                          \
    }
#define PREPATE_STRING_SOURCE_DESTINATION_PARAMS                                                                       \
    CHECK(destination, false, "Expecting a valid (non-null) destination string");                                      \
    CHECK(source, false, "Expecting a valid (non-null) source parameter");                                             \
    CHECK(maxDestinationSize > 0,                                                                                      \
          false,                                                                                                       \
          "Expecting at least one character available in destination (maxDestinationSize should be bigger than 0)");   \
    COMPUTE_TEXT_SIZE(source, sourceSize);                                                                             \
    CHECK(sourceSize < maxDestinationSize,                                                                             \
          false,                                                                                                       \
          "Current destination size (%d) is smaller than the size of the text (%d)",                                   \
          maxDestinationSize,                                                                                          \
          sourceSize);
#define VALIDATE_STRINGS_TO_COMPARE                                                                                    \
    CHECK(sir1, false, "Expecting a valid (non-null) first parameter !");                                              \
    CHECK(sir2, false, "Expecting a valid (non-null) first parameter !");                                              \
    const uint8* p1 = (const uint8*) sir1;                                                                             \
    const uint8* p2 = (const uint8*) sir2;

#define VALIDATE_ALLOCATED_SPACE(requiredSpace, returnValue)                                                           \
    if ((requiredSpace) > (Allocated & 0x7FFFFFFF))                                                                    \
    {                                                                                                                  \
        CHECK(Grow(requiredSpace), returnValue, "Fail to allocate space for %d bytes", (requiredSpace));               \
    }

char tempCharForReferenceReturn;

namespace AppCUI::Utils
{
// Statical functions
uint32 String::Len(const char* string)
{
    if (string == nullptr)
        return 0;
    const char* p = string;
    while ((*string) != 0)
    {
        string++;
    }
    return (uint32) (string - p);
}
bool String::Add(
      char* destination,
      const char* source,
      uint32 maxDestinationSize,
      uint32 destinationSize,
      uint32 sourceSize,
      uint32* resultedDestinationSize)
{
    PREPATE_STRING_SOURCE_DESTINATION_PARAMS;
    COMPUTE_TEXT_SIZE(destination, destinationSize);
    CHECK(destinationSize + sourceSize < maxDestinationSize,
          false,
          "A total space of %d bytes is required to add string (available is %d)",
          destinationSize + sourceSize,
          maxDestinationSize);

    if (sourceSize > 0)
    {
        memcpy(destination + destinationSize, source, sourceSize);
    }
    destination[sourceSize + destinationSize] = 0;
    if (resultedDestinationSize)
        *resultedDestinationSize = sourceSize + destinationSize;
    return true;
}
bool String::Set(
      char* destination,
      const char* source,
      uint32 maxDestinationSize,
      uint32 sourceSize,
      uint32* resultedDestinationSize)
{
    PREPATE_STRING_SOURCE_DESTINATION_PARAMS;

    if (sourceSize > 0)
    {
        memcpy(destination, source, sourceSize);
    }
    destination[sourceSize] = 0;
    if (resultedDestinationSize)
        *resultedDestinationSize = sourceSize;
    return true;
}
bool String::Equals(const char* sir1, const char* sir2, bool ignoreCase)
{
    VALIDATE_STRINGS_TO_COMPARE;
    if (ignoreCase)
    {
        while ((*p1) && (*p2) && ((string_lowercase_table[*p1]) == (string_lowercase_table[*p2])))
        {
            p1++;
            p2++;
        }
        return ((*p2) == 0) && ((*p1) == 0);
    }
    else
    {
        while ((*p1) && (*p2) && ((*p1) == (*p2)))
        {
            p1++;
            p2++;
        }
        return ((*p2) == 0) && ((*p1) == 0);
    }
}
bool String::StartsWith(const char* sir1, const char* sir2, bool ignoreCase)
{
    VALIDATE_STRINGS_TO_COMPARE;

    if (ignoreCase)
    {
        while ((*p1) && (*p2) && ((string_lowercase_table[*p1]) == (string_lowercase_table[*p2])))
        {
            p1++;
            p2++;
        }
        return (*p2) == 0;
    }
    else
    {
        while ((*p1) && (*p2) && ((*p1) == (*p2)))
        {
            p1++;
            p2++;
        }
        return (*p2) == 0;
    }
}
bool String::StartsWith(string_view sir1, string_view sir2, bool ignoreCase)
{
    auto p1   = (const uint8*) sir1.data();
    auto p2   = (const uint8*) sir2.data();
    auto p2_e = p2 + sir2.length();
    CHECK(p1, false, "");
    CHECK(p2, false, "");
    CHECK(sir1.length() >= sir2.length(), false, "");
    if (ignoreCase)
    {
        while (p2 < p2_e)
        {
            if (string_lowercase_table[*p2] != string_lowercase_table[*p1])
                return false;
            p2++;
            p1++;
        }
        return true;
    }
    else
    {
        while (p2 < p2_e)
        {
            if ((*p2) != (*p1))
                return false;
            p2++;
            p1++;
        }
        return true;
    }
}
bool String::EndsWith(const char* sir1, const char* sir2, bool ignoreCase, uint32 sir1Size, uint32 sir2Size)
{
    VALIDATE_STRINGS_TO_COMPARE
    COMPUTE_TEXT_SIZE(sir1, sir1Size);
    COMPUTE_TEXT_SIZE(sir2, sir2Size);

    if (sir2Size > sir1Size)
        return false;
    p1 += (sir2Size - sir1Size);
    if (ignoreCase)
    {
        while ((*p1) && (*p2) && ((string_lowercase_table[*p1]) == (string_lowercase_table[*p2])))
        {
            p1++;
            p2++;
        }
        return ((*p2) == 0) && ((*p1) == 0);
    }
    else
    {
        while ((*p1) && (*p2) && ((*p1) == (*p2)))
        {
            p1++;
            p2++;
        }
        return ((*p2) == 0) && ((*p1) == 0);
    }
}
bool String::Contains(const char* sir, const char* textToFind, bool ignoreCase)
{
    CHECK((sir != nullptr) && (textToFind != nullptr),
          false,
          "Invalid parameters (both 'sir' and 'textToFind' must not be null)");
    if ((*textToFind) == 0)
        return true; // empty string exists in every strings
    const uint8* p_sir  = (const uint8*) sir;
    const uint8* p_find = (const uint8*) textToFind;
    const uint8* ps;
    const uint8* pf;
    uint8 char_to_find = *(p_find);

    if (ignoreCase)
    {
        char_to_find = string_lowercase_table[*p_find];
        while (*p_sir)
        {
            if (string_lowercase_table[*p_sir] == char_to_find)
            {
                ps = p_sir;
                pf = p_find;
                for (; (*pf) && (*ps) && (string_lowercase_table[*ps] == string_lowercase_table[*pf]); pf++, ps++)
                    ;
                if (!(*pf))
                    return true;
            }
            p_sir++;
        }
    }
    else
    {
        while (*p_sir)
        {
            if ((*p_sir) == char_to_find)
            {
                ps = p_sir;
                pf = p_find;
                for (; (*pf) && (*ps) && ((*ps) == (*pf)); pf++, ps++)
                    ;
                if (!(*pf))
                    return true;
            }
            p_sir++;
        }
    }
    return false;
}
int32 String::Compare(const char* sir1, const char* sir2, bool ignoreCase)
{
    VALIDATE_STRINGS_TO_COMPARE;
    if (ignoreCase)
    {
        while ((*p1) && (*p2) && ((string_lowercase_table[*p1]) == (string_lowercase_table[*p2])))
        {
            p1++;
            p2++;
        }
        if (string_lowercase_table[*p1] < string_lowercase_table[*p2])
            return -1;
        if (string_lowercase_table[*p1] > string_lowercase_table[*p2])
            return 1;
        return 0;
    }
    else
    {
        while ((*p1) && (*p2) && ((*p1) == (*p2)))
        {
            p1++;
            p2++;
        }
        if ((*p1) < (*p2))
            return -1;
        if ((*p1) > (*p2))
            return 1;
        return 0;
    }
}

//--------------------------------------------------- CONSTRUCTORI OBIECT
//----------------------------------------------------------------
String::String(void)
{
    Text = nullptr;
    Size = Allocated = 0;
}

String::String(const String& s)
{
    Text = nullptr;
    Size = Allocated = 0;
    if (Create(s.Size + 32))
    {
        if (s.Text)
        {
            memcpy(this->Text, s.Text, s.Size + 1);
            this->Size = s.Size;
        }
    }
}
String::~String(void)
{
    Destroy();
}
void String::Destroy()
{
    if ((Text != nullptr) && ((Allocated & STRING_FLAG_STACK_BUFFER) == 0))
    {
        delete[] Text;
    }
    Text = nullptr;
    Size = Allocated = 0;
}

bool String::Create(uint32 initialAllocatedBufferSize)
{
    CHECK(initialAllocatedBufferSize == 0, false, "initialAllocatedBufferSize must be bigger than 0 !");
    initialAllocatedBufferSize = ((initialAllocatedBufferSize | 15) + 1) & 0x7FFFFFFF;
    if (initialAllocatedBufferSize <= (Allocated & 0x7FFFFFFF))
    {
        *Text = 0;
        Size  = 0;
        return true;
    }
    Destroy();
    char* temp = new char[initialAllocatedBufferSize];
    CHECK(temp, false, "Failed to allocate %d bytes", initialAllocatedBufferSize);
    Text      = temp;
    Size      = 0;
    *Text     = 0;
    Allocated = initialAllocatedBufferSize;
    return true;
}
bool String::Create(const char* text)
{
    CHECK(text, false, "Expecting a non-null string !");
    uint32 len = String::Len(text);
    CHECK(Create(len + 1), false, "Fail to create string buffer with len: %d", len);
    memcpy(this->Text, text, len + 1);
    Size            = len + 1;
    this->Text[len] = 0;
    return true;
}
bool String::Create(char* buffer, uint32 bufferSize, bool emptyString)
{
    CHECK(buffer, false, "Expecting a valid (non-null) buffer");
    CHECK(bufferSize >= 1, false, "bufferSize must be bigger than 1");
    CHECK(bufferSize < 0x7FFFFFFF, false, "bufferSize must be smaller than 0x7FFFFFFF");
    Destroy();
    Text      = buffer;
    Allocated = (bufferSize & 0x7FFFFFFF) | STRING_FLAG_STACK_BUFFER;
    if (emptyString)
    {
        Size       = 0;
        Text[Size] = 0;
    }
    else
    {
        const char* e = buffer + bufferSize - 1;
        Size          = 0;
        while ((buffer < e) && (*buffer))
        {
            buffer++;
            Size++;
        }
        *buffer = 0;
    }
    return true;
}

void String::Clear()
{
    if (Text)
    {
        *Text = 0;
        Size  = 0;
    }
}

bool String::Realloc(uint32 newSize)
{
    if (newSize <= (Allocated & 0x7FFFFFFF))
        return true;
    return Grow(newSize);
}
bool String::Grow(uint32 newSize)
{
    newSize = ((newSize | 15) + 1) & 0x7FFFFFFF;
    if (newSize <= (Allocated & 0x7FFFFFFF))
        return true;
    char* temp = new char[newSize];
    CHECK(temp, false, "Failed to allocate: %d bytes", newSize);
    if (Text)
    {
        memcpy(temp, Text, Size + 1);
        if ((Allocated & STRING_FLAG_STACK_BUFFER) == 0)
            delete []Text;
    }
    Text      = temp;
    Allocated = newSize;
    return true;
}

// ==============================================================[ADD FUNCTIONS]=====================
bool String::Add(const char* text, uint32 txSize)
{
    CHECK(text, false, "Expecting a non-null parameter !");
    COMPUTE_TEXT_SIZE(text, txSize);
    VALIDATE_ALLOCATED_SPACE(this->Size + txSize + 1, false);
    memcpy(this->Text + this->Size, text, txSize);
    this->Size += txSize;
    this->Text[this->Size] = 0;
    return true;
}
bool String::Add(const String& text)
{
    return this->Add(text.Text, text.Size);
}
bool String::Add(string_view text)
{
    CHECK(text.length() <= 0x7FFFFFFF, false, "");
    return this->Add(text.data(), (uint32) text.length());
}
bool String::Add(const String* text)
{
    CHECK(text, false, "Expecting a non-null first parameter !");
    return this->Add(text->Text, text->Size);
}
bool String::AddChar(char ch)
{
    CHECK(ch, false, "NULL character can not be added !");
    char temp[2];
    temp[0] = ch;
    temp[1] = 0;
    return this->Add(temp, 1);
}
bool String::AddChars(char ch, uint32 count)
{
    CHECK(ch, false, "NULL character can not be added !");
    CHECK(count, false, "'count' should be bigger than 0");
    VALIDATE_ALLOCATED_SPACE(this->Size + count + 1, false);
    char* p = this->Text + this->Size;
    this->Size += count;
    while (count)
    {
        *p = ch;
        p++;
        count--;
    }
    *p = 0;
    return true;
}

// ============================================================================================[SET
// FUNCTIONS]=====================
bool String::Set(const char* text, uint32 txSize)
{
    CHECK(text, false, "Expecting a non-null parameter !");
    COMPUTE_TEXT_SIZE(text, txSize);
    VALIDATE_ALLOCATED_SPACE(txSize + 1, false);
    memcpy(this->Text, text, txSize);
    this->Size             = txSize;
    this->Text[this->Size] = 0;
    return true;
}
bool String::Set(const String& text)
{
    return this->Set(text.Text, text.Size);
}
bool String::Set(string_view text)
{
    CHECK(text.length() <= 0x7FFFFFFF, false, "");
    return this->Set(text.data(), (uint32) text.length());
}
bool String::Set(const String* text)
{
    CHECK(text, false, "Expecting a non-null first parameter !");
    return this->Set(text->Text, text->Size);
}
bool String::Set(u16string_view unicodeText)
{
    CHECK(unicodeText.size() < 0xFFFFFE, false, "Unicode text is too large");
    VALIDATE_ALLOCATED_SPACE((uint32) unicodeText.size() + 1, false);

    auto* p = unicodeText.data();
    auto* e = p + unicodeText.size();
    auto* t = this->Text;
    while (p < e)
    {
        if (((*p) > 0) && ((*p) <= 127))
            *t = (char) (*p);
        else
        {
            RETURNERROR(false, "Invalid (non-ascii) character [Code: %u]", (uint32) *p);
        }
        t++;
        p++;
    }
    *t         = 0;
    this->Size = (uint32) unicodeText.size();
    return true;
}
bool String::Set(CharacterView unicodeText)
{
    CHECK(unicodeText.size() < 0xFFFFFE, false, "Unicode text is too large");
    VALIDATE_ALLOCATED_SPACE((uint32) unicodeText.size() + 1, false);

    auto* p = unicodeText.data();
    auto* e = p + unicodeText.size();
    auto* t = this->Text;
    while (p < e)
    {
        if ((p->Code > 0) && (p->Code <= 127))
            *t = (char) p->Code;
        else
        {
            RETURNERROR(false, "Invalid (non-ascii) character [Code: %u]", (uint32) p->Code);
        }
        t++;
        p++;
    }
    *t         = 0;
    this->Size = (uint32) unicodeText.size();
    return true;
}
bool String::SetChars(char ch, uint32 count)
{
    CHECK(ch, false, "NULL character can not be added !");
    CHECK(count, false, "'count' should be bigger than 0");
    VALIDATE_ALLOCATED_SPACE(count + 1, false);
    char* p = this->Text;
    this->Size += count;
    while (count)
    {
        *p = ch;
        p++;
        count--;
    }
    *p = 0;
    return true;
}

bool String::InsertChar(char character, uint32 position)
{
    CHECK(character, false, "NULL character can not be added !");
    VALIDATE_ALLOCATED_SPACE(this->Size + 2, false);
    CHECK(position <= this->Size,
          false,
          "Invalid insert offset: %d (should be between 0 and %d)",
          position,
          this->Size);
    if (position == this->Size)
    {
        this->Text[this->Size++] = character;
        this->Text[this->Size]   = 0;
    }
    else
    {
        memmove(this->Text + position + 1, this->Text + position, this->Size - position);
        this->Text[position] = character;
        this->Size++;
        this->Text[this->Size] = 0;
    }
    return true;
}
bool String::DeleteChar(uint32 position)
{
    CHECK(position < this->Size,
          false,
          "Invalid delete offset: %d (should be between 0 and %d)",
          position,
          this->Size - 1);
    if ((position + 1) == this->Size)
    {
        this->Size--;
        this->Text[this->Size] = 0;
    }
    else
    {
        memmove(this->Text + position, this->Text + position + 1, this->Size - (position + 1));
        this->Size--;
        this->Text[this->Size] = 0;
    }
    return true;
}
bool String::Delete(uint32 start, uint32 end)
{
    CHECK(end <= this->Size, false, "Invalid delete offset: %d (should be between 0 and %d)", end, this->Size);
    CHECK(start < end, false, "Start parameter (%d) should be smaller than End parameter (%d)", start, end);
    if (end == this->Size)
    {
        this->Size             = start;
        this->Text[this->Size] = 0;
    }
    else
    {
        memmove(this->Text + start, this->Text + end, this->Size - end);
        this->Size -= (end - start);
        this->Text[this->Size] = 0;
    }
    return true;
}
char String::GetChar(int32 index) const
{
    if (Text == nullptr)
        return 0;
    if ((index >= 0) && (index < (int32) Size))
        return Text[(uint32) index];
    uint32 idx = (uint32) (-index);
    if (idx < Size)
        return Text[Size - idx];
    return 0;
}
bool String::SetChar(int32 index, char value)
{
    CHECK(Text, false, "Text buffer was not allocated !");
    if ((index >= 0) && (index < (int32) Size))
    {
        Text[index] = value;
        return true;
    }
    uint32 idx = (uint32) (-index);
    if (idx < Size)
    {
        Text[Size + idx] = value;
        return true;
    }
    RETURNERROR(false, "Invalid text index: %d for a text with length %d", index, this->Size);
}

bool String::SetFormat(const char* format, ...)
{
    va_list args;
    int32 len, len2;

    CHECK(format, false, "Expecting a valid(non-null) format parameter !");
    va_start(args, format);
    len = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    CHECK(len >= 0, false, "Invalid format (unable to format your string) !");
    VALIDATE_ALLOCATED_SPACE(((uint32) len) + 2, false);

    va_start(args, format);
    len2 = vsnprintf(Text, (Allocated & 0x7FFFFFFF) - 1, format, args);
    va_end(args);
    if (len2 < 0)
    {
        Clear();
        RETURNERROR(false, "Fail on vsnprintf !");
    }
    this->Size       = (uint32) len2;
    Text[this->Size] = 0;

    return true;
}
bool String::AddFormat(const char* format, ...)
{
    va_list args;
    int32 len, len2;

    CHECK(format, false, "Expecting a valid(non-null) format parameter !");
    va_start(args, format);
    len = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    CHECK(len >= 0, false, "Invalid format (unable to format your string) !");
    VALIDATE_ALLOCATED_SPACE(((uint32) len) + 2 + this->Size, false);

    va_start(args, format);
    len2 = vsnprintf(Text + this->Size, (Allocated & 0x7FFFFFFF) - 1, format, args);
    va_end(args);
    if (len2 < 0)
    {
        Clear();
        RETURNERROR(false, "Fail on vsnprintf !");
    }
    this->Size += (uint32) len2;
    Text[this->Size] = 0;

    return true;
}
string_view String::Format(const char* format, ...)
{
    va_list args;
    int32 len, len2;

    CHECK(format, "", "Expecting a valid(non-null) format parameter !");
    va_start(args, format);
    len = vsnprintf(nullptr, 0, format, args);
    va_end(args);
    CHECK(len >= 0, "", "Invalid format (unable to format your string) !");

    VALIDATE_ALLOCATED_SPACE(((uint32) len) + 2, "");

    va_start(args, format);
    len2 = vsnprintf(Text, (Allocated & 0x7FFFFFFF) - 1, format, args);
    va_end(args);
    if (len2 < 0)
    {
        Clear();
        RETURNERROR("", "Fail on vsnprintf !");
    }
    this->Size       = ((uint32) len2);
    Text[this->Size] = 0;

    return string_view{ Text, Size };
}

bool String::Truncate(uint32 newText)
{
    if (newText <= Size)
    {
        Size       = newText;
        Text[Size] = 0;
        return true;
    }
    return false;
}

bool String::StartsWith(const char* text, bool ignoreCase) const
{
    return String::StartsWith(Text, text, ignoreCase);
}
bool String::StartsWith(const String* text, bool ignoreCase) const
{
    CHECK(text != nullptr, false, "Expecting a non null parameter");
    return String::StartsWith(Text, text->Text, ignoreCase);
}
bool String::StartsWith(const String& text, bool ignoreCase) const
{
    return String::StartsWith(Text, text.Text, ignoreCase);
}
bool String::Contains(const char* text, bool ignoreCase) const
{
    return String::Contains(this->Text, text, ignoreCase);
}
bool String::Contains(const String& ss, bool ignoreCase) const
{
    return String::Contains(this->Text, ss.Text, ignoreCase);
}
bool String::EndsWith(const char* ss, bool ignoreCase) const
{
    return String::EndsWith(Text, ss, ignoreCase, Size);
}
bool String::EndsWith(const String* text, bool ignoreCase) const
{
    CHECK(text != nullptr, false, "Expecting a non null parameter");
    return String::EndsWith(Text, text->Text, ignoreCase, Size, text->Size);
}
bool String::EndsWith(const String& text, bool ignoreCase) const
{
    return String::EndsWith(Text, text.Text, ignoreCase, Size, text.Size);
}
bool String::Equals(const char* text, bool ignoreCase) const
{
    return String::Equals(this->Text, text, ignoreCase);
}
bool String::Equals(const String& text, bool ignoreCase) const
{
    if (this->Size != text.Size)
        return false;
    return String::Equals(this->Text, text.Text, ignoreCase);
}
int32 String::CompareWith(const char* text, bool ignoreCase) const
{
    return String::Compare(this->Text, text, ignoreCase);
}

char& String::operator[](int32 poz)
{
    if ((Text == nullptr) || (Size == 0))
    {
        tempCharForReferenceReturn = 0;
        return tempCharForReferenceReturn;
    }
    if (poz < 0)
        poz += (int32) this->Size;
    if (((poz + 1) > (int32) Size) || (poz < 0))
    {
        tempCharForReferenceReturn = 0;
        return tempCharForReferenceReturn;
    }
    return Text[poz];
}
void String::ConvertToInternalNewLineFormat()
{
    if ((this->Text == nullptr) || (this->Size == 0))
        return; // nothing to convert
    char* s = Text;
    char* e = Text + this->Size;
    char* o = Text;
    while (s < e)
    {
        if ((*s) == '\r')
        {
            *o = NEW_LINE_CODE;
            s++;
            o++;
            if ((s < e) && ((*s) == '\n'))
                s++;
            continue;
        }
        if ((*s) == '\n')
        {
            *o = NEW_LINE_CODE;
            s++;
            o++;
            if ((s < e) && ((*s) == '\r'))
                s++;
            continue;
        }
        *o = *s;
        o++;
        s++;
    }
    this->Size -= (uint32) (s - o);
    this->Text[this->Size] = 0;
}
} // namespace AppCUI::Utils