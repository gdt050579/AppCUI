#include <string>

#include "os.h"
#include "ncurses.h"
#include "Color.h"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;


const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

int _special_characters_consolas_font[AppCUI::Console::SpecialChars::Count] = {
    0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551,                     // double line box
    0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502,                     // single line box
    0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                     // arrows
    32, 0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590, // blocks
    0x25CF, 0x25CB, 0x221A,                                             // symbols
};

bool Terminal::initScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    clear();

    colors.Init();

    size_t width = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height < MAX_TTY_ROW || width < MAX_TTY_COL, false, "Failed to get window sizes");
    // create canvases
    CHECK(ScreenCanvas.Create(width, height), false, "Fail to create an internal canvas of %d x %d size", width, height);
    CHECK(OriginalScreenCanvas.Create(width, height), false, "Fail to create the original screen canvas of %d x %d size", width, height);
    
    return true;
}

void Terminal::OnFlushToScreen()
{
    AppCUI::Console::Character* charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const size_t width = ScreenCanvas.GetWidth();
    const size_t height = ScreenCanvas.GetHeight();
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const AppCUI::Console::Character ch = charsBuffer[y * width + x];
            cchar_t t = {0, {ch.Code, 0}};
            colors.SetColor(ch.Color.Forenground, ch.Color.Background);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(ch.Color.Forenground, ch.Color.Background);
        }
    }
    refresh();
}

bool Terminal::OnUpdateCursor()
{
    if (ScreenCanvas.GetCursorVisibility())
    {
        curs_set(1);
        move(ScreenCanvas.GetCursorY(), ScreenCanvas.GetCursorX());
    }
    else
    {
        curs_set(0);
    }
    refresh();
    return true;
}

void Terminal::uninitScreen()
{
    endwin();
}