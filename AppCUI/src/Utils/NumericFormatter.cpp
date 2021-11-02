#include "AppCUI.hpp"

using namespace AppCUI::Utils;
static const char* BaseLettersUpperCase = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

std::string_view NumericFormatter::ToHexString(unsigned long long value)
{
    char* e = temp + sizeof(temp) - 1;
    char* s = e - 1;
    *e      = 0;
    e--;
    do
    {
        s--;
        *s = BaseLettersUpperCase[value & 0x0F];
        value >>= 4;
    } while (value > 0);
    return std::string_view{ s, (size_t) (e - s) };
}
std::string_view NumericFormatter::ToDecStringUnsigned(unsigned long long value)
{
    char* e = temp + sizeof(temp) - 1;
    char* s = e - 1;
    *e      = 0;
    e--;
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