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
#include "Color.h"

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


Console::Console() : AbstractConsole()
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
    raw();
    nonl();

    use_extended_names(true);

    start_color();
    use_default_colors();
    colors.Init(COLORS);

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

void Console::OnFlushToScreen()
{
    for (size_t y = 0; y < ConsoleSize.Height; y++)
    {
        for (size_t x = 0; x < ConsoleSize.Width; x++)
        {
            CHAR_INFO& ch = WorkingBuffer[y * ConsoleSize.Width + x];
            const auto fg = static_cast<AppCUI::Console::Color::Type>(ch.characterColor & 0xF);
            const auto bg = static_cast<AppCUI::Console::Color::Type>(ch.characterColor >> 4);

            cchar_t t = {0, {ch.characterCode, 0}};
            colors.SetColor(fg, bg);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(fg, bg);
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
