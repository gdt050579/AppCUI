#include "AppCUI.hpp"

namespace AppCUI::Utils::Number
{
#define NUMBER_FLAG_NEGATIVE 0x00000001
#define NUMBER_FLAG_SECOND   0x00000002

uint8 __base_translation__[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255, 255, 255, 255, 255, 255, 10,
    11,  12,  13,  14,  15,  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
#define SKIP_SPACES                                                                                                    \
    if ((res.ParseFlags & NumberParseFlags::TrimSpaces) != NumberParseFlags::None)                                     \
    {                                                                                                                  \
        while ((start < end) && (((*start) == ' ') || ((*start) == '\t')))                                             \
        {                                                                                                              \
            start++;                                                                                                   \
        }                                                                                                              \
    }
#define VALIDATE_END_OF_STREAM CHECK(start < end, false, "Invalid string buffer (start should be smaller then end)");

#define PARSE_NUMBER                                                                                                   \
    _parse_number_result_ res;                                                                                         \
    CHECK(ParseNumber(res, text, flags, size), std::nullopt, "");

struct _parse_number_result_
{
    uint64 Value;
    long double SecondValue;
    unsigned int Flags;
    unsigned int Base;
    unsigned int* Size;
    NumberParseFlags ParseFlags;
};

bool _parse_number_string_buffer_(const uint8* start, const uint8* end, _parse_number_result_& res)
{
    const uint8* original_start = start;
    SKIP_SPACES;
    VALIDATE_END_OF_STREAM;

    // positive or negative
    res.Flags = 0;
    if ((*start) == '+')
    {
        start++;
    }
    else if ((*start) == '-')
    {
        res.Flags |= NUMBER_FLAG_NEGATIVE;
        start++;
    }
    SKIP_SPACES;
    VALIDATE_END_OF_STREAM;

    // compute the base
    NumberParseFlags base_Flags = res.ParseFlags & 0x0F;
    switch (base_Flags)
    {
    case NumberParseFlags::Base2:
        res.Base = 2;
        break;
    case NumberParseFlags::Base8:
        res.Base = 8;
        break;
    case NumberParseFlags::Base10:
        res.Base = 10;
        break;
    case NumberParseFlags::Base16:
        res.Base = 16;
        break;
    case NumberParseFlags::BaseAuto:
        res.Base = 10;
        if (((*start) == '0') && ((start + 1) < end))
        {
            char _base = ((*(start + 1)) | 0x20);
            if (_base == 'b')
                res.Base = 2;
            else if (_base == 'x')
                res.Base = 16;
            else
                res.Base = 8;
            start += 2;
            SKIP_SPACES;
            VALIDATE_END_OF_STREAM;
        }
        break;
    default:
        RETURNERROR(
              false, "Invalid combination of flags (numerical base could not been infered) => Flags: %d", base_Flags);
    }

    // compute
    uint8 charValue;
    uint64 newValue;
    res.Value = 0;
    while (start < end)
    {
        charValue = __base_translation__[*start];
        if (charValue >= res.Base)
            break;
        newValue = (res.Value * res.Base) + (uint64) charValue;
        CHECK(newValue > res.Value, false, "Integer overflow !");
        res.Value = newValue;
        start++;
    }
    if (start >= end)
        return true;
    // check next character
    if ((*start) == '.')
    {
        start++;
        res.SecondValue            = 0;
        uint64 devide  = 1;
        uint64 s_value = 0;
        res.Flags |= NUMBER_FLAG_SECOND;
        // consider only base 10
        while (start < end)
        {
            charValue = __base_translation__[*start];
            if (charValue >= 10)
                break;
            newValue = (s_value * 10) + (uint64) charValue;
            CHECK(newValue > s_value, false, "Integer overflow (2)!");
            s_value = newValue;
            devide *= 10;
            CHECK(devide < 100000000ULL, false, "");
            start++;
        }
        res.SecondValue = ((long double) s_value) / ((long double) devide);
        if (start >= end)
            return true;
    }
    SKIP_SPACES;
    if (res.Size)
    {
        *res.Size = (unsigned int) (start - original_start);
        return true;
    }
    return (start == end);
}

inline bool ParseNumber(_parse_number_result_& res, string_view text, NumberParseFlags flags, unsigned int* size)
{
    const uint8* start = reinterpret_cast<const uint8*>(text.data());
    CHECK(start, false, "Expecting a non-null string to convert to number !");
    const uint8* end = start + text.size();
    CHECK(start < end, false, "Expecting a non-empty string to convert to number !");

    res.ParseFlags = flags;
    res.Size       = size;
    CHECK(_parse_number_string_buffer_(start, end, res), false, "Fail to parse string for a number !");
    return true;
}

optional<uint64> ToUInt64(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_NEGATIVE | NUMBER_FLAG_SECOND)) == 0),
          std::nullopt,
          "Invalid format for an uint64 value");
    return res.Value;
}
optional<unsigned int> ToUInt32(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_NEGATIVE | NUMBER_FLAG_SECOND)) == 0),
          std::nullopt,
          "Invalid format for an unsigned int value");
    CHECK(res.Value <= 0xFFFFFFFFULL, std::nullopt, "Value can not be stored in an unsigned int variable");
    return (unsigned int) (res.Value);
}
optional<uint16> ToUInt16(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_NEGATIVE | NUMBER_FLAG_SECOND)) == 0),
          std::nullopt,
          "Invalid format for an uint16 value");
    CHECK(res.Value <= 0xFFFFULL, std::nullopt, "Value can not be stored in an uint16 variable");
    return (uint16) (res.Value);
}
optional<uint8> ToUInt8(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_NEGATIVE | NUMBER_FLAG_SECOND)) == 0),
          std::nullopt,
          "Invalid format for an uint8 value");
    CHECK(res.Value <= 0xFFULL, std::nullopt, "Value can not be stored in an uint8 variable");
    return (uint8) (res.Value);
}
optional<char> ToInt8(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_SECOND)) == 0), std::nullopt, "Invalid format for a char value");
    if (res.Flags & NUMBER_FLAG_NEGATIVE)
    {
        CHECK(res.Value <= ((1ULL << 7)), std::nullopt, "Value can not be stored in a char variable");
        return (char) (-((int64) (res.Value)));
    }
    else
    {
        CHECK(res.Value <= ((1ULL << 7) - 1), std::nullopt, "Value can not be stored in a char variable");
        return (char) (res.Value);
    }
}
optional<int16> ToInt16(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_SECOND)) == 0), std::nullopt, "Invalid format for a int16 value");
    if (res.Flags & NUMBER_FLAG_NEGATIVE)
    {
        CHECK(res.Value <= ((1ULL << 15)), std::nullopt, "Value can not be stored in a int16 variable");
        return (int16) (-((int64) (res.Value)));
    }
    else
    {
        CHECK(res.Value <= ((1ULL << 15) - 1), std::nullopt, "Value can not be stored in a int16 variable");
        return (int16) (res.Value);
    }
}
optional<int> ToInt32(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_SECOND)) == 0), std::nullopt, "Invalid format for an int value");
    if (res.Flags & NUMBER_FLAG_NEGATIVE)
    {
        CHECK(res.Value <= ((1ULL << 31)), std::nullopt, "Value can not be stored in an int variable");
        return (int) (-((int64) (res.Value)));
    }
    else
    {
        CHECK(res.Value <= ((1ULL << 31) - 1), std::nullopt, "Value can not be stored in an int variable");
        return (int) (res.Value);
    }
}
optional<int64> ToInt64(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    CHECK(((res.Flags & (NUMBER_FLAG_SECOND)) == 0), std::nullopt, "Invalid format for a int64 value");
    if (res.Flags & NUMBER_FLAG_NEGATIVE)
    {
        CHECK(res.Value <= ((1ULL << 63)), std::nullopt, "Value can not be stored in a int64 variable");
        return (-((int64) (res.Value)));
    }
    else
    {
        CHECK(res.Value <= ((uint64) ((1ULL << 63) - 1)),
              std::nullopt,
              "Value can not be stored in a int64 variable");
        return (int64) (res.Value);
    }
}
optional<float> ToFloat(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    float f = (float) res.Value;
    if (res.Flags & NUMBER_FLAG_SECOND)
        f += (float) res.SecondValue;
    if (res.Flags & NUMBER_FLAG_NEGATIVE)
        f = -f;
    return f;
}
optional<double> ToDouble(string_view text, NumberParseFlags flags, unsigned int* size)
{
    PARSE_NUMBER;
    double f = (double) res.Value;
    if (res.Flags & NUMBER_FLAG_SECOND)
        f += (double) res.SecondValue;
    if (res.Flags & NUMBER_FLAG_NEGATIVE)
        f = -f;
    return f;
}
} // namespace AppCUI::Utils::Number