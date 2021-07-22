#include "os.h"
#include <string.h>

using namespace AppCUI::Internal;

WindowsTerminal::WindowsTerminal()
{
    ConsoleBuffer = nullptr;
    ConsoleBufferCount = 0;
}
WindowsTerminal::~WindowsTerminal()
{
    if (ConsoleBuffer != nullptr)
        delete[]ConsoleBuffer;
    ConsoleBuffer = nullptr;
    ConsoleBufferCount = 0;
}
bool WindowsTerminal::ResizeConsoleBuffer(unsigned int width, unsigned int height)
{
    unsigned int newCount = ((width * height) | 0xFF) + 1;
    if (newCount <= ConsoleBufferCount)
        return true; // no need to resize
    CHAR_INFO* temp = new CHAR_INFO[newCount];
    CHECK(temp, false, "Fail to allocate: %d characters for a %dx%d sized terminal", newCount, width, height);
    if (this->ConsoleBuffer)
        delete this->ConsoleBuffer;
    this->ConsoleBuffer = temp;
    this->ConsoleBufferCount = newCount;
    return true;
}
bool WindowsTerminal::OnInit(const InitializationData & initData)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    this->hstdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    this->hstdIn = GetStdHandle(STD_INPUT_HANDLE);
    CHECK(GetConsoleMode(this->hstdIn, &originalStdMode), false, "Fail to query input mode !");
    CHECK(SetConsoleMode(this->hstdIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT), false, "Fail to set up input reader mode !");
    CHECK(GetConsoleScreenBufferInfo(this->hstdOut, &csbi), false, "Unable to read console screen buffer !");
    CHECK(csbi.dwSize.X > 0, false, "Received invalid screen width from the system --> exiting");
    CHECK(csbi.dwSize.Y > 0, false, "Received invalid screen height from the system --> exiting");

    // create canvases
    CHECK(this->ScreenCanvas.Create(csbi.dwSize.X, csbi.dwSize.Y), false, "Fail to create an internal canvas of %d x %d size", csbi.dwSize.X, csbi.dwSize.Y);
    CHECK(this->OriginalScreenCanvas.Create(csbi.dwSize.X, csbi.dwSize.Y), false, "Fail to create the original screen canvas of %d x %d size", csbi.dwSize.X, csbi.dwSize.Y);
    
    // create temporary rendering buffer
    CHECK(ResizeConsoleBuffer(csbi.dwSize.X, csbi.dwSize.Y), false, "Fail to create console buffer");
    SMALL_RECT BufRect = { 0, 0, (SHORT)(csbi.dwSize.X - 1), (SHORT)(csbi.dwSize.Y - 1) };

    // make a copy of the original screem buffer into the canvas
    CHECK(ReadConsoleOutput(this->hstdOut, this->ConsoleBuffer, csbi.dwSize, { 0,0 }, &BufRect), false, "Unable to make a copy of the initial screen buffer !");
    CHAR_INFO * p = this->ConsoleBuffer;
    for (unsigned int y = 0; y < (unsigned int)csbi.dwSize.Y; y++)
    {
        for (unsigned int x = 0; x < (unsigned int)csbi.dwSize.X; x++,p++)
        {
            this->OriginalScreenCanvas.WriteCharacter(x, y, p->Char.UnicodeChar, AppCUI::Console::ColorPair{static_cast<AppCUI::Console::Color>(p->Attributes & 0x0F),static_cast<AppCUI::Console::Color>((p->Attributes & 0xF0)>>4)});
        }
    }

    // copy current cursor position
    CONSOLE_CURSOR_INFO cInfo;
    CHECK(GetConsoleCursorInfo(this->hstdOut, &cInfo), false, "Unable to read console cursor informations !");
    if (cInfo.bVisible)
        this->OriginalScreenCanvas.HideCursor();
    else
        this->OriginalScreenCanvas.SetCursor(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);

    // Build the key translation matrix [could be improved with a static vector]
    for (unsigned int tr = 0; tr < KEYTRANSLATION_MATRIX_SIZE; tr++)
        this->KeyTranslationMatrix[tr] = AppCUI::Input::Key::None;
    for (unsigned int tr = 0; tr < 12; tr++)
        this->KeyTranslationMatrix[VK_F1 + tr] = static_cast<AppCUI::Input::Key>(((unsigned int)AppCUI::Input::Key::F1) + tr);
    for (unsigned int tr = 'A'; tr <= 'Z'; tr++)
        this->KeyTranslationMatrix[tr] = static_cast<AppCUI::Input::Key>(((unsigned int)AppCUI::Input::Key::A) + (tr - 'A'));
    for (unsigned int tr = '0'; tr <= '9'; tr++)
        this->KeyTranslationMatrix[tr] = static_cast<AppCUI::Input::Key>(((unsigned int)AppCUI::Input::Key::N0) + (tr - '0'));

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
void WindowsTerminal::OnUninit()
{

}
void WindowsTerminal::OnFlushToScreen()
{
    unsigned int w = this->ScreenCanvas.GetWidth();
    unsigned int h = this->ScreenCanvas.GetHeight();
    COORD winSize = { (SHORT)w, (SHORT)h };
    SMALL_RECT sr = { 0,0,winSize.X, winSize.Y };
    // copy the entire buffer
    AppCUI::Console::Character * c = this->ScreenCanvas.GetCharactersBuffer();
    AppCUI::Console::Character * e = c + (w*h);
    CHAR_INFO* d = this->ConsoleBuffer;
    while (c < e) {
        d->Char.UnicodeChar = c->Code;
        d->Attributes = ((unsigned char)c->Color.Forenground) | (((unsigned char)(c->Color.Background)) << 4);
        d++;
        c++;
    }    
    WriteConsoleOutputW(this->hstdOut, this->ConsoleBuffer, winSize, { 0,0 }, &sr);
}
bool WindowsTerminal::OnUpdateCursor()
{
    if (this->ScreenCanvas.GetCursorVisibility())
    {
        COORD c = { (SHORT) this->ScreenCanvas.GetCursorX(), (SHORT)this->ScreenCanvas.GetCursorY() };
        CHECK(SetConsoleCursorPosition(this->hstdOut, c), false, "SetConsoleCursorPosition failed !");
        CONSOLE_CURSOR_INFO	ci = { 10,TRUE };
        CHECK(SetConsoleCursorInfo(this->hstdOut, &ci), false, "SetConsoleCursorInfo failed !");
    }
    else {
        CONSOLE_CURSOR_INFO	ci = { 10,FALSE };
        CHECK(SetConsoleCursorInfo(this->hstdOut, &ci), false, "SetConsoleCursorInfo failed !");
    }
    return true;
}
void WindowsTerminal::GetSystemEvent(AppCUI::Internal::SystemEvents::Event & evnt)
{
    DWORD			            nrread;
    INPUT_RECORD	            ir;
    AppCUI::Input::Key    eventShiftState;

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
        if ((((unsigned int)eventShiftState) & ((unsigned int)(AppCUI::Input::Key::Alt | AppCUI::Input::Key::Ctrl))) != 0)
            evnt.asciiCode = 0;

        if (evnt.keyCode == AppCUI::Input::Key::None)
        {
            if (eventShiftState != this->shiftState)
                evnt.eventType = SystemEvents::SHIFT_STATE_CHANGED;
            else if ((evnt.asciiCode > 0) && (ir.Event.KeyEvent.bKeyDown))
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

bool WindowsTerminal::IsEventAvailable()
{
    DWORD			nrread;
    INPUT_RECORD	ir;

    if (PeekConsoleInput(this->hstdIn, &ir, 1, &nrread) == FALSE)
        return false;
    return (nrread > 0);
}
