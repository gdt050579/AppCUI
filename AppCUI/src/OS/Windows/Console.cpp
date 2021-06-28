#include "os.h"
#include <string.h>

using namespace AppCUI::Internal;
int _special_characters_consolas_font[AppCUI::Console::SpecialChars::Count] = {
       0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551,                      // double line box
       0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502,                      // single line box
       0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                      // arrows
       32, 0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590,  // blocks
       0x25CF, 0x25CB, 0x221A,                                              // symbols
};

bool Console::OnInit()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    this->OSSpecific.hstdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    CHECK(GetConsoleScreenBufferInfo(this->OSSpecific.hstdOut, &csbi), false, "Unable to read console screen buffer !");
    CHECK(csbi.dwSize.X > 0, false, "Received invalid screen width from the system --> exiting");
    CHECK(csbi.dwSize.Y > 0, false, "Received invalid screen height from the system --> exiting");
    this->BeforeInitConfig.consoleSize.Set(csbi.dwSize.X, csbi.dwSize.Y);
    this->BeforeInitConfig.screenBuffer = new CHAR_INFO[BeforeInitConfig.consoleSize.Width * BeforeInitConfig.consoleSize.Height];
    CHECK(this->BeforeInitConfig.screenBuffer, false, "Fail to allocate space for initial screen of size %d x %d", BeforeInitConfig.consoleSize.Width, BeforeInitConfig.consoleSize.Height);
    SMALL_RECT BufRect = { 0, 0, (SHORT)(csbi.dwSize.X - 1), (SHORT)(csbi.dwSize.Y - 1) };    
    CHECK(ReadConsoleOutput(this->OSSpecific.hstdOut, this->BeforeInitConfig.screenBuffer, csbi.dwSize, { 0,0 }, &BufRect), false, "Unable to make a copy of the initial screen buffer !");
    CHECK(this->CreateScreenBuffers(csbi.dwSize.X, csbi.dwSize.Y), false, "Fail to create screen buffers");

    CONSOLE_CURSOR_INFO cInfo;
    CHECK(GetConsoleCursorInfo(this->OSSpecific.hstdOut, &cInfo), false, "Unable to read console cursor informations !");
    this->BeforeInitConfig.CursorVisible = cInfo.bVisible;
    this->BeforeInitConfig.CursorX = csbi.dwCursorPosition.X;
    this->BeforeInitConfig.CursorY = csbi.dwCursorPosition.Y;

    SpecialCharacters = _special_characters_consolas_font;
    this->TranslateX = this->TranslateY = 0;

    return true;
}
void Console::OnUninit()
{

}
void Console::OnFlushToScreen()
{
    COORD winSize = { (SHORT)this->ConsoleSize.Width, (SHORT)this->ConsoleSize.Height };
    SMALL_RECT sr = { 0,0,(SHORT)this->ConsoleSize.Width, (SHORT)this->ConsoleSize.Height };
    WriteConsoleOutputW(this->OSSpecific.hstdOut, this->WorkingBuffer, winSize, { 0,0 }, &sr);
}
bool Console::OnUpdateCursor()
{
    if (Cursor.Visible)
    {
        COORD c = { (SHORT)Cursor.X, (SHORT)Cursor.Y };
        CHECK(SetConsoleCursorPosition(this->OSSpecific.hstdOut, c), false, "SetConsoleCursorPosition failed !");
        CONSOLE_CURSOR_INFO	ci = { 10,TRUE };
        CHECK(SetConsoleCursorInfo(this->OSSpecific.hstdOut, &ci), false, "SetConsoleCursorInfo failed !");
    }
    else {
        CONSOLE_CURSOR_INFO	ci = { 10,FALSE };
        CHECK(SetConsoleCursorInfo(this->OSSpecific.hstdOut, &ci), false, "SetConsoleCursorInfo failed !");
    }
    return true;
}
