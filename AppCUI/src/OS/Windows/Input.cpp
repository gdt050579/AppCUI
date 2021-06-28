#include "os.h"

using namespace AppCUI::Internal;

bool Input::Init()
{
    this->hstdIn = GetStdHandle(STD_INPUT_HANDLE);
    CHECK(GetConsoleMode(this->hstdIn, &originalStdMode), false, "Fail to query input mode !");
    CHECK(SetConsoleMode(this->hstdIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT), false, "Fail to set up input reader mode !");

    // Build the key translation matrix [could be improved with a static vector]
    for (unsigned int tr = 0; tr < KEYTRANSLATION_MATRIX_SIZE; tr++)
        this->KeyTranslationMatrix[tr] = AppCUI::Input::Key::None;
    for (unsigned int tr = 0; tr < 12; tr++)
        this->KeyTranslationMatrix[VK_F1 + tr] = static_cast<AppCUI::Input::Key::Type>(AppCUI::Input::Key::F1 + tr);
    for (unsigned int tr = 'A'; tr <= 'Z'; tr++)
        this->KeyTranslationMatrix[tr] = static_cast<AppCUI::Input::Key::Type>(AppCUI::Input::Key::A + (tr-'A'));
    for (unsigned int tr = '0'; tr <= '9'; tr++)
        this->KeyTranslationMatrix[tr] = static_cast<AppCUI::Input::Key::Type>(AppCUI::Input::Key::N0 + (tr - '0'));

    this->KeyTranslationMatrix[VK_RETURN] = AppCUI::Input::Key::Enter;
    this->KeyTranslationMatrix[VK_ESCAPE] = AppCUI::Input::Key::Escape;
    this->KeyTranslationMatrix[VK_INSERT] = AppCUI::Input::Key::Insert;
    this->KeyTranslationMatrix[VK_BACK] = AppCUI::Input::Key::Backspace;
    this->KeyTranslationMatrix[VK_TAB] = AppCUI::Input::Key::Tab;
    this->KeyTranslationMatrix[VK_DELETE] = AppCUI::Input::Key::Delete;
    this->KeyTranslationMatrix[VK_LEFT] = AppCUI::Input::Key::Left;
    this->KeyTranslationMatrix[VK_UP] = AppCUI::Input::Key::Up;
    this->KeyTranslationMatrix[VK_RIGHT] = AppCUI::Input::Key::Right;
    this->KeyTranslationMatrix[VK_DOWN] = AppCUI::Input::Key::Down;
    this->KeyTranslationMatrix[VK_PRIOR] = AppCUI::Input::Key::PageUp;
    this->KeyTranslationMatrix[VK_NEXT] = AppCUI::Input::Key::PageDown;
    this->KeyTranslationMatrix[VK_HOME] = AppCUI::Input::Key::Home;
    this->KeyTranslationMatrix[VK_END] = AppCUI::Input::Key::End;
    this->KeyTranslationMatrix[VK_SPACE] = AppCUI::Input::Key::Space;    

    this->shiftState = AppCUI::Input::Key::None;

    return true;
}
void Input::Uninit()
{

}
void Input::GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt)
{
    DWORD			            nrread;
    INPUT_RECORD	            ir;
    AppCUI::Input::Key::Type    eventShiftState;

    evnt.eventType = SystemEvents::NONE;
    if ((ReadConsoleInput(this->hstdIn, &ir, 1, &nrread) == FALSE) || (nrread != 1))
        return;

    switch (ir.EventType)
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
            evnt.mouseX = ir.Event.MouseEvent.dwMousePosition.X;
            evnt.mouseY = ir.Event.MouseEvent.dwMousePosition.Y;
            
            if (ir.Event.MouseEvent.dwEventFlags == 0)
            {
                if (ir.Event.MouseEvent.dwButtonState)
                    evnt.eventType = SystemEvents::MOUSE_DOWN;
                else
                    evnt.eventType = SystemEvents::MOUSE_UP;
                return;
            }
            if (ir.Event.MouseEvent.dwEventFlags == MOUSE_MOVED)
            {
                evnt.eventType = SystemEvents::MOUSE_MOVE;
                return;
            }
            break;
        case WINDOW_BUFFER_SIZE_EVENT:
            evnt.newWidth = ir.Event.WindowBufferSizeEvent.dwSize.X;
            evnt.newHeight = ir.Event.WindowBufferSizeEvent.dwSize.Y;
            evnt.eventType = SystemEvents::APP_RESIZED;
            break;
    }

}
