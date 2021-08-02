#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>
#include <string>

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

constexpr int KEY_DELETE      = 0x7F;
constexpr int INSERT_MODE_KEY = 'i';    // Enter insert mode
constexpr int KEY_ESCAPE      = '\x1B'; // ESC key
constexpr int KEY_TAB         = '\t';

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
    cbreak();
    noecho();
    nonl();
    raw();
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    set_escdelay(0);

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        keyTranslationMatrixInsert[KEY_F(i + 1)] = static_cast<Key>(static_cast<unsigned int>(Key::F1) + i);

        // If we press F1 + shift => it generates F13
        keyTranslationMatrixInsert[KEY_F(i + 13)] =
              static_cast<Key>(static_cast<unsigned int>(Key::F1) + i) | Key::Shift;
    }

    keyTranslationMatrixInsert[KEY_ENTER]     = Key::Enter;
    keyTranslationMatrixInsert[13]            = Key::Enter;
    keyTranslationMatrixInsert[10]            = Key::Enter;
    keyTranslationMatrixInsert[KEY_UP]        = Key::Up;
    keyTranslationMatrixInsert[KEY_RIGHT]     = Key::Right;
    keyTranslationMatrixInsert[KEY_DOWN]      = Key::Down;
    keyTranslationMatrixInsert[KEY_LEFT]      = Key::Left;
    keyTranslationMatrixInsert[KEY_PPAGE]     = Key::PageUp;
    keyTranslationMatrixInsert[KEY_NPAGE]     = Key::PageDown;
    keyTranslationMatrixInsert[KEY_HOME]      = Key::Home;
    keyTranslationMatrixInsert[KEY_END]       = Key::End;
    keyTranslationMatrixInsert[KEY_BACKSPACE] = Key::Backspace;
    keyTranslationMatrixInsert[KEY_DELETE]    = Key::Backspace;
    keyTranslationMatrixInsert[KEY_ESCAPE]    = Key::Escape;
    keyTranslationMatrixInsert[KEY_TAB]       = Key::Tab;

    // In normal mode allow only ESC
    keyTranslationMatrixNormal[KEY_ESCAPE] = Key::Escape;

    mode       = TerminalMode::TerminalNormal;
    shiftState = Key::Ctrl;

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

void NcursesTerminal::handleKeyNormalMode(SystemEvents::Event& evt, const int c)
{
    if (c == INSERT_MODE_KEY)
    {
        mode          = TerminalMode::TerminalInsert;
        evt.eventType = SystemEvents::REDRAW;
        return;
    }

    if (keyTranslationMatrixNormal.find(c) != keyTranslationMatrixNormal.end())
    {
        evt.keyCode = keyTranslationMatrixNormal[c];
        return;
    }

    if (islower(c))
    {
        evt.keyCode |= Key::Ctrl | static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'a'));
        return;
    }
    else if (isupper(c))
    {
        evt.keyCode |= Key::Ctrl | Key::Shift | static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'A'));
        return;
    }
    else if (isdigit(c))
    {
        evt.keyCode |= Key::Ctrl | static_cast<Key>(static_cast<unsigned int>(Key::N0) + (c - '0'));
        return;
    }

    evt.eventType = SystemEvents::NONE;
}

void NcursesTerminal::handleKeyInsertMode(SystemEvents::Event& evt, const int c)
{
    if (c == KEY_ESCAPE) // ESC
    {
        mode          = TerminalMode::TerminalNormal;
        evt.eventType = SystemEvents::REDRAW;
        return;
    }

    if (keyTranslationMatrixInsert.find(c) != keyTranslationMatrixInsert.end())
    {
        evt.keyCode = keyTranslationMatrixInsert[c];
        return;
    }

    if ((c >= 32) && (c <= 127))
    {
        evt.asciiCode = c;
        if (islower(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'a'));
        }
        else if (isupper(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'A'));
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

    evt.eventType = SystemEvents::NONE;
}

void NcursesTerminal::handleKey(SystemEvents::Event& evt, const int c)
{
    debugChar(0, c, "key");
    evt.eventType = SystemEvents::KEY_PRESSED;
    if (mode == TerminalMode::TerminalNormal)
    {
        handleKeyNormalMode(evt, c);
    }
    else if (mode == TerminalMode::TerminalInsert)
    {
        handleKeyInsertMode(evt, c);
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