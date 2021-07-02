#include "os.h"
#include <ncurses.h>

// See docs/TERMINAL.md

using namespace AppCUI::Internal;

Input::~Input()
{
}

bool Input::Init()
{
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    return true;
}

void Input::Uninit()
{
}

void Input::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
    evnt.eventType = SystemEvents::NONE;

    int c = getch();
    if (c == ERR) 
    {
        return;    
    }

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
            //mvaddstr(0, 0, (std::to_string(mouse_event.y) +  " " + std::to_string(mouse_event.x)).c_str());

            if (((state & BUTTON1_PRESSED) != 0) || ((state & BUTTON1_RELEASED) != 0))
            {
                if (state & BUTTON1_PRESSED) 
                {
                    evnt.eventType = SystemEvents::MOUSE_DOWN;
                    //mvaddstr(mouse_event.y, mouse_event.x, "pressed");
                }
                else if (state & BUTTON1_RELEASED)
                {
                    evnt.eventType = SystemEvents::MOUSE_UP;
                    //mvaddstr(mouse_event.y, mouse_event.x, "released");
                }
            }
            else if (state & REPORT_MOUSE_POSITION) 
            {
                evnt.eventType = SystemEvents::MOUSE_MOVE;
                //mvaddstr(mouse_event.y, mouse_event.x, "update");
            }
        }
        break;
    }
    refresh();
}
