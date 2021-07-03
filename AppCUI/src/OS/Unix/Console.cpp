#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <array>
#include <ncurses.h>

#include "os.h"
#include "Internal.h"

using namespace AppCUI::Internal;

const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

int _special_characters_consolas_font[AppCUI::Console::SpecialChars::Count] = {
    0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551,                     // double line box
    0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502,                     // single line box
    0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                     // arrows
    32, 0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590, // blocks
    0x25CF, 0x25CB, 0x221A,                                             // symbols
};

const size_t NR_COLORS = 16;

#define COLOR_LIGHT(color) ((color) | (1 << 3))

// Mapping from AppCUI::Console::Color to ncurses colors
constexpr int internal_color_mapping[NR_COLORS] = {
    /* Black */     COLOR_BLACK,
    /* DarkBlue */  COLOR_BLUE,
    /* DarkGreen */ COLOR_GREEN,
    /* Teal */      COLOR_CYAN,
    /* DarkRed */   COLOR_RED,
    /* Magenta */   COLOR_MAGENTA,
    /* Olive */     COLOR_YELLOW,
    /* Silver */    COLOR_WHITE,

    /* GRAY */      COLOR_LIGHT(COLOR_BLACK),
    /* Blue */      COLOR_LIGHT(COLOR_BLUE),
    /* Green */     COLOR_LIGHT(COLOR_GREEN),
    /* Aqua */      COLOR_LIGHT(COLOR_CYAN),
    /* Red */       COLOR_LIGHT(COLOR_RED),
    /* Pink */      COLOR_LIGHT(COLOR_MAGENTA),
    /* Yellow */    COLOR_LIGHT(COLOR_YELLOW),
    /* White */     COLOR_LIGHT(COLOR_WHITE),
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
constexpr int pairMapping[] = {
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



constexpr int getPairId(int fg, int bg) 
{
    fg = internal_color_mapping[fg];
    bg = internal_color_mapping[bg];
    return pairMapping[fg * NR_COLORS + bg];
}

void initColorPairs(void)
{
    for (size_t fg = 0; fg < NR_COLORS; fg++) {
        for (size_t bg = 0; bg < NR_COLORS; bg++) {
            const size_t pair_id = getPairId(fg, bg);
            if (pair_id == 0) continue;
            init_pair(pair_id, internal_color_mapping[fg], internal_color_mapping[bg]);
        }
    }
}

void setColor(int fg, int bg)
{
    const int color_num = getPairId(fg, bg);
    attron(COLOR_PAIR(color_num));
}

void unsetColor(int fg, int bg)
{
    const int color_num = getPairId(fg, bg);
    attroff(COLOR_PAIR(color_num));
}

Console::Console() : AbstractConsole(), term_has_colors(false)
{
}

bool Console::OnInit()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    clear();
    
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);

    //raw();
    use_extended_names(true);

    if (has_colors())
    {
        term_has_colors = true;
        start_color();
        use_default_colors();
        initColorPairs();
    }

    size_t width = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height < MAX_TTY_ROW || width < MAX_TTY_COL, false, "Failed to get window sizes");

    SpecialCharacters = _special_characters_consolas_font;
    this->BeforeInitConfig.consoleSize.Set(width, height);
    this->BeforeInitConfig.screenBuffer = new CHAR_INFO[BeforeInitConfig.consoleSize.Width * BeforeInitConfig.consoleSize.Height];
    CHECK(this->CreateScreenBuffers(width, height), false, "Fail to create screen buffers");
    return true;
}

void Console::OnUninit()
{ 
    endwin();
}

void drawPalette() {
    clear();
    for (size_t fg = 0; fg < NR_COLORS; fg++) 
    {
        for (size_t bg = 0; bg < NR_COLORS; bg++) 
        {
            setColor(fg, bg);
            mvaddch(fg, bg, 'a');
            unsetColor(fg, bg);
        }
    }
    refresh();
}

void Console::OnFlushToScreen()
{
    //drawPalette();    
    //return;
    for (size_t y = 0; y < ConsoleSize.Height; y++)
    {
        for (size_t x = 0; x < ConsoleSize.Width; x++)
        {
            CHAR_INFO& ch = WorkingBuffer[y * ConsoleSize.Width + x];
            const int fg = (ch.characterColor & 0xF);
            const int bg = (ch.characterColor >> 4);
            cchar_t t = {0, {ch.characterCode, 0}};
            if (term_has_colors)
            {
                setColor(fg, bg);
                mvadd_wch(y, x, &t);
                unsetColor(fg, bg);
            }
            else 
            {
                mvadd_wch(y, x, &t);
            }
        }
    }
    refresh();
}


bool Console::OnUpdateCursor()
{
    if (Cursor.Visible)
    {
        curs_set(1);
        move(Cursor.Y, Cursor.X);
    }
    else
    {
        curs_set(0);
    }
    refresh();
    return true;
}

Console::~Console()
{
}
