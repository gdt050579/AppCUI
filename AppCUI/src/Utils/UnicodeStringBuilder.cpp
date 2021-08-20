#include <Internal.hpp>

using namespace AppCUI::Utils;
using namespace AppCUI::Graphics;

#define LOCAL_BUFFER_FLAG       0x80000000
#define MAX_ALLOCATION_SIZE     0x00FFFFFF

template <typename T>
void CopyText(char16_t * dest, const T* source, size_t len)
{
    while (len>0)
    {
        *dest = (char16_t) (*source);
        dest++;
        source++;
        len--;
    }
}

bool AppCUI::Utils::ConvertUTF8CharToUnicodeChar(const char8_t* p, const char8_t* end, UnicodeChar& result)
{
    // unicode encoding (based on the code described in https://en.wikipedia.org/wiki/UTF-8)
    if (((*p) >> 5) == 6) // binary encoding 110xxxxx, followed by 10xxxxxx
    {
        CHECK(p + 1 < end, false, "Invalid unicode sequence (missing one extra character after 110xxxx)");
        CHECK((p[1] >> 6) == 2, false, "Invalid unicode sequence (110xxxx should be followed by 10xxxxxx)");
        result.Value  = (((unsigned short) ((*p) & 0x1F)) << 6) | ((unsigned short) ((*(p + 1)) & 63));
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

void UnicodeStringBuilder::Create(char16_t* localBuffer, size_t localBufferSize)
{
    if ((localBuffer == nullptr) || (localBufferSize == 0) || (localBufferSize > MAX_ALLOCATION_SIZE))
    {
        this->Chars     = nullptr;
        this->Size      = 0;
        this->Allocated = 0;
    }
    else
    {
        this->Chars     = localBuffer;
        this->Size      = 0;
        this->Allocated = (unsigned int) (localBufferSize & 0x7FFFFFFF) | LOCAL_BUFFER_FLAG;
    }
}
void UnicodeStringBuilder::Destroy()
{
    if ((!(this->Allocated & LOCAL_BUFFER_FLAG)) && (this->Chars))
        delete[] this->Chars;
    this->Chars     = nullptr;
    this->Size      = 0;
    this->Allocated = 0;
}
bool UnicodeStringBuilder::Resize(size_t newSize)
{
    // make sure that the original size is a 32 bytes value (smaller than 0x00FFFFFF)
    CHECK(newSize <= MAX_ALLOCATION_SIZE, false, "Size must be smaller than 0x00FFFFFF");
    if (newSize <= (size_t)Allocated)
        return true;
    size_t alingSize = (newSize | 0xFF) + 1;
    CHECK(alingSize >= newSize, false, "Integer overflow (x86 case) for %z size!", newSize);
    // make sure that the aligned size (8-bytes aligned) is smaller that 0x00FFFFFF
    CHECK(alingSize <= MAX_ALLOCATION_SIZE, false, "Size must be smaller than 0x00FFFFFF");
    char16_t* newBuf;
    try
    {
        newBuf = new char16_t[alingSize];
    }
    catch (...)
    {
        RETURNERROR(false, "Fail to allocte %z elemens for Unicode buffer", alingSize);
    }
    if (this->Chars)
    {
        if (this->Size > 0)
        {
            memcpy(newBuf, this->Chars, sizeof(char16_t) * this->Size);
        }
        delete[] this->Chars;
    }
    newBuf = this->Chars;
    this->Allocated = (unsigned int) alingSize;
    return true;
}
UnicodeStringBuilder::UnicodeStringBuilder()
{
    Create(nullptr, 0);
}
UnicodeStringBuilder::UnicodeStringBuilder(char16_t* localBuffer, size_t localBufferSize)
{
    Create(localBuffer, localBufferSize);
}
UnicodeStringBuilder::UnicodeStringBuilder(const AppCUI::Utils::ConstString& text)
{
    Create(nullptr, 0);
    if (!Set(text))
        Destroy();
}
UnicodeStringBuilder::UnicodeStringBuilder(char16_t* localBuffer, size_t localBufferSize, const AppCUI::Utils::ConstString& text)
{
    Create(localBuffer, localBufferSize);
    if (!Set(text))
        Destroy();
}
UnicodeStringBuilder::UnicodeStringBuilder(const AppCUI::Graphics::CharacterBuffer& charBuffer)
{
    Create(nullptr, 0);
    if (!Set(charBuffer))
        Destroy();
}
UnicodeStringBuilder::UnicodeStringBuilder(char16_t* localBuffer, size_t localBufferSize, const AppCUI::Graphics::CharacterBuffer& charBuffer)
{
    Create(localBuffer, localBufferSize);
    if (!Set(charBuffer))
        Destroy();
}
UnicodeStringBuilder::~UnicodeStringBuilder()
{
    Destroy();
}
bool UnicodeStringBuilder::Add(const AppCUI::Utils::ConstString& text)
{
    ConstStringObject obj(text);
    CHECK(Resize(obj.Length+this->Size), false, "Fail to resize buffer !");
    // at this point we know that obj.Length is storable on an unsigned int value
    switch (obj.Encoding)
    {
    case StringEncoding::Ascii:
        CopyText<unsigned char>(this->Chars + this->Size, (const unsigned char*) obj.Data, obj.Length);
        this->Size += (unsigned int) obj.Length;
        return true;
    case StringEncoding::CharacterBuffer:
        CopyText<Character>(this->Chars + this->Size, (const Character*) obj.Data, obj.Length);
        this->Size += (unsigned int) obj.Length;
        return true;
    case StringEncoding::Unicode16:
        memcpy(this->Chars + this->Size, obj.Data, sizeof(char16_t) * obj.Length);
        this->Size += (unsigned int) obj.Length;
        return true;
    case StringEncoding::UTF8:
        const char8_t* start = (const char8_t*) obj.Data;
        const char8_t* end   = start + obj.Length;
        auto* p              = this->Chars + this->Size;

        UnicodeChar uc;
        while (start < end)
        {
            if ((*start) < 0x80)
            {
                *p = *start;
                start++;
            }
            else
            {
                CHECK(ConvertUTF8CharToUnicodeChar(start, end, uc), false, "Fail to convert unicode character !");
                *p = uc.Value;
                start += uc.Length;
            }
            p++;
        }
        this->Size += (unsigned int) (p - this->Chars);
        return true;
    }
    RETURNERROR(false, "Fail to Set a string (unknwon variant type)");
}
bool UnicodeStringBuilder::Set(const AppCUI::Utils::ConstString& text)
{
    this->Size = 0;
    return Add(text);
}
bool UnicodeStringBuilder::Set(const AppCUI::Graphics::CharacterBuffer& charBuffer)
{
    CHECK(Resize(charBuffer.Len()), false, "Fail to resize buffer !");
    CopyText<Character>(this->Chars, charBuffer.GetBuffer(), charBuffer.Len());
    this->Size = (unsigned int) charBuffer.Len();
    return true;
}
bool UnicodeStringBuilder::Add(const AppCUI::Graphics::CharacterBuffer& charBuffer)
{
    CHECK(Resize(charBuffer.Len()+this->Size), false, "Fail to resize buffer !");
    CopyText<Character>(this->Chars + this->Size, charBuffer.GetBuffer(), charBuffer.Len());
    this->Size += (unsigned int) charBuffer.Len();
    return true;
}
void UnicodeStringBuilder::ToString(std::string& output) const
{
    if (this->Chars == nullptr)
        output.clear();
    else
    {
        output.reserve((size_t)this->Size + 1);
        char temp[128];
        char16_t* p      = this->Chars;
        char16_t* e      = p + this->Size;
        unsigned char* r = (unsigned char *)&temp;
        size_t sz   = 0;
        while (p<e)
        {
            if (sz >= sizeof(temp))
            {
                output += std::string_view{ temp, sz };
                sz = 0;
                r  = (unsigned char*) &temp;
            }
            if (*p >= 0x80)
                *r = '?';
            else
                *r = (unsigned char)(*p);
            r++;
            sz++;
            p++;
        }
        if (sz >= sizeof(temp))
            output += std::string_view{ temp, sz };
    }
}
void UnicodeStringBuilder::ToString(std::u16string& output) const
{
    if (this->Chars == nullptr)
        output.clear();
    else
    {
        output.reserve((size_t)this->Size + 1);
        output = std::u16string_view{ this->Chars, this->Size };
    }
}
void UnicodeStringBuilder::ToPath(std::filesystem::path& output) const
{
    if (this->Chars == nullptr)
        output.clear();
    else
    {
        output = std::u16string_view{ this->Chars, this->Size };   
    }
}