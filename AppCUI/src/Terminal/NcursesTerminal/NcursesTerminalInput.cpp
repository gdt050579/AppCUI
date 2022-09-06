#include "NcursesTerminal.hpp"
#include "ncurses.h"
#include <poll.h>

namespace AppCUI::Internal
{
using namespace Input;

constexpr int KEY_DELETE      = 0x7F;
constexpr int INSERT_MODE_KEY = 'i';    // Enter insert mode
constexpr int KEY_ESCAPE      = '\x1B'; // ESC key
constexpr int KEY_TAB         = '\t';

void DebugChar(int y, int c, const char* prefix)
{
    string_view myName = keyname(c);
    move(y, 0);
    clrtoeol();
    addstr((std::string(prefix) + " " + std::to_string(c) + " " + myName.data()).c_str());
}

bool NcursesTerminal::InitInput()
{
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    nonl();
    raw();
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
    mouseinterval(0);
    set_escdelay(0);

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        keyTranslationMatrix[KEY_F(i + 1)] = static_cast<Key>(static_cast<uint32>(Key::F1) + i);

        // If we press F1 + shift => it generates F13
        keyTranslationMatrix[KEY_F(i + 13)] = static_cast<Key>(static_cast<uint32>(Key::F1) + i) | Key::Shift;
    }

    keyTranslationMatrix[KEY_ENTER]     = Key::Enter;
    keyTranslationMatrix[13]            = Key::Enter;
    keyTranslationMatrix[10]            = Key::Enter;
    keyTranslationMatrix[KEY_ESCAPE]    = Key::Escape;
    keyTranslationMatrix[KEY_DELETE]    = Key::Delete;
    keyTranslationMatrix[KEY_BACKSPACE] = Key::Backspace;
    keyTranslationMatrix[KEY_TAB]       = Key::Tab;
    keyTranslationMatrix[KEY_LEFT]      = Key::Left;
    keyTranslationMatrix[KEY_UP]        = Key::Up;
    keyTranslationMatrix[KEY_DOWN]      = Key::Down;
    keyTranslationMatrix[KEY_RIGHT]     = Key::Right;
    keyTranslationMatrix[KEY_PPAGE]     = Key::PageUp;
    keyTranslationMatrix[KEY_NPAGE]     = Key::PageDown;
    keyTranslationMatrix[KEY_HOME]      = Key::Home;
    keyTranslationMatrix[KEY_END]       = Key::End;
    keyTranslationMatrix[KEY_DELETE]    = Key::Backspace;

    mode = TerminalMode::TerminalNormal;

    return true;
}

void NcursesTerminal::HandleMouse(SystemEvent& evt, const int)
{
    MEVENT mouseEvent;
    if (getmouse(&mouseEvent) == OK)
    {
        evt.mouseX        = mouseEvent.x;
        evt.mouseY        = mouseEvent.y;
        const auto& state = mouseEvent.bstate;

        if (state & BUTTON1_PRESSED)
        {
            evt.eventType = SystemEventType::MouseDown;
        }
        else if (state & BUTTON1_RELEASED)
        {
            evt.eventType = SystemEventType::MouseUp;
        }
        else if (state & REPORT_MOUSE_POSITION)
        {
            evt.eventType = SystemEventType::MouseMove;
        }
    }
}

void NcursesTerminal::HandleKeyNormalMode(SystemEvent& evt, const int c)
{
    if (c == INSERT_MODE_KEY)
    {
        mode          = TerminalMode::TerminalInsert;
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    }

    if (keyTranslationMatrix.find(c) != keyTranslationMatrix.end())
    {
        evt.keyCode = keyTranslationMatrix[c];
        return;
    }

    if (islower(c))
    {
        evt.keyCode |= Key::Ctrl | static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'a'));
        return;
    }
    else if (isupper(c))
    {
        evt.keyCode |= Key::Ctrl | Key::Shift | static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'A'));
        return;
    }
    else if (isdigit(c))
    {
        evt.keyCode |= Key::Ctrl | static_cast<Key>(static_cast<uint32>(Key::N0) + (c - '0'));
        return;
    }

    evt.eventType = SystemEventType::None;
}

void NcursesTerminal::HandleKeyInsertMode(SystemEvent& evt, const int c)
{
    if (c == KEY_ESCAPE) // ESC
    {
        mode          = TerminalMode::TerminalNormal;
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    }

    if (keyTranslationMatrix.find(c) != keyTranslationMatrix.end())
    {
        evt.keyCode = keyTranslationMatrix[c];
        return;
    }

    if ((c >= 32) && (c <= 127))
    {
        evt.unicodeCharacter = c;
        if (islower(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'a'));
        }
        else if (isupper(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'A'));
        }
        else if (isdigit(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<uint32>(Key::N0) + (c - '0'));
        }
        else if (c == ' ')
        {
            evt.keyCode |= Key::Space;
        }
        return;
    }

    evt.eventType = SystemEventType::None;
}

void NcursesTerminal::HandleKey(SystemEvent& evt, const int c)
{
    DebugChar(0, c, "key");
    evt.eventType = SystemEventType::KeyPressed;
    if (mode == TerminalMode::TerminalNormal)
    {
        HandleKeyNormalMode(evt, c);
    }
    else if (mode == TerminalMode::TerminalInsert)
    {
        HandleKeyInsertMode(evt, c);
    }
}

void NcursesTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    evnt.eventType        = SystemEventType::None;
    evnt.keyCode          = Key::None;
    evnt.unicodeCharacter = 0;
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
        HandleMouse(evnt, c);
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
        HandleKey(evnt, c);
        return;
    }
    refresh();
}
bool NcursesTerminal::IsEventAvailable()
{
    NOT_IMPLEMENTED(false);
}

void NcursesTerminal::UnInitInput()
{
}
}