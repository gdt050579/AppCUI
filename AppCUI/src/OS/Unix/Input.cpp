#include "os.h"
#include <ncurses.h>

/**
    Short (not) unix terminal and terminal introduction
    1. https://en.wikipedia.org/wiki/Terminal_emulator
    2. https://en.wikipedia.org/wiki/Xterm (the default terminal emulator on most systems)
    3. https://en.wikipedia.org/wiki/ANSI_escape_code
    4. https://en.wikipedia.org/wiki/Termcap
    5. https://en.wikipedia.org/wiki/Terminfo
    6. https://tldp.org/HOWTO/Text-Terminal-HOWTO-16.html

    The basic gist is as follows:
    
    You have 3 parts of the application:
    1. The terminal (usually a program that runs by itself and can run other programs)
    2. The application that wants to output/draw something on the terminal it runs on
    3. A helper library like ncurses

    What usually happens:
    1. You init ncurses using its functions
    2. Ncurses will find out the terminal you're running from (for example xterm-256color) and check its capabilities,
       for example: can it display 256 colors or 8? how many color pairs can it store? what mouse events does it send.
    3. Along with capabilities, every terminal also defines what control sequences to send so that it can perform
       those capabilities
       For example, to move the cursor to a certain line L and column C, your program has to send "\033[<L>;<C>H"
    4. But you don't have to do it manually, ncurses does it for you, by reading your terminal capabilities file
       and finding all kinds of commands using their names.
       For example "clear=\E[H\E[​2J,", which means that for ncurses to do a screen clear, it can send a
       "\E[H\E[​2J" control sequence to the terminal and it will be interpreted as a clear command. The command
       names are predefined by a standard, but are optional to be implemented by terminals. If a terminal doesn't define
       a clear capability, it will not be executed when you use ncurses
*/


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
