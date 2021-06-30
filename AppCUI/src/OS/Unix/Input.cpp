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
    return true;
}
void Input::Uninit()
{
}
void Input::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
#ifndef NO_CURSES
    mousemask(ALL_MOUSE_EVENTS, NULL);
    int c = getch();
    MEVENT mouse_event;
    switch (c)
    {
    case KEY_UP:
        printf("keyup");
        break;
    case KEY_DOWN:
        printf("keydown");
        break;
    case KEY_MOUSE:
        if (getmouse(&mouse_event) == OK)
        {
            evnt.mouseX = mouse_event.x;
            evnt.mouseY = mouse_event.y;
            const auto &state = mouse_event.bstate;

            if (state & BUTTON1_PRESSED)
            {
                evnt.eventType = SystemEvents::MOUSE_DOWN;
                printw("pressed");
            }
            else if (state & BUTTON1_RELEASED)
            {
                evnt.eventType = SystemEvents::MOUSE_UP;
                printw("up");
            }
        }
        break;
    }
#endif
}
