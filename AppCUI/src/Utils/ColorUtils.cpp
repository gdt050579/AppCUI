#include "AppCUI.hpp"
#include <string.h>

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
            // invalid number
            return std::nullopt;
        }
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
            // invalid name
            return std::nullopt;
        }
        number = color_indexes[hash % HASH_DEVIDER];
    }
    // check the value
    if (number > 16)
        return std::nullopt;
    return static_cast<Graphics::Color>(number);
}
std::optional<Graphics::ColorPair> ColorUtils::GetColorPair(std::string_view name)
{
}
} // namespace AppCUI::Utils