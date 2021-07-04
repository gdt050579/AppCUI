#include "os.h"
#include <ncurses.h>
#include <poll.h>

// See docs/TERMINAL.md

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

constexpr size_t KEY_TRANSLATION_MATRIX_SIZE = KEY_MAX;
constexpr int KEY_DELETE = 0x7F;

Input::Input() : AbstractInput()
{
    shiftState = Key::None;

    KeyTranslationMatrix.reserve(KEY_TRANSLATION_MATRIX_SIZE);
    for (size_t tr = 0; tr < KEY_TRANSLATION_MATRIX_SIZE; tr++)
    {
        this->KeyTranslationMatrix[tr] = AppCUI::Input::Key::None;
    }
    for (size_t i = 0; i < 12; i++)
    {
        KeyTranslationMatrix[KEY_F(i+1)] = static_cast<Key::Type>(Key::F1 + i);
    }
}

Input::~Input()
{
}

bool Input::Init()
{
    KeyTranslationMatrix[KEY_ENTER] = Key::Enter;
    //KeyTranslationMatrix[KEY_ESCAPE] ? 
    //KeyTranslationMatrix[KEY_INSERT] ? 
    KeyTranslationMatrix[KEY_BACKSPACE] = Key::Backspace;
    // KeyTranslationMatrix[KEY_DELETE] = Key::Backspace;
    // KeyTranslationMatrix['\t'] = Key::Tab;

    KeyTranslationMatrix[KEY_UP] = Key::Up;
    KeyTranslationMatrix[KEY_RIGHT] = Key::Right;
    KeyTranslationMatrix[KEY_DOWN] = Key::Down;
    KeyTranslationMatrix[KEY_LEFT] = Key::Left;

    KeyTranslationMatrix[KEY_PPAGE] = Key::PageUp;
    KeyTranslationMatrix[KEY_NPAGE] = Key::PageDown;

    KeyTranslationMatrix[KEY_HOME] = Key::Home;
    KeyTranslationMatrix[KEY_END] = Key::End;

    KeyTranslationMatrix[' '] = Key::Space;
    return true;
}

void Input::Uninit()
{
}

void debugChar(int y, int c, const char* prefix)
{
    std::string_view myName = keyname(c);
    move(y, 0);
    clrtoeol();
    addstr((std::string(prefix) + " " + std::to_string(c) + " " + myName.data()).c_str());
}

void Input::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
    evnt.eventType = SystemEvents::NONE;

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

    switch (c)
    {
        case KEY_MOUSE:
        {
            // Need xterm-1003 for mouse events & colors
            MEVENT mouseEvent;
            if (getmouse(&mouseEvent) == OK)
            {
                evnt.mouseX = mouseEvent.x;
                evnt.mouseY = mouseEvent.y;
                const auto &state = mouseEvent.bstate;
                
                if (state & BUTTON1_PRESSED) 
                {
                    evnt.eventType = SystemEvents::MOUSE_DOWN;
                }
                else if (state & BUTTON1_RELEASED)
                {
                    evnt.eventType = SystemEvents::MOUSE_UP;
                }
                else if (state & REPORT_MOUSE_POSITION) 
                {
                    evnt.eventType = SystemEvents::MOUSE_MOVE;
                }
            }
            break;
        }
        default:
        {
            //debugChar(0, c, "key");
            evnt.eventType = SystemEvents::KEY_PRESSED;
            if (c > KEY_MIN && c < KEY_MAX && KeyTranslationMatrix[c] != Key::None)
            {
                evnt.keyCode = KeyTranslationMatrix[c];
                break;
            }
            else if (c >= KEY_F(13) && c <= KEY_F(24))
            {
                // F1 - F12 + shift
                evnt.keyCode = KeyTranslationMatrix[c - KEY_F(12)] | Key::Shift;
                break;
            }
            else if ((c >= 32) && (c <= 127))
            {
                evnt.asciiCode = c;
                //debugChar(0, c, "normal key");
                // This is only for letters
                if (isupper(c))
                {
                    //debugChar(0, c, "shift key");
                    evnt.keyCode = Key::Shift;
                }

                break;
            }
            else 
            {
                evnt.eventType = SystemEvents::NONE;
            }
            break;
        }
    }
    refresh();
    timeout(10);
}
