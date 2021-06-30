#define _XOPEN_SOURCE_EXTENDED
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
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

Console::~Console()
{

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
    clear();
    for (size_t y = 0; y < ConsoleSize.Height; y++)
    {
        for (size_t x = 0; x < ConsoleSize.Width; x++)
        {
            CHAR_INFO& ch = WorkingBuffer[y * ConsoleSize.Width + x];
            //auto pair = init_pair(y * ConsoleSize.Width + x, ch.characterColor & 0xF, ch.characterColor >> 4);
            //attron(COLOR_PAIR(pair));
            cchar_t t{0, {ch.characterCode, 0}};
            mvadd_wch(y, x, &t);
            //attroff(COLOR_PAIR(pair));

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