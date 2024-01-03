#include "AppCUI.hpp"

namespace AppCUI ::Utils
{
constexpr static string_view color_names[] = {
    "Black", "DarkBlue", "DarkGreen", "Teal", "DarkRed", "Magenta", "Olive", "Silver",      "Gray",
    "Blue",  "Green",    "Aqua",      "Red",  "Pink",    "Yellow",  "White", "Transparent",
};
constexpr uint32 HASH_DEVIDER     = 47;
uint8 color_indexes[HASH_DEVIDER] = { 0xFF, 0xFF, 0xFF, 2,    5,    16,   0xFF, 0xFF, 9,    0xFF, 11,   0xFF,
                                      0xFF, 0xFF, 8,    0xFF, 0xFF, 15,   10,   0xFF, 7,    0xFF, 0xFF, 0,
                                      1,    13,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 14,   12,   0xFF, 0xFF, 0xFF,
                                      0xFF, 3,    0xFF, 0xFF, 4,    6,    0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const uint8* SkipSpaces(const uint8* start, const uint8* end)
{
    while ((start < end) && (((*start) == ' ') || ((*start) == '\t') || ((*start) == '\n') || ((*start) == '\r')))
        start++;
    return start;
}
string_view ColorUtils::GetColorName(Graphics::Color color)
{
    if (((uint8) color) < (sizeof(color_names) / sizeof(string_view)))
        return color_names[(uint8) color];
    return "?";
}
std::optional<Graphics::Color> ColorUtils::GetColor(std::string_view name)
{
    auto p        = (const uint8*) name.data();
    auto e        = p + name.length();
    uint32 hash   = 0;
    uint32 number = 0;
    p             = SkipSpaces(p, e);
    if (p >= e)
        return std::nullopt;
    if (((*p) >= '0') && ((*p) <= '9'))
    {
        while (p < e)
        {
            if (((*p) >= '0') && ((*p) <= '9'))
            {
                number = number * 10 + ((*p) - '0');
                p++;
                continue;
            }
            break;
        }
        p = SkipSpaces(p, e);
        // if not at the end of the string
        if (p < e)
            return std::nullopt;
    }
    else
    {
        // Text name
        while (p < e)
        {
            if (((*p) >= 'a') && ((*p) <= 'z'))
            {
                hash = (hash << 1) ^ (static_cast<uint32>((*p) - 'a'));
                p++;
                continue;
            }
            if (((*p) >= 'A') && ((*p) <= 'Z'))
            {
                hash = (hash << 1) ^ (static_cast<uint32>((*p) - 'A'));
                p++;
                continue;
            }
            break;
        }
        p = SkipSpaces(p, e);
        // if not at the end of the string
        if (p < e)
            return std::nullopt;
        number = color_indexes[hash % HASH_DEVIDER];
    }
    // check the value
    if (number > 16)
        return std::nullopt;
    return static_cast<Graphics::Color>(number);
}
std::optional<Graphics::ColorPair> ColorUtils::GetColorPair(std::string_view name)
{
    auto s = (const char*) name.data();
    auto e = s + name.length();
    auto m = s;
    while ((m < e) && ((*m) != ':') && ((*m) != ','))
        m++;
    auto col_1 = ColorUtils::GetColor(string_view( s, (size_t)(m - s) ));
    if (!col_1.has_value())
        return std::nullopt;
    m++;
    if (m<e)
    {
        auto col_2 = ColorUtils::GetColor(string_view( m, (size_t)(e - m) ));
        if (!col_2.has_value())
            return std::nullopt;
        return Graphics::ColorPair{ col_1.value(), col_2.value() };
    }
    // if second parameter was not provided, consider the backgourn as transparent
    return Graphics::ColorPair{ col_1.value(), Graphics::Color::Transparent };
}
} // namespace AppCUI::Utils