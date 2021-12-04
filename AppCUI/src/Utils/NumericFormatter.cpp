#include "AppCUI.hpp"
#include <charconv>

namespace AppCUI
{
using namespace Utils;
constexpr unsigned int NUMERIC_FORMAT_HEAP_SIZE   = 1024;
static const char* BaseLettersUpperCase           = "0123456789ABCDEF";
static const char* BaseLettersLowerCase           = "0123456789abcdef";
static const unsigned char NumberMaxSizeForBase[] = {
    0 /*0*/,  0 /*1*/,   66 /*2*/,  43 /*3*/,  34 /*4*/,  30 /*5*/,  27 /*6*/,  25 /*7*/, 24 /*8*/,
    23 /*9*/, 22 /*10*/, 21 /*11*/, 20 /*12*/, 20 /*13*/, 19 /*14*/, 19 /*15*/, 18 /*16*/
};

std::string_view NumericFormatter::ToGenericBase(unsigned long long value, unsigned long long base)
{
    // it is assume that `base` is between 2 and 35 (checked before calling this private method)
    char* e = temp + sizeof(temp) - 1;
    char* s = e;
    *e      = 0;
    do
    {
        s--;
        *s = BaseLettersUpperCase[value % base];
        value /= base;
    } while (value > 0);
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToHexString(unsigned long long value)
{
    char* e = temp + sizeof(temp) - 1;
    char* s = e;
    *e      = 0;
    do
    {
        s--;
        *s = BaseLettersUpperCase[value & 0x0F];
        value >>= 4;
    } while (value > 0);
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToOctString(unsigned long long value)
{
    char* e = temp + sizeof(temp) - 1;
    char* s = e;
    *e      = 0;
    do
    {
        s--;
        *s = BaseLettersUpperCase[value & 0x07];
        value >>= 3;
    } while (value > 0);
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToBinString(unsigned long long value)
{
    char* e = temp + sizeof(temp) - 1;
    char* s = e;
    *e      = 0;
    do
    {
        s--;
        if (value & 1)
            *s = '1';
        else
            *s = '0';
        value >>= 1;
    } while (value > 0);
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToDecStringUnsigned(unsigned long long value)
{
    char* e = temp + sizeof(temp) - 1;
    char* s = e;
    *e      = 0;
    do
    {
        s--;
        *s = BaseLettersUpperCase[value % 10];
        value /= 10;
    } while (value > 0);
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToDecStringSigned(long long value)
{
    bool negative = value < 0;
    char* e       = temp + sizeof(temp) - 1;
    char* s       = e;
    *e            = 0;
    if (negative)
    {
        do
        {
            s--;
            *s = BaseLettersUpperCase[-(value % 10)];
            value /= 10;
        } while (value != 0);
        s--;
        *s = '-';
    }
    else
    {
        do
        {
            s--;
            *s = BaseLettersUpperCase[value % 10];
            value /= 10;
        } while (value != 0);
    }
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToBaseUnsigned(unsigned long long value, int base)
{
    CHECK((base >= 2) && (base <= 16), nullptr, "Expecting a valid base (2..16) --> received: %d", base);
    switch (base)
    {
    case 2:
        return ToBinString(value);
    case 8:
        return ToOctString(value);
    case 10:
        return ToDecStringUnsigned(value);
    case 16:
        return ToHexString(value);
    default:
        return ToGenericBase(value, base);
    }
}
std::string_view NumericFormatter::ToBaseSigned(long long value, int base)
{
    CHECK((base >= 2) && (base <= 16), nullptr, "Expecting a valid base (2..16) --> received: %d", base);
    switch (base)
    {
    case 2:
        return ToBinString(*(unsigned long long*) &value);
    case 8:
        return ToOctString(*(unsigned long long*) &value);
    case 10:
        return ToDecStringSigned(value);
    case 16:
        return ToHexString(*(unsigned long long*) &value);
    default:
        return ToGenericBase(*(unsigned long long*) &value, base);
    }
}

std::string_view NumericFormatter::ToStringUnsigned(unsigned long long value, NumericFormat fmt)
{
    CHECK((fmt.Base >= 2) && (fmt.Base <= 16), nullptr, "Expecting a valid base (2..16) --> received: %d", fmt.Base);
    char* s;
    char* e;
    const char* base_letter;
    if (this->heapBuffer)
    {
        s = e = this->heapBuffer + NUMERIC_FORMAT_HEAP_SIZE - 1;
    }
    else
    {
        unsigned char sz = NumberMaxSizeForBase[fmt.Base];
        if (fmt.Base == 2)
        {
            if (value <= 0xFFULL)
                sz = 8;
            else if (value <= 0xFFFFULL)
                sz = 16;
            else if (value < 0xFFFFFFFFULL)
                sz = 32;
        }
        // need to check if stack size is large enough !
        sz = std::max<>(sz, fmt.DigitsCount);
        if (fmt.GroupSize > 0)
            sz += (sz / fmt.GroupSize) + 1;
        sz += 3; // if prefix/suffixes are available
        if (sz >= sizeof(this->temp))
        {
            this->heapBuffer = new char[NUMERIC_FORMAT_HEAP_SIZE];
            s = e = this->heapBuffer + NUMERIC_FORMAT_HEAP_SIZE - 1;
        }
        else
        {
            s = e = this->temp + sizeof(this->temp) - 1;
        }
    }
    *e = 0; // last char is always null;
    if ((fmt.Base == 16) && ((fmt.Flags & NumericFormatFlags::HexSuffix) != NumericFormatFlags::None))
    {
        s--;
        *s = 'h';
    }
    if ((fmt.Flags & NumericFormatFlags::LowerCase) != NumericFormatFlags::None)
        base_letter = BaseLettersLowerCase;
    else
        base_letter = BaseLettersUpperCase;
    // build the string
    unsigned int count  = 0;
    unsigned int g_size = fmt.GroupSize;
    if (fmt.GroupSize == 0)
    {
        do
        {
            s--;
            *s = base_letter[value % fmt.Base];
            value /= fmt.Base;
            count++;
        } while (value > 0);
    }
    else
    {
        do
        {
            s--;
            *s = base_letter[value % fmt.Base];
            value /= fmt.Base;
            count++;
            g_size--;
            if ((g_size == 0) && (value > 0))
            {
                s--;
                *s     = fmt.GroupSeparator;
                g_size = fmt.GroupSize;
            }
        } while (value > 0);
    }
    // add extra Digits
    if (fmt.GroupSize == 0)
    {
        while (count < fmt.DigitsCount)
        {
            s--;
            *s = '0';
            count++;
        }
    }
    else
    {
        while (count < fmt.DigitsCount)
        {
            if (g_size == 0)
            {
                s--;
                *s     = fmt.GroupSeparator;
                g_size = fmt.GroupSize;
            }
            s--;
            *s = '0';
            count++;
            g_size--;
        }
    }
    // add prefixes
    switch (fmt.Base)
    {
    case 2:
        if ((fmt.Flags & NumericFormatFlags::BinPrefix) != NumericFormatFlags::None)
        {
            s--;
            *s = 'b';
            s--;
            *s = '0';
        }
        break;
    case 8:
        if (((fmt.Flags & NumericFormatFlags::OctPrefix) != NumericFormatFlags::None) && ((*s) != '0'))
        {
            s--;
            *s = '0';
        }
        break;
    case 10:
        if (((fmt.Flags & NumericFormatFlags::MinusSign) != NumericFormatFlags::None))
        {
            s--;
            *s = '-';
        }
        else
        {
            if (((fmt.Flags & NumericFormatFlags::PlusSign) != NumericFormatFlags::None))
            {
                s--;
                *s = '+';
            }
        }
        break;
    case 16:
        if ((fmt.Flags & NumericFormatFlags::HexPrefix) != NumericFormatFlags::None)
        {
            s--;
            *s = 'x';
            s--;
            *s = '0';
        }
        break;

    default:
        break;
    }
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToStringSigned(long long value, NumericFormat fmt)
{
    if ((fmt.Base == 10) && (value < 0))
    {
        // need to convert to unsigned (there is a known bug if value is the minimum possible value of long long)
        fmt.Flags |= NumericFormatFlags::MinusSign;
        return ToStringUnsigned((unsigned long long) (-value), fmt);
    }
    else
    {
        return ToStringUnsigned(*(unsigned long long*) &value, fmt);
    }
}
std::string_view NumericFormatter::ToDec(float value)
{
    String tmp;
    tmp.Create(this->temp, sizeof(this->temp), true);
    return tmp.Format("%.3f", value);
}
std::string_view NumericFormatter::ToDec(double value)
{
    String tmp;
    tmp.Create(this->temp, sizeof(this->temp), true);
    return tmp.Format("%.3lf", value);
}
} // namespace AppCUI