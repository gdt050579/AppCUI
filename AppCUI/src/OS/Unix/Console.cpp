#define _XOPEN_SOURCE_EXTENDED
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


const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

bool constexpr is_invalid_sizes(const size_t width, const size_t height)
{
    return height >= MAX_TTY_ROW || width >= MAX_TTY_COL;
}

int _special_characters_consolas_font[AppCUI::Console::SpecialChars::Count] = {
    0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551,                     // double line box
    0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502,                     // single line box
    0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                     // arrows
    32, 0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590, // blocks
    0x25CF, 0x25CB, 0x221A,                                             // symbols
};

using namespace AppCUI::Internal;

#define COLOR_LIGHT(color) ((color) | (1 << 3))

constexpr auto color_mapping = std::array<int, 18>{
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

    /* Transparent */ -1,
    /* NoColor     */ -1,
};

int map_internal_color_to_ncurses(int color) 
{
    if (color == AppCUI::Console::Color::Transparent) 
    {
        return 16;
    }
    if (color == AppCUI::Console::Color::NoColor) 
    {
        return 17;
    }
    return color;  
}

constexpr int get_pair_id(const int fg, const int bg) 
{
    return 1 + fg * color_mapping.size() + bg;
}

void init_colorpairs(void)
{
    for (size_t fg = 0; fg < color_mapping.size(); fg++) {
        for (size_t bg = 0; bg < color_mapping.size(); bg++) {
            init_pair(get_pair_id(fg, bg), color_mapping[fg], color_mapping[bg]);
        }
    }
}

void setcolor(int fg, int bg)
{
    fg = map_internal_color_to_ncurses(fg);
    bg = map_internal_color_to_ncurses(bg);
    const int color_num = get_pair_id(fg, bg);
    attron(COLOR_PAIR(color_num));
}

void unsetcolor(int fg, int bg)
{
    fg = map_internal_color_to_ncurses(fg);
    bg = map_internal_color_to_ncurses(bg);
    const int color_num = get_pair_id(fg, bg);
    attroff(COLOR_PAIR(color_num));
}

bool Console::OnInit()
{
#ifndef NO_CURSES
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    clear();

    start_color();
    use_default_colors();
    init_colorpairs();

    size_t width = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(!is_invalid_sizes(width, height), false, "Failed to get window sizes");

    SpecialCharacters = _special_characters_consolas_font;
    this->BeforeInitConfig.consoleSize.Set(width, height);
    this->BeforeInitConfig.screenBuffer = new CHAR_INFO[BeforeInitConfig.consoleSize.Width * BeforeInitConfig.consoleSize.Height];
    CHECK(this->CreateScreenBuffers(width, height), false, "Fail to create screen buffers");
#endif
    return true;
}

void Console::OnUninit()
{ 
#ifndef NO_CURSES
    endwin();
#endif 
}

void Console::OnFlushToScreen()
{
#ifndef NO_CURSES
    //clear();
    /*
    clear();
    for (size_t fg = 0; fg < color_mapping.size(); fg++) {
        for (size_t bg = 0; bg < color_mapping.size(); bg++) {
            setcolor(fg, bg);
            mvaddch(fg, bg, 'a');
            unsetcolor(fg, bg);
        }
    }

    refresh();
    return;
    */
    for (size_t y = 0; y < ConsoleSize.Height; y++)
    {
        for (size_t x = 0; x < ConsoleSize.Width; x++)
        {
            CHAR_INFO& ch = WorkingBuffer[y * ConsoleSize.Width + x];
            const int fg = ch.characterColor & 0xF;
            const int bg = ch.characterColor >> 4;
            setcolor(fg, bg);
            cchar_t t = {0, {ch.characterCode, 0}};
            mvadd_wch(y, x, &t);
            unsetcolor(fg, bg);
        }
    }
    refresh();
#endif 
}
bool Console::OnUpdateCursor()
{
#ifndef NO_CURSES
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
#endif 
    return true;
}

Console::~Console()
{
}
