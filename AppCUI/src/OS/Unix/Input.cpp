#include "os.h"
#ifndef NO_CURSES
#include <ncurses.h>
#endif

using namespace AppCUI::Internal;

Input::~Input()
{
}

bool Input::Init()
{
    keypad(stdscr, TRUE);
    mouseinterval(0);
    return true;
}

void Input::Uninit()
{
}

void Input::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
#ifndef NO_CURSES
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    int c = getch();

    switch (c)
    {
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_MOUSE:
        // Need xterm-1003 for mouse events & colors
        MEVENT mouse_event;
        if (getmouse(&mouse_event) == OK)
        {
            evnt.mouseX = mouse_event.x;
            evnt.mouseY = mouse_event.y;
            const auto &state = mouse_event.bstate;
            // mvaddstr(0, 0, (std::to_string(mouse_event.y) +  " " + std::to_string(mouse_event.x)).c_str());

            if ((state & BUTTON1_PRESSED) || (state & BUTTON1_RELEASED))
            {
                if (state & BUTTON1_PRESSED) 
                {
                    evnt.eventType = SystemEvents::MOUSE_DOWN;
                    // mvaddstr(mouse_event.y, mouse_event.x, "pressed");
                }
                if (state & BUTTON1_RELEASED)
                {
                    evnt.eventType = SystemEvents::MOUSE_UP;
                    // mvaddstr(mouse_event.y, mouse_event.x, "released");
                }
            }
            else if (state & REPORT_MOUSE_POSITION) 
            {
                evnt.eventType = SystemEvents::MOUSE_MOVE;
                // mvaddstr(mouse_event.y, mouse_event.x, "update");
            }
        }
        break;
    }
    // refresh();
#endif
}
