#include "AppCUI.hpp"
#include <string.h>

namespace AppCUI ::Utils
{
constexpr static string_view color_names[] = {
    "Black", "DarkBlue", "DarkGreen", "Teal", "DarkRed", "Magenta", "Olive", "Silver",      "Gray",
    "Blue",  "Green",    "Aqua",      "Red",  "Pink",    "Yellow",  "White", "Transparent",
};
string_view ColorUtils::GetColorName(Graphics::Color color)
{
    if (((uint8) color) < (sizeof(color_names) / sizeof(string_view)))
        return color_names[(uint8) color];
    return "?";
}
} // namespace AppCUI::Utils