#include "../../include/Internal.h"
#include <string.h>

using namespace AppCUI::Internal;
int _special_characters_consolas_font[AppCUI::Console::SpecialChars::Count] = {
       0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551, // double line box
       0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502, // single line box
       0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194, // arrows
};

bool ConsoleRenderer::Init()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    this->hstdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    CHECK(GetConsoleScreenBufferInfo(this->hstdOut, &csbi), false, "Unable to read console screen buffer !");
    CHECK(csbi.dwSize.X > 0, false, "Received invalid screen width from the system --> exiting");
    CHECK(csbi.dwSize.Y > 0, false, "Received invalid screen height from the system --> exiting");
    this->BeforeInitConfig.consoleSize.Set(csbi.dwSize.X, csbi.dwSize.Y);
    this->BeforeInitConfig.screenBuffer = new CHAR_INFO[BeforeInitConfig.consoleSize.Width * BeforeInitConfig.consoleSize.Height];
    CHECK(this->BeforeInitConfig.screenBuffer, false, "Fail to allocate space for initial screen of size %d x %d", BeforeInitConfig.consoleSize.Width, BeforeInitConfig.consoleSize.Height);
    SMALL_RECT BufRect = { 0, 0, (SHORT)(csbi.dwSize.X - 1), (SHORT)(csbi.dwSize.Y - 1) };    
    CHECK(ReadConsoleOutput(this->hstdOut, this->BeforeInitConfig.screenBuffer, csbi.dwSize, { 0,0 }, &BufRect), false, "Unable to make a copy of the initial screen buffer !");
    CHECK(this->CreateScreenBuffers(csbi.dwSize.X, csbi.dwSize.Y), false, "Fail to create screen buffers");

    // set up special characters (unicode codes)
    SpecialCharacters = _special_characters_consolas_font;
    

    //this->OriginalCursorX = csbi.dwCursorPosition.X;
    //this->OriginalCursorY = csbi.dwCursorPosition.Y;
    //this->MaxWidthValue = this->Width - 1;
    //this->MaxHeightValue = this->Height - 1;
    //// get original cursor position
    //CONSOLE_CURSOR_INFO cInfo;
    //CHECK(GetConsoleCursorInfo(this->hstdOut, &cInfo), false, "Unable to read console cursor informations !");
    //this->OriginalCursorIsVisible = cInfo.bVisible;

    //// initializez si bufferul de lucru

    //memcpy(this->Buf, this->OriginalBuf, sizeof(CHAR_INFO)*this->OriginalWidth*this->OriginalHeight);
    //memcpy(this->DoubleBuf, this->OriginalBuf, sizeof(CHAR_INFO)*this->OriginalWidth*this->OriginalHeight);
    //// asignari de taste

    this->TranslateX = this->TranslateY = 0;

    return true;
}
void ConsoleRenderer::FlushToScreen()
{
    COORD winSize = { (SHORT)this->ConsoleSize.Width, (SHORT)this->ConsoleSize.Height };
    SMALL_RECT sr = { 0,0,(SHORT)this->ConsoleSize.Width, (SHORT)this->ConsoleSize.Height };
    WriteConsoleOutputW(this->hstdOut, this->WorkingBuffer, winSize, { 0,0 }, &sr);
}
bool InputReader::Init()
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
void InputReader::GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt)
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

            if (eventShiftState != AppCUI::Input::Key::None)
                evnt.keyCode |= eventShiftState;

            // if ALT or CTRL are pressed, clear the ascii code
            if ((evnt.keyCode & (AppCUI::Input::Key::Alt | AppCUI::Input::Key::Ctrl)) != 0)
                evnt.asciiCode = 0;

            if ((ir.Event.KeyEvent.bKeyDown) || (eventShiftState != this->shiftState))
                evnt.eventType = SystemEvents::KEY_PRESSED;
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
    }

}
