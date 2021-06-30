#include "os.h"

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
void Input::GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt)
{
    /*   switch (ir.EventType)
    {
        case KEY_EVENT:
            if ((ir.Event.KeyEvent.uChar.AsciiChar >= 32) && (ir.Event.KeyEvent.uChar.AsciiChar <= 127) && (ir.Event.KeyEvent.bKeyDown))
                evnt.asciiCode = ir.Event.KeyEvent.uChar.AsciiChar;
            else
                evnt.asciiCode = 0;                  
            if (ir.Event.KeyEvent.wVirtualKeyCode < KEYTRANSLATION_MATRIX_SIZE)
                evnt.keyCode = KeyTranslationMatrix[ir.Event.KeyEvent.wVirtualKeyCode];
            else
                evnt.keyCode = AppCUI::Input::Key::None;

            eventShiftState = AppCUI::Input::Key::None;
            if ((ir.Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0)
                eventShiftState |= AppCUI::Input::Key::Alt;
            if ((ir.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) != 0)
                eventShiftState |= AppCUI::Input::Key::Shift;
            if ((ir.Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0)
                eventShiftState |= AppCUI::Input::Key::Ctrl;

            // if ALT or CTRL are pressed, clear the ascii code
            if ((eventShiftState & (AppCUI::Input::Key::Alt | AppCUI::Input::Key::Ctrl)) != 0)
                evnt.asciiCode = 0;

            if (evnt.keyCode == AppCUI::Input::Key::None)
            {
                if (eventShiftState != this->shiftState)
                    evnt.eventType = SystemEvents::SHIFT_STATE_CHANGED;
                else if ((evnt.asciiCode>0) && (ir.Event.KeyEvent.bKeyDown))
                    evnt.eventType = SystemEvents::KEY_PRESSED;
                evnt.keyCode = eventShiftState;
            }
            else {
                evnt.keyCode |= eventShiftState;
                if (ir.Event.KeyEvent.bKeyDown)
                    evnt.eventType = SystemEvents::KEY_PRESSED;
            }
            this->shiftState = eventShiftState;
            break;
        case MOUSE_EVENT:

        case WINDOW_BUFFER_SIZE_EVENT:
            evnt.newWidth = ir.Event.WindowBufferSizeEvent.dwSize.X;
            evnt.newHeight = ir.Event.WindowBufferSizeEvent.dwSize.Y;
            evnt.eventType = SystemEvents::APP_RESIZED;
            break;
    }
    */
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
            const auto& state = mouse_event.bstate;

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
}
