#include "AppCUI.hpp"
#include <charconv>

using namespace AppCUI::Utils;
static const char* BaseLettersUpperCase = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

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
    CHECK((base >= 2) && (base < 35), nullptr, "Expecting a valid base (2..35) --> received: %d", base);
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
    CHECK((base >= 2) && (base < 35), nullptr, "Expecting a valid base (2..35) --> received: %d", base);
    switch (base)
    {
    case 2:
        return ToBinString(*(unsigned long long*) &value);
    case 8:
        return ToOctString(*(unsigned long long*) &value);
    case 10:
        return ToDecStringSigned(value);
    case 16:
        return ToHexString(*(unsigned long long *)&value);
    default:
        return ToGenericBase(*(unsigned long long*) &value, base);
    }
}