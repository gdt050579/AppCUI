#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>
#include <string>

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

constexpr int KEY_DELETE = 0x7F;

bool NcursesTerminal::OnInit(const InitializationData& initData)
{
    mode = TerminalMode::TerminalNormal;

    if (!initScreen())
        return false;
    if (!initInput())
        return false;
    return true;
}

void NcursesTerminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}

void debugChar(int y, int c, const char* prefix)
{
    std::string_view myName = keyname(c);
    move(y, 0);
    clrtoeol();
    addstr((std::string(prefix) + " " + std::to_string(c) + " " + myName.data()).c_str());
}

bool NcursesTerminal::initInput()
{
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    raw();
    nonl();
    set_escdelay(0);

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        keyTranslationMatrixNormal[KEY_F(i + 1)] = static_cast<Key>(static_cast<unsigned int>(Key::F1) + i);

        // If we press F1 + shift => it generates F13
        keyTranslationMatrixNormal[KEY_F(i + 13)] =
              static_cast<Key>(static_cast<unsigned int>(Key::F1) + i) | Key::Shift;
    }

    keyTranslationMatrixNormal[KEY_ENTER] = Key::Enter;
    keyTranslationMatrixNormal[13]        = Key::Enter;
    keyTranslationMatrixNormal[10]        = Key::Enter;

    keyTranslationMatrixNormal[KEY_UP]    = Key::Up;
    keyTranslationMatrixNormal[KEY_RIGHT] = Key::Right;
    keyTranslationMatrixNormal[KEY_DOWN]  = Key::Down;
    keyTranslationMatrixNormal[KEY_LEFT]  = Key::Left;
    keyTranslationMatrixNormal[KEY_PPAGE] = Key::PageUp;
    keyTranslationMatrixNormal[KEY_NPAGE] = Key::PageDown;
    keyTranslationMatrixNormal[KEY_HOME]  = Key::Home;
    keyTranslationMatrixNormal[KEY_END]   = Key::End;
    keyTranslationMatrixNormal['t']       = Key::Tab;

    keyTranslationMatrixNormal[KEY_BACKSPACE] = Key::Backspace;
    keyTranslationMatrixNormal[0x7F]          = Key::Backspace;
    keyTranslationMatrixNormal[0x1B]          = Key::Escape;

    // keyTranslationMatrixNormal[] = Key::Tab;
    // keyTranslationMatrixNormal[KEY_ESCAPE] ?
    // keyTranslationMatrixNormal[KEY_INSERT] ?
    // keyTranslationMatrixNormal[KEY_DELETE] = Key::Delete;

    return true;
}

void NcursesTerminal::handleMouse(SystemEvents::Event& evt, const int c)
{
    MEVENT mouseEvent;
    if (getmouse(&mouseEvent) == OK)
    {
        evt.mouseX        = mouseEvent.x;
        evt.mouseY        = mouseEvent.y;
        const auto& state = mouseEvent.bstate;

        if (state & BUTTON1_PRESSED)
        {
            evt.eventType = SystemEvents::MOUSE_DOWN;
        }
        else if (state & BUTTON1_RELEASED)
        {
            evt.eventType = SystemEvents::MOUSE_UP;
        }
        else if (state & REPORT_MOUSE_POSITION)
        {
            evt.eventType = SystemEvents::MOUSE_MOVE;
        }
    }
}

void NcursesTerminal::handleKey(SystemEvents::Event& evt, const int c)
{
    // debugChar(0, c, "key");
    evt.eventType = SystemEvents::KEY_PRESSED;
    if (mode == TerminalMode::TerminalNormal)
    {
        if (c == 'i')
        {
            mode          = TerminalMode::TerminalInsert;
            evt.eventType = SystemEvents::REDRAW;
            return;
        }
        else if (keyTranslationMatrixNormal.find(c) != keyTranslationMatrixNormal.end())
        {
            evt.keyCode = keyTranslationMatrixNormal[c];
            return;
        }
        else
        {
            evt.eventType = SystemEvents::NONE;
            return;
        }
    }
    else if (mode == TerminalMode::TerminalInsert)
    {
        if (c == '\x1B') // ESC
        {
            mode          = TerminalMode::TerminalNormal;
            evt.eventType = SystemEvents::REDRAW;
            return;
        }
        else if (keyTranslationMatrixNormal.find(c) != keyTranslationMatrixNormal.end())
        {
            evt.keyCode = keyTranslationMatrixNormal[c];
            return;
        }
        else if ((c >= 32) && (c <= 127))
        {
            evt.asciiCode = c;
            if (islower(c))
            {
                evt.keyCode |= static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'a'));
            }
            else if (isupper(c))
            {
                evt.keyCode |= Key::Shift | static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'A'));
            }
            else if (isdigit(c))
            {
                evt.keyCode |= static_cast<Key>(static_cast<unsigned int>(Key::N0) + (c - '0'));
            }
            else if (c == ' ')
            {
                evt.keyCode |= Key::Space;
            }
            return;
        }
        else
        {
            evt.eventType = SystemEvents::NONE;
            return;
        }
    }
}

void NcursesTerminal::GetSystemEvent(AppCUI::Internal::SystemEvents::Event& evnt)
{
    evnt.eventType = SystemEvents::NONE;
    evnt.keyCode   = Key::None;
    evnt.asciiCode = 0;
    // select on stdin with timeout, should  translate to about ~30 fps
    pollfd readFD;
    readFD.fd     = STDIN_FILENO;
    readFD.events = POLLIN | POLLERR;
    // poll for 30 milliseconds
    poll(&readFD, 1, 30);

    int c = getch();
    if (c == ERR)
    {
        return;
    }
    else if (c == KEY_MOUSE)
    {
        handleMouse(evnt, c);
        return;
    }
    else if (c == KEY_RESIZE)
    {
        // one day, but this day is not today
        // evnt.eventType = SystemEvents::APP_RESIZED;
        return;
    }
    else
    {
        handleKey(evnt, c);
        return;
    }
    refresh();
}
bool NcursesTerminal::IsEventAvailable()
{
    NOT_IMPLEMENTED(false);
}
void NcursesTerminal::uninitInput()
{
}

const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

bool NcursesTerminal::initScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
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
    AppCUI::Console::Character* charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const size_t width                      = ScreenCanvas.GetWidth();
    const size_t height                     = ScreenCanvas.GetHeight();
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const AppCUI::Console::Character ch = charsBuffer[y * width + x];

            cchar_t t = { 0, { ch.Code, 0 } };
            colors.SetColor(ch.Color.Forenground, ch.Color.Background);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(ch.Color.Forenground, ch.Color.Background);
        }
    }
    if (mode == TerminalMode::TerminalInsert)
    {
        colors.SetColor(Console::Color::White, Console::Color::Green);
        mvaddch(height - 1, width - 3, ' ');
        mvaddch(height - 1, width - 2, 'I');
        mvaddch(height - 1, width - 1, ' ');
        colors.UnsetColor(Console::Color::White, Console::Color::Green);
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
