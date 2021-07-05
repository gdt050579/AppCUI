#include "os.h"
#include <string.h>

using namespace AppCUI::Internal;


Console::~Console()
{

}
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
