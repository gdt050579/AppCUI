#include "NcursesTerminal.hpp"
#include "ncurses.h"

const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

using namespace Internal;

bool NcursesTerminal::initScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    clear();

    colors.Init();

    size_t width  = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height < MAX_TTY_ROW || width < MAX_TTY_COL, false, "Failed to get window sizes");
    // create canvases
    CHECK(ScreenCanvas.Create(width, height),
          false,
          "Fail to create an internal canvas of %d x %d size",
          width,
          height);
    CHECK(OriginalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          width,
          height);

    return true;
}

void NcursesTerminal::OnFlushToScreen()
{
    Graphics::Character* charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const size_t width                      = ScreenCanvas.GetWidth();
    const size_t height                     = ScreenCanvas.GetHeight();
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const Graphics::Character ch = charsBuffer[y * width + x];

            cchar_t t = { 0, { ch.Code, 0 }, {} };
            colors.SetColor(ch.Color.Foreground, ch.Color.Background);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(ch.Color.Foreground, ch.Color.Background);
        }
    }
    if (mode == TerminalMode::TerminalInsert)
    {
        colors.SetColor(Graphics::Color::White, Graphics::Color::Green);
        mvaddch(height - 1, width - 3, ' ');
        mvaddch(height - 1, width - 2, 'I');
        mvaddch(height - 1, width - 1, ' ');
        colors.UnsetColor(Graphics::Color::White, Graphics::Color::Green);
    }
    else if (mode == TerminalMode::TerminalNormal)
    {
        colors.SetColor(Graphics::Color::White, Graphics::Color::DarkRed);
        mvaddch(height - 1, width - 3, ' ');
        mvaddch(height - 1, width - 2, 'N');
        mvaddch(height - 1, width - 1, ' ');
        colors.UnsetColor(Graphics::Color::White, Graphics::Color::DarkRed);
    }
    move(LastCursorY, LastCursorX);
    refresh();
}

bool NcursesTerminal::OnUpdateCursor()
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
void NcursesTerminal::RestoreOriginalConsoleSettings()
{
}

void NcursesTerminal::uninitScreen()
{
    endwin();
}
