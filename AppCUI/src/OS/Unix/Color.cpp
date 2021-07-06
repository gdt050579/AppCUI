#include <ncurses.h>
#include <map>

#include "Color.h"
#include "Internal.h"

using namespace AppCUI::Terminal;
using namespace AppCUI::Console;

constexpr size_t EXTENDED_COLORSET_SIZE = 16;
constexpr size_t REDUCED_COLORSET_SIZE = 8;

constexpr std::array<Color, NR_APPCUI_COLORS> TrueColors = {
    Color::Black, Color::DarkBlue, Color::DarkGreen, Color::Teal, 
    Color::DarkRed, Color::Magenta, Color::Olive, Color::Silver, 
    Color::Gray, Color::Blue, Color::Green, Color::Aqua, 
    Color::Red, Color::Pink, Color::Yellow, Color::White
};

#define COLOR_LIGHT(color) ((color) | (1 << 3))
// Mapping from AppCUI::Console::Color to ncurses colors
std::map<AppColor, int> appcuiColorToCursesColorExtended = {
    /* Black */     {AppColor::Black, COLOR_BLACK},
    /* DarkBlue */  {AppColor::DarkBlue, COLOR_BLUE},
    /* DarkGreen */ {AppColor::DarkGreen, COLOR_GREEN},
    /* Teal */      {AppColor::Teal, COLOR_CYAN},
    /* DarkRed */   {AppColor::DarkRed, COLOR_RED},
    /* Magenta */   {AppColor::Magenta, COLOR_MAGENTA},
    /* Olive */     {AppColor::Olive, COLOR_YELLOW},
    /* Silver */    {AppColor::Silver, COLOR_WHITE},

    /* GRAY */      {AppColor::Gray, COLOR_LIGHT(COLOR_BLACK)},
    /* Blue */      {AppColor::Blue, COLOR_LIGHT(COLOR_BLUE)},
    /* Green */     {AppColor::Green, COLOR_LIGHT(COLOR_GREEN)},
    /* Aqua */      {AppColor::Aqua, COLOR_LIGHT(COLOR_CYAN)},
    /* Red */       {AppColor::Red, COLOR_LIGHT(COLOR_RED)},
    /* Pink */      {AppColor::Pink, COLOR_LIGHT(COLOR_MAGENTA)},
    /* Yellow */    {AppColor::Yellow, COLOR_LIGHT(COLOR_YELLOW)},
    /* White */     {AppColor::White, COLOR_LIGHT(COLOR_WHITE)}
};


// Mapping from AppCUI::Console::Color to ncurses colors but only with 8 base colors
std::map<AppColor, int> appcuiColorToCursesColorReduced = {
    /* Black */     {AppColor::Black, COLOR_BLACK},
    /* DarkBlue */  {AppColor::DarkBlue, COLOR_BLUE},
    /* DarkGreen */ {AppColor::DarkGreen, COLOR_GREEN},
    /* Teal */      {AppColor::Teal, COLOR_CYAN},
    /* DarkRed */   {AppColor::DarkRed, COLOR_RED},
    /* Magenta */   {AppColor::Magenta, COLOR_MAGENTA},
    /* Olive */     {AppColor::Olive, COLOR_YELLOW},
    /* Silver */    {AppColor::Silver, COLOR_WHITE},

    /* GRAY */      {AppColor::Gray, COLOR_BLACK},
    /* Blue */      {AppColor::Blue, COLOR_BLUE},
    /* Green */     {AppColor::Green, COLOR_GREEN},
    /* Aqua */      {AppColor::Aqua, COLOR_CYAN},
    /* Red */       {AppColor::Red, COLOR_RED},
    /* Pink */      {AppColor::Pink, COLOR_MAGENTA},
    /* Yellow */    {AppColor::Yellow, COLOR_YELLOW},
    /* White */     {AppColor::White, COLOR_WHITE}
};

// In ncurses, we have to declare the color we're gonna use
// That means we have to say init_pair(1, COLOR_BLACK, COLOR_BLUE)
// This will define a color pair of foreground black and background blue that has ID = 1
// ID = 0 is reserved by WHITE foreground on BLACK background
//
// For every foreground-background pair we have to declare an ID, so it is accessible 
// after it has been initialized
//
// This id could have had the formula of 1 + fg * NR_COLORS + bg, 0 <= bg <= 15, 0 <= fg <= 15,
// until we realize that since we can't use id 0, we can't fit all the 256 colors in 255 clots (without 0)
//
// This way - we arrive at another formula (1 + fg * NR_COLORS + bg), BUT without declaring
// the pair (0, COLOR_WHITE, COLOR_BLACK)
//
// Thus we have this mapping array, where the key is (fg * NR_COLORS + bg) and the 
// result is the pair id 
//
// Notice the little 0 after 240 (it's fg=WHITE=15 * NR_COLORS + bg=BLACK=0)
constexpr std::array<int, NR_APPCUI_COLORS*NR_APPCUI_COLORS> pairMappingExtended = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
    129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
    145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
    161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
    177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
    193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
    209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240,
    0, 241, 242, 0243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};

constexpr std::array<int, NR_APPCUI_COLORS*NR_APPCUI_COLORS>  pairMappingReduced = {
    1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 
    9, 10, 11, 12, 13, 14, 15, 16, 0, 0, 0, 0, 0, 0, 0, 0, 
    17, 18, 19, 20, 21, 22, 23, 24, 0, 0, 0, 0, 0, 0, 0, 0, 
    25, 26, 27, 28, 29, 30, 31, 32, 0, 0, 0, 0, 0, 0, 0, 0, 
    33, 34, 35, 36, 37, 38, 39, 40, 0, 0, 0, 0, 0, 0, 0, 0, 
    41, 42, 43, 44, 45, 46, 47, 48, 0, 0, 0, 0, 0, 0, 0, 0, 
    49, 50, 51, 52, 53, 54, 55, 56, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

ColorManager::ColorManager(): nrColors(0)
{
    
}

void ColorManager::Init()
{
    start_color();
    use_default_colors();
    nrColors = COLORS;

    if (nrColors >= EXTENDED_COLORSET_SIZE)
    {
        nrColors = EXTENDED_COLORSET_SIZE;
        appcuiColorMapping = appcuiColorToCursesColorExtended;
        pairMapping = pairMappingExtended;
    }
    else if (nrColors >= REDUCED_COLORSET_SIZE)
    {
        nrColors = REDUCED_COLORSET_SIZE;
        appcuiColorMapping = appcuiColorToCursesColorExtended;
        pairMapping = pairMappingReduced;
    }
    else 
    {
        nrColors = 0;
    }

    initColorPairs();
}

constexpr int ColorManager::getPairId(const AppColor fg, const AppColor bg) 
{
    const char fgMapped = std::underlying_type<AppColor>::type(appcuiColorMapping[fg]);
    const char bgMapped = std::underlying_type<AppColor>::type(appcuiColorMapping[bg]);
    return pairMapping[fgMapped * NR_APPCUI_COLORS + bgMapped];
}

void ColorManager::initColorPairs(void)
{

    for (const AppColor& fg: TrueColors) {
        for (const AppColor& bg: TrueColors) {
            const size_t pair_id = getPairId(fg, bg);
            if (pair_id == 0) continue;
            init_pair(pair_id, appcuiColorMapping[fg], appcuiColorMapping[bg]);
        }
    }
}

void ColorManager::SetColor(const AppColor fg, const AppColor bg)
{
    if (nrColors == 0) return;
    const int color_num = getPairId(fg, bg);
    attron(COLOR_PAIR(color_num));
}

void ColorManager::UnsetColor(const AppColor fg, const AppColor bg)
{
    if (nrColors == 0) return;
    const int color_num = getPairId(fg, bg);
    attroff(COLOR_PAIR(color_num));
}

void ColorManager::ResetColor()
{
    if (nrColors == 0) return;
    attron(COLOR_PAIR(0));
}