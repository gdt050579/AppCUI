#include "os.h"
#include <ncurses.h>
#include <poll.h>

// See docs/TERMINAL.md

/*
    Some of the keys are not yet supported.
    KeyTranslationMatrix is used for low hanging fruit Key command translation.
    Advanced commands that fill more than event.keyCode are processed separately
*/

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

constexpr int KEY_DELETE = 0x7F;
constexpr int META_BIT = (1 << 7);

// iTerm2 with proper Meta key configuration will send characters with the last (8th bit set)
constexpr bool isMetaCharacter8bit(const int val)
{
    return (val & META_BIT) == META_BIT;
}
constexpr int unMetaCharacter8bit(const int val)
{
    return (val & (~META_BIT));
}

void debugChar(int y, int c, const char* prefix)
{
    std::string_view myName = keyname(c);
    move(y, 0);
    clrtoeol();
    addstr((std::string(prefix) + " " + std::to_string(c) + " " + myName.data()).c_str());
}

bool Terminal::initInput()
{
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    raw();
    nonl();

    for (size_t tr = 0; tr < KEY_TRANSLATION_MATRIX_SIZE; tr++)
    {
        this->KeyTranslationMatrix[tr] = Key::None;
    }

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        // If we press F1 + shift => it generates F13
        KeyTranslationMatrix[KEY_F(i+1)] = static_cast<Key::Type>(Key::F1 + i);
        KeyTranslationMatrix[KEY_F(i+13)] = static_cast<Key::Type>(Key::F1 + i) | Key::Shift;
    }

    KeyTranslationMatrix[KEY_ENTER] = Key::Enter;
    KeyTranslationMatrix[13] = Key::Enter;
    KeyTranslationMatrix[10] = Key::Enter;

    KeyTranslationMatrix[KEY_BACKSPACE] = Key::Backspace;
    KeyTranslationMatrix[0x7F] = Key::Backspace;

    
    KeyTranslationMatrix[KEY_UP] = Key::Up;
    KeyTranslationMatrix[KEY_RIGHT] = Key::Right;
    KeyTranslationMatrix[KEY_DOWN] = Key::Down;
    KeyTranslationMatrix[KEY_LEFT] = Key::Left;

    KeyTranslationMatrix[KEY_PPAGE] = Key::PageUp;
    KeyTranslationMatrix[KEY_NPAGE] = Key::PageDown;

    KeyTranslationMatrix[KEY_HOME] = Key::Home;
    KeyTranslationMatrix[KEY_END] = Key::End;

    // KeyTranslationMatrix[] = Key::Tab;
    //KeyTranslationMatrix[KEY_ESCAPE] ? 
    //KeyTranslationMatrix[KEY_INSERT] ? 
    // KeyTranslationMatrix[KEY_DELETE] = Key::Delete;
    return true;
}


void Terminal::handleMouse(SystemEvents::Event &evt, const int c)
{
    MEVENT mouseEvent;
    if (getmouse(&mouseEvent) == OK)
    {
        evt.mouseX = mouseEvent.x;
        evt.mouseY = mouseEvent.y;
        const auto &state = mouseEvent.bstate;
        
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

void Terminal::handleKey(SystemEvents::Event &evt, const int c)
{
    // debugChar(0, c, "key");
    evt.eventType = SystemEvents::KEY_PRESSED;

    if (c < KEY_TRANSLATION_MATRIX_SIZE && KeyTranslationMatrix[c] != Key::None)
    {
        evt.keyCode = KeyTranslationMatrix[c];
        return;
    }
    else if ((c >= 32) && (c <= 127))
    {
        evt.asciiCode = c;
        if (islower(c))
        {
            evt.keyCode |= static_cast<Key::Type>(Key::A + (c - 'a'));
        }
        else if (isupper(c))
        {
            evt.keyCode |= Key::Shift | static_cast<Key::Type>(Key::A + (c - 'A'));
        }
        else if (isdigit(c))
        {
            evt.keyCode |= static_cast<Key::Type>(Key::N0 + (c - '0'));
        }
        else if (c == ' ') 
        {
            evt.keyCode |= Key::Space;
        }
        return;
    }
    else 
    {
        //debugChar(0, c, "unsupported key: ");
        evt.eventType = SystemEvents::NONE;
        return;
    }
}


void Terminal::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
    evnt.eventType = SystemEvents::NONE;
    evnt.keyCode = Key::None;
    evnt.asciiCode = 0;
    // select on stdin with timeout, should  translate to about ~30 fps
    pollfd readFD;
    readFD.fd = STDIN_FILENO;
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
bool Terminal::IsEventAvailable()
{
	NOT_IMPLEMENTED(false);
}
void Terminal::uninitInput()
{
}