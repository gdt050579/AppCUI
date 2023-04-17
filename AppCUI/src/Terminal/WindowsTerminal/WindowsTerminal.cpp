#include "WindowsTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;
using namespace Graphics;

WindowsTerminal::WindowsTerminal()
    : consoleBufferCount(0), fpsMode(false), lastMousePosition({ 0xFFFFFFFFu, 0xFFFFFFFFu })
{
}

WindowsTerminal::~WindowsTerminal()
{
    consoleBufferCount = 0;
}

bool WindowsTerminal::ResizeConsoleBuffer(uint32 width, uint32 height)
{
    const uint32 newCount = ((width * height) | 0xFF) + 1;
    CHECK(newCount > consoleBufferCount, true, "No need to resize!");

    consoleBuffer.reset(new CHAR_INFO[newCount]);
    CHECK(consoleBuffer.get(),
          false,
          "Fail to allocate: %d characters for a %dx%d sized terminal",
          newCount,
          width,
          height);

    consoleBufferCount = newCount;

    return true;
}

bool WindowsTerminal::CopyOriginalScreenBuffer(uint32 width, uint32 height, uint32 mouseX, uint32 mouseY)
{
    CHECK(this->originalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          width,
          height);
    SMALL_RECT BufRect = { 0, 0, (SHORT) (width - 1), (SHORT) (height - 1) };

    // allocate memory for a console buffer
    std::unique_ptr<CHAR_INFO[]> temp(new CHAR_INFO[(size_t) width * (size_t) height]);
    CHECK(temp.get(), false, "Fail to allocate %dx%d buffer to store the original screen buffer !", width, height);

    // make a copy of the original screem buffer into the canvas
    while (true)
    {
        CHECKBK(
              ReadConsoleOutput(this->hstdOut, temp.get(), { (SHORT) width, (SHORT) height }, { 0, 0 }, &BufRect),
              "Unable to make a copy of the initial screen buffer !");

        CHAR_INFO* p = temp.get();
        for (uint32 y = 0; y < (uint32) height; y++)
        {
            for (uint32 x = 0; x < (uint32) width; x++, p++)
            {
                this->originalScreenCanvas.WriteCharacter(
                      x,
                      y,
                      p->Char.UnicodeChar,
                      Graphics::ColorPair{ static_cast<Graphics::Color>(p->Attributes & 0x0F),
                                           static_cast<Graphics::Color>((p->Attributes & 0xF0) >> 4) });
            }
        }

        // copy current cursor position
        CONSOLE_CURSOR_INFO cInfo;
        CHECKBK(GetConsoleCursorInfo(this->hstdOut, &cInfo), "Unable to read console cursor informations !");
        if (cInfo.bVisible)
        {
            this->originalScreenCanvas.HideCursor();
        }
        else
        {
            this->originalScreenCanvas.SetCursor(mouseX, mouseY);
        }
        return true;
    }

    return false;
}

bool WindowsTerminal::ResizeConsoleScreenBufferSize(uint32 width, uint32 height)
{
    COORD coord = { 0, 0 };
    coord.X     = (SHORT) width;
    coord.Y     = (SHORT) height;

    CHECK(SetConsoleScreenBufferSize(this->hstdOut, coord),
          false,
          "Fail to resize the console buffer to a %ux%u size (SetConsoleScreenBufferSize has the followin error "
          "code: %d)",
          width,
          height,
          GetLastError());

    return true;
}

bool WindowsTerminal::ResizeConsoleWindowSize(uint32 width, uint32 height)
{
    SMALL_RECT rect;

    rect.Left   = 0;
    rect.Top    = 0;
    rect.Right  = (SHORT) (width - 1);
    rect.Bottom = (SHORT) (height - 1);

    CHECK(SetConsoleWindowInfo(this->hstdOut, TRUE, &rect),
          false,
          "Fail to resize the window to a %ux%u size (SetConsoleScreenBufferSize has the followin error code: %d)",
          width,
          height,
          GetLastError());

    return true;
}

Size WindowsTerminal::MaximizeTerminal()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    CHECK(::ShowWindow(GetConsoleWindow(), SW_MAXIMIZE), Size(), "Fail to maximize the console !");
    CHECK(GetConsoleScreenBufferInfo(this->hstdOut, &csbi), Size(), "Unable to read console screen buffer !");
    return Size(csbi.dwSize.X, csbi.dwSize.Y);
}

Size WindowsTerminal::FullScreenTerminal()
{
    COORD coord = { 0, 0 };
    CHECK(SetConsoleDisplayMode(this->hstdOut, CONSOLE_FULLSCREEN_MODE, &coord),
          Size(),
          "Fail to maximize the console ==> Error code: 0x%08X",
          GetLastError());
    CHECK(SetConsoleMode(this->hstdIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT),
          Size(),
          "Fail to set up input reader mode !");
    return Size(coord.X, coord.Y);
}

Size WindowsTerminal::ResizeTerminal(const InitializationData& initData, const Size& currentSize)
{
    CHECK(initData.Width < 0xFFFF,
          Size(),
          "initData.Width has be smaller than 0xFFFF --> curently is: %u",
          initData.Width);
    CHECK(initData.Height < 0xFFFF,
          Size(),
          "initData.Height has be smaller than 0xFFFF --> curently is: %u",
          initData.Height);

    if ((currentSize.Width == initData.Width) && (currentSize.Height == initData.Height))
        return currentSize;

    CONSOLE_SCREEN_BUFFER_INFOEX csbi;
    CONSOLE_FONT_INFOEX fnt;

    // check if size is not smaller than the maximum allowed
    fnt.cbSize = sizeof(fnt);
    CHECK(GetCurrentConsoleFontEx(this->hstdOut, FALSE, &fnt), Size(), "Fail to get Font information !");
    CHECK(fnt.dwFontSize.X > 0, Size(), "GetCurrentConsoleFontEx(...) returned an invalid font character width !");
    CHECK(fnt.dwFontSize.Y > 0, Size(), "GetCurrentConsoleFontEx(...) returned an invalid font character height !");
    int minWidth  = GetSystemMetrics(SM_CXMIN) / fnt.dwFontSize.X;
    int minHeight = GetSystemMetrics(SM_CYMIN) / fnt.dwFontSize.Y;
    LOG_INFO("Minim size for a console: %dx%d", minWidth, minHeight);
    CHECK((int) initData.Width >= minWidth,
          Size(),
          "Minimal width for a console is %d (requested is %u)",
          minWidth,
          initData.Width);
    CHECK((int) initData.Height >= minHeight,
          Size(),
          "Minimal height for a console is %d (requested is %u)",
          minHeight,
          initData.Height);

    // first resize the window to a smaller value
    CHECK(ResizeConsoleWindowSize(1, 1), Size(), "");
    CHECK(ResizeConsoleScreenBufferSize(initData.Width, initData.Height), Size(), "");
    CHECK(ResizeConsoleWindowSize(initData.Width, initData.Height), Size(), "");

    csbi.cbSize = sizeof(csbi);
    CHECK(GetConsoleScreenBufferInfoEx(this->hstdOut, &csbi),
          Size(),
          "Unable to get console screen buffer info (LastError = 0x%08X)",
          GetLastError());
    CHECK(csbi.dwSize.X == initData.Width,
          Size(),
          "Console screen buffer width has not changed (curently is %u, expected value is %u)",
          csbi.dwSize.X,
          initData.Width);
    CHECK((csbi.srWindow.Right + 1 - csbi.srWindow.Left) == initData.Width,
          Size(),
          "Console window width has not changed (curently is %u, expected value is %u)",
          (csbi.srWindow.Right + 1 - csbi.srWindow.Left),
          initData.Width);
    CHECK(csbi.dwSize.Y == initData.Height,
          Size(),
          "Console screen buffer width has not changed (curently is %u, expected value is %u)",
          csbi.dwSize.Y,
          initData.Height);
    CHECK((csbi.srWindow.Bottom + 1 - csbi.srWindow.Top) == initData.Height,
          Size(),
          "Console window width has not changed (curently is %u, expected value is %u)",
          (csbi.srWindow.Bottom + 1 - csbi.srWindow.Top),
          initData.Height);

    return Size(initData.Width, initData.Height);

    // for SetConsoleScreenBufferSize:
    /*
     * The specified width and height cannot be less than the width and height of the console screen buffer's window.
     * https://docs.microsoft.com/en-us/windows/console/setconsolescreenbuffersize
     */

    // for SetConsoleWindowInfo
    /*
     * The function fails if the specified window rectangle extends beyond the boundaries of the console screen buffer.
     * https://docs.microsoft.com/en-us/windows/console/setconsolewindowinfo
     */

    /*
    // if new width is larged then the old width, first resize screen buffer, than window size
    // if new width is smaller then the old width, first resize window size, than screen buffer
    // The height will be kept as it is during this resize
    if (initData.Width > currentSize.Width)
    {
        CHECK(ResizeConsoleScreenBufferSize(initData.Width, currentSize.Height), Size(), "");
        CHECK(ResizeConsoleWindowSize(initData.Width, currentSize.Height), Size(), "");
    }
    if (initData.Width < currentSize.Width)
    {
        CHECK(ResizeConsoleWindowSize(initData.Width, currentSize.Height), Size(), "");
        CHECK(ResizeConsoleScreenBufferSize(initData.Width, currentSize.Height), Size(), "");
    }
    // sanity check --> validate if the width has changed
    csbi.cbSize = sizeof(csbi);
    CHECK(GetConsoleScreenBufferInfoEx(this->hstdOut, &csbi),
          Size(),
          "Unable to get console screen buffer info (LastError = 0x%08X)",
          GetLastError());
    CHECK(csbi.dwSize.X == initData.Width,
          Size(),
          "Console screen buffer width has not changed (curently is %u, expected value is %u)",
          csbi.dwSize.X,
          initData.Width);
    CHECK((csbi.srWindow.Right + 1 - csbi.srWindow.Left) == initData.Width,
          Size(),
          "Console window width has not changed (curently is %u, expected value is %u)",
          (csbi.srWindow.Right + 1 - csbi.srWindow.Left),
          initData.Width);

    // now resize the height
    if (initData.Height > currentSize.Height)
    {
        CHECK(ResizeConsoleScreenBufferSize(initData.Width, initData.Height), Size(), "");
        CHECK(ResizeConsoleWindowSize(initData.Width, initData.Height), Size(), "");
    }
    if (initData.Height < currentSize.Height)
    {
        CHECK(ResizeConsoleWindowSize(initData.Width, initData.Height), Size(), "");
        CHECK(ResizeConsoleScreenBufferSize(initData.Width, initData.Height), Size(), "");
    }
    // sanity check --> validate if the height has changed
    csbi.cbSize = sizeof(csbi);
    CHECK(GetConsoleScreenBufferInfoEx(this->hstdOut, &csbi),
          Size(),
          "Unable to get console screen buffer info (LastError = 0x%08X)",
          GetLastError());
    CHECK(csbi.dwSize.Y == initData.Height,
          Size(),
          "Console screen buffer width has not changed (curently is %u, expected value is %u)",
          csbi.dwSize.Y,
          initData.Height);
    CHECK((csbi.srWindow.Bottom + 1 - csbi.srWindow.Top) == initData.Height,
          Size(),
          "Console window width has not changed (curently is %u, expected value is %u)",
          (csbi.srWindow.Bottom + 1 - csbi.srWindow.Top),
          initData.Height);

    return Size(initData.Width, initData.Height);
    //*/
}

Size WindowsTerminal::UpdateTerminalSize(const InitializationData& initData, const Size& currentSize)
{
    Size resultedSize;

    if ((initData.Flags & InitializationFlags::Maximized) != InitializationFlags::None)
        resultedSize = MaximizeTerminal();
    else if ((initData.Flags & InitializationFlags::Fullscreen) != InitializationFlags::None)
        resultedSize = FullScreenTerminal();
    else if ((initData.Width == 0) && (initData.Height == 0))
        resultedSize = currentSize; // default values (keep the existing settings)
    else
        resultedSize = ResizeTerminal(initData, currentSize); // custom size

    // sanity check
    CHECK(resultedSize.Width > 0, Size(), "Something went wrong with windows API ==> resulted width is 0 !");
    CHECK(resultedSize.Height > 0, Size(), "Something went wrong with windows API ==> resulted height is 0 !");

    if ((initData.Flags & InitializationFlags::FixedSize) != InitializationFlags::None)
    {
        // make sure that current window is unmoveable
    }

    return resultedSize;
}

bool WindowsTerminal::ComputeCharacterSize(const Application::InitializationData& initData)
{
    if (initData.CharSize == CharacterSize::Default)
        return true; // leave the settings as they are
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize           = sizeof(CONSOLE_FONT_INFOEX);
    cfi.FontWeight       = FW_NORMAL;
    cfi.dwFontSize.X     = 0;
    cfi.nFont            = 0;
    cfi.FontFamily       = FF_DONTCARE;
    string_view fontName = initData.FontName;
    if (fontName.size() == 0)
        fontName = "Consolas"; // default font name
    CHECK(fontName.size() < 32, false, "Invalid font name (should be less than 32 characters) !");
    // copy the font name
    for (size_t i = 0; i < fontName.size(); i++)
        cfi.FaceName[i] = fontName[i];
    cfi.FaceName[fontName.size()] = 0;

    switch (initData.CharSize)
    {
    case CharacterSize::Tiny:
        cfi.dwFontSize.Y = 8;
        break;
    case CharacterSize::Small:
        cfi.dwFontSize.Y = 14;
        break;
    case CharacterSize::Normal:
        cfi.dwFontSize.Y = 18;
        break;
    case CharacterSize::Large:
        cfi.dwFontSize.Y = 24;
        break;
    case CharacterSize::Huge:
        cfi.dwFontSize.Y = 36;
        break;
    default:
        RETURNERROR(false, "Invalid/unknwon value for character size: %d", (uint32) initData.CharSize);
    }
    CHECK(SetCurrentConsoleFontEx(this->hstdOut, FALSE, &cfi),
          false,
          "Fail to set character size (Error = 0x%08X)",
          GetLastError());
    return true;
}

void WindowsTerminal::BuildKeyTranslationMatrix()
{
    // Build the key translation matrix [could be improved with a static vector]
    for (uint32 tr = 0; tr < KEYTRANSLATION_MATRIX_SIZE; tr++)
        this->KeyTranslationMatrix[tr] = Input::Key::None;
    for (uint32 tr = 0; tr < 12; tr++)
        this->KeyTranslationMatrix[VK_F1 + tr] = static_cast<Input::Key>(((uint32) Input::Key::F1) + tr);
    for (uint32 tr = 'A'; tr <= 'Z'; tr++)
        this->KeyTranslationMatrix[tr] = static_cast<Input::Key>(((uint32) Input::Key::A) + (tr - 'A'));
    for (uint32 tr = '0'; tr <= '9'; tr++)
        this->KeyTranslationMatrix[tr] = static_cast<Input::Key>(((uint32) Input::Key::N0) + (tr - '0'));

    this->KeyTranslationMatrix[VK_RETURN] = Input::Key::Enter;
    this->KeyTranslationMatrix[VK_ESCAPE] = Input::Key::Escape;
    this->KeyTranslationMatrix[VK_INSERT] = Input::Key::Insert;
    this->KeyTranslationMatrix[VK_BACK]   = Input::Key::Backspace;
    this->KeyTranslationMatrix[VK_TAB]    = Input::Key::Tab;
    this->KeyTranslationMatrix[VK_DELETE] = Input::Key::Delete;
    this->KeyTranslationMatrix[VK_LEFT]   = Input::Key::Left;
    this->KeyTranslationMatrix[VK_UP]     = Input::Key::Up;
    this->KeyTranslationMatrix[VK_RIGHT]  = Input::Key::Right;
    this->KeyTranslationMatrix[VK_DOWN]   = Input::Key::Down;
    this->KeyTranslationMatrix[VK_PRIOR]  = Input::Key::PageUp;
    this->KeyTranslationMatrix[VK_NEXT]   = Input::Key::PageDown;
    this->KeyTranslationMatrix[VK_HOME]   = Input::Key::Home;
    this->KeyTranslationMatrix[VK_END]    = Input::Key::End;
    this->KeyTranslationMatrix[VK_SPACE]  = Input::Key::Space;

    this->shiftState = Input::Key::None;
}

bool WindowsTerminal::OnInit(const Application::InitializationData& initData)
{
    CONSOLE_SCREEN_BUFFER_INFOEX csbi;

    this->hstdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    this->hstdIn  = GetStdHandle(STD_INPUT_HANDLE);
    CHECK(GetConsoleMode(this->hstdIn, &originalStdMode), false, "Fail to query input mode !");
    CHECK(SetConsoleMode(this->hstdIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS),
          false,
          "Fail to set up input reader mode !");
    csbi.cbSize = sizeof(csbi);
    CHECK(GetConsoleScreenBufferInfoEx(this->hstdOut, &csbi), false, "Unable to read console screen buffer !");
    CHECK(csbi.dwSize.X > 0, false, "Received invalid screen width from the system --> exiting");
    CHECK(csbi.dwSize.Y > 0, false, "Received invalid screen height from the system --> exiting");

    // set the colors
    csbi.ColorTable[0]  = 0x000000;
    csbi.ColorTable[1]  = 0x800000;
    csbi.ColorTable[2]  = 0x008000;
    csbi.ColorTable[3]  = 0x808000;
    csbi.ColorTable[4]  = 0x000080;
    csbi.ColorTable[5]  = 0x800080;
    csbi.ColorTable[6]  = 0x008080;
    csbi.ColorTable[7]  = 0xC0C0C0;
    csbi.ColorTable[8]  = 0x808080;
    csbi.ColorTable[9]  = 0xFF0000;
    csbi.ColorTable[10] = 0x00FF00;
    csbi.ColorTable[11] = 0xFFFF00;
    csbi.ColorTable[12] = 0x0000FF;
    csbi.ColorTable[13] = 0xFF00FF;
    csbi.ColorTable[14] = 0x00FFFF;
    csbi.ColorTable[15] = 0xFFFFFF;

    // recompute window size
    csbi.srWindow.Left   = 0;
    csbi.srWindow.Top    = 0;
    csbi.srWindow.Right  = csbi.dwSize.X;
    csbi.srWindow.Bottom = csbi.dwSize.Y;

    auto res = SetConsoleScreenBufferInfoEx(this->hstdOut, &csbi);

    // copy original screen buffer information
    CHECK(CopyOriginalScreenBuffer(csbi.dwSize.X, csbi.dwSize.Y, csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y),
          false,
          "Fail to copy original screen buffer");

    // update character size
    CHECK(ComputeCharacterSize(initData), false, "Fail to change character size");

    // computer terminal size
    Size terminalSize = UpdateTerminalSize(initData, Size(csbi.dwSize.X, csbi.dwSize.Y));
    CHECK((terminalSize.Width > 0) && (terminalSize.Height > 0), false, "Fail to update terminal size !");

    // create canvases
    CHECK(screenCanvas.Create(terminalSize.Width, terminalSize.Height),
          false,
          "Fail to create an internal canvas of %u x %u size",
          terminalSize.Width,
          terminalSize.Height);

    // create temporary rendering buffer
    CHECK(ResizeConsoleBuffer(terminalSize.Width, terminalSize.Height), false, "Fail to create console buffer");

    // build the key translation matrix
    BuildKeyTranslationMatrix();

    // fpsMode
    this->fpsMode   = ((initData.Flags & InitializationFlags::EnableFPSMode) != InitializationFlags::None);
    this->startTime = GetTickCount();
    return true;
}

void WindowsTerminal::RestoreOriginalConsoleSettings()
{
}

void WindowsTerminal::OnUnInit()
{
}

void WindowsTerminal::OnFlushToScreen()
{
    uint32 w      = screenCanvas.GetWidth();
    uint32 h      = screenCanvas.GetHeight();
    COORD winSize = { (SHORT) w, (SHORT) h };
    SMALL_RECT sr = { 0, 0, winSize.X, winSize.Y };
    // copy the entire buffer
    // LOG_INFO("Flushing a buffer of size: %dx%d = %d chars, allocated = %d ",w,h,w*h,this->ConsoleBufferCount)
    Graphics::Character* c = screenCanvas.GetCharactersBuffer();
    Graphics::Character* e = c + ((size_t) w * (size_t) h);
    CHAR_INFO* d           = consoleBuffer.get();
    while (c < e)
    {
        d->Char.UnicodeChar = c->Code;
        d->Attributes       = ((uint8) c->Color.Foreground) | (((uint8) (c->Color.Background)) << 4);
        d++;
        c++;
    }
    WriteConsoleOutputW(hstdOut, consoleBuffer.get(), winSize, { 0, 0 }, &sr);
}

void WindowsTerminal::OnFlushToScreen(const Graphics::Rect& rect)
{
    const auto screenWidth = screenCanvas.GetWidth();
    const int32 l          = std::max<>(0, rect.GetLeft());
    const int32 r          = std::min<>((int32) screenWidth - 1, rect.GetRight());
    const int32 t          = std::max<>(0, rect.GetTop());
    const int32 b          = std::min<>((int32) screenCanvas.GetHeight() - 1, rect.GetBottom());
    const auto szW         = (r + 1) - l;

    if ((l > r) || (t > b))
        return;

    CHAR_INFO* d               = consoleBuffer.get();
    Graphics::Character* start = screenCanvas.GetCharactersBuffer() + screenWidth * t + l;
    auto y                     = t;
    while (y <= b)
    {
        Graphics::Character* c = start;
        Graphics::Character* e = c + szW;
        while (c < e)
        {
            d->Char.UnicodeChar = c->Code;
            d->Attributes       = ((uint8) c->Color.Foreground) | (((uint8) (c->Color.Background)) << 4);
            d++;
            c++;
        }
        y++;
        start += screenWidth;
    }

    COORD winSize = { (SHORT) ((r + 1) - l), (SHORT) ((b + 1) - t) };
    SMALL_RECT sr = { (SHORT) l, (SHORT) t, (SHORT) r, (SHORT) b };
    WriteConsoleOutputW(hstdOut, consoleBuffer.get(), winSize, { 0, 0 }, &sr);
}

bool WindowsTerminal::OnUpdateCursor()
{
    if (screenCanvas.GetCursorVisibility())
    {
        const COORD c = { (SHORT) screenCanvas.GetCursorX(), (SHORT) screenCanvas.GetCursorY() };
        CHECK(SetConsoleCursorPosition(this->hstdOut, c), false, "SetConsoleCursorPosition failed !");
        const static CONSOLE_CURSOR_INFO ci = { 10, TRUE };
        CHECK(SetConsoleCursorInfo(this->hstdOut, &ci), false, "SetConsoleCursorInfo failed !");
    }
    else
    {
        const static CONSOLE_CURSOR_INFO ci = { 10, FALSE };
        CHECK(SetConsoleCursorInfo(this->hstdOut, &ci), false, "SetConsoleCursorInfo failed !");
    }
    return true;
}

void WindowsTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    evnt.eventType    = SystemEventType::None;
    evnt.updateFrames = false;

    INPUT_RECORD ir;
    DWORD nrread;
    if (this->fpsMode)
    {
        DWORD64 cTime = GetTickCount64();
        DWORD diff    = 33;
        if (cTime >= this->startTime)
        {
            diff -= (cTime - this->startTime);
            if (diff > 33)
                diff = 0;
        }
        else
        {
            diff = 0;
        }
        if (diff == 0)
        {
            this->startTime = cTime;
            // a new framerate request should be send
            evnt.updateFrames = true;
            if ((PeekConsoleInput(this->hstdIn, &ir, 1, &nrread) == FALSE) || (nrread != 1))
                return; // no event - only process the framerate update
            // there is one event - read-it
            if ((ReadConsoleInputW(this->hstdIn, &ir, 1, &nrread) == FALSE) || (nrread != 1))
                return; // some error happened, process the framerate update
        }
        else
        {
            // wait until a framerate will be required
            DWORD result = WaitForSingleObject(this->hstdIn, diff);
            if (result == WAIT_OBJECT_0)
            {
                // there is an event available
                if ((ReadConsoleInputW(this->hstdIn, &ir, 1, &nrread) == FALSE) || (nrread != 1))
                    return; // some error happened, process the framerate update
            }
            else
            {
                if (result == WAIT_TIMEOUT)
                {
                    this->startTime   = GetTickCount64();
                    evnt.updateFrames = true;
                    return; // process framerate update;
                }
                else
                {
                    // other error
                    return;
                }
            }
        }
    }
    else
    {
        if ((ReadConsoleInputW(this->hstdIn, &ir, 1, &nrread) == FALSE) || (nrread != 1))
            return;
    }

    switch (ir.EventType)
    {
    case KEY_EVENT:
    {
        if ((ir.Event.KeyEvent.uChar.UnicodeChar >= 32) && (ir.Event.KeyEvent.bKeyDown))
            evnt.unicodeCharacter = ir.Event.KeyEvent.uChar.UnicodeChar;
        else
            evnt.unicodeCharacter = 0;
        if (ir.Event.KeyEvent.wVirtualKeyCode < KEYTRANSLATION_MATRIX_SIZE)
            evnt.keyCode = KeyTranslationMatrix[ir.Event.KeyEvent.wVirtualKeyCode];
        else
            evnt.keyCode = Input::Key::None;

        auto eventShiftState = Input::Key::None;
        if ((ir.Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0)
            eventShiftState |= Input::Key::Alt;
        if ((ir.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) != 0)
            eventShiftState |= Input::Key::Shift;
        if ((ir.Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0)
            eventShiftState |= Input::Key::Ctrl;

        // if ALT or CTRL are pressed, clear the ascii code
        if ((((uint32) eventShiftState) & ((uint32) (Input::Key::Alt | Input::Key::Ctrl))) != 0)
            evnt.unicodeCharacter = 0;

        if (evnt.keyCode == Input::Key::None)
        {
            if (eventShiftState != this->shiftState)
                evnt.eventType = SystemEventType::ShiftStateChanged;
            else if ((evnt.unicodeCharacter > 0) && (ir.Event.KeyEvent.bKeyDown))
                evnt.eventType = SystemEventType::KeyPressed;
            evnt.keyCode = eventShiftState;
        }
        else
        {
            evnt.keyCode |= eventShiftState;
            if (ir.Event.KeyEvent.bKeyDown)
                evnt.eventType = SystemEventType::KeyPressed;
        }
        this->shiftState = eventShiftState;
    }
    break;
    case MOUSE_EVENT:

        // for Windows 11
        if (ir.Event.MouseEvent.dwEventFlags == 0x01)
        {
            if (ir.Event.MouseEvent.dwMousePosition.X == this->lastMousePosition.x &&
                ir.Event.MouseEvent.dwMousePosition.Y == this->lastMousePosition.y)
            {
                break;
            }

            this->lastMousePosition.x = ir.Event.MouseEvent.dwMousePosition.X;
            this->lastMousePosition.y = ir.Event.MouseEvent.dwMousePosition.Y;
        }

        evnt.mouseX      = ir.Event.MouseEvent.dwMousePosition.X;
        evnt.mouseY      = ir.Event.MouseEvent.dwMousePosition.Y;
        evnt.mouseButton = Input::MouseButton::None;
        if (ir.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
            evnt.mouseButton |= Input::MouseButton::Left;
        else if (ir.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
            evnt.mouseButton |= Input::MouseButton::Right;
        else if (ir.Event.MouseEvent.dwButtonState > 0)
            evnt.mouseButton |= Input::MouseButton::Center;

        switch (ir.Event.MouseEvent.dwEventFlags)
        {
        case 0:
            if (ir.Event.MouseEvent.dwButtonState)
                evnt.eventType = SystemEventType::MouseDown;
            else
                evnt.eventType = SystemEventType::MouseUp;
            return;
        case DOUBLE_CLICK:
            evnt.eventType = SystemEventType::MouseDown;
            evnt.mouseButton |= Input::MouseButton::DoubleClicked;
            break;
        case MOUSE_MOVED:
            evnt.eventType = SystemEventType::MouseMove;
            return;
        case MOUSE_WHEELED:
            evnt.eventType = SystemEventType::MouseWheel;
            if (ir.Event.MouseEvent.dwButtonState >= 0x80000000)
                evnt.mouseWheel = Input::MouseWheel::Down;
            else
                evnt.mouseWheel = Input::MouseWheel::Up;
            return;
        }
        break;
    case WINDOW_BUFFER_SIZE_EVENT:
        if (ResizeConsoleBuffer(ir.Event.WindowBufferSizeEvent.dwSize.X, ir.Event.WindowBufferSizeEvent.dwSize.Y))
        {
            evnt.newWidth  = ir.Event.WindowBufferSizeEvent.dwSize.X;
            evnt.newHeight = ir.Event.WindowBufferSizeEvent.dwSize.Y;
            evnt.eventType = SystemEventType::AppResized;
        }
        else
        {
            LOG_ERROR(
                  "Internal error - fail to resize console buffer to %dx%d",
                  ir.Event.WindowBufferSizeEvent.dwSize.X,
                  ir.Event.WindowBufferSizeEvent.dwSize.Y);
        }
        break;
    }
}

void WindowsTerminal::Update()
{
    OnFlushToScreen();

    if ((screenCanvas.GetCursorVisibility() != lastCursorVisibility) || (screenCanvas.GetCursorX() != lastCursorX) ||
        (screenCanvas.GetCursorY() != lastCursorY) || screenCanvas.GetWidth() != lastWidth ||
        screenCanvas.GetHeight() != lastHeight)
    {
        if (this->OnUpdateCursor())
        {
            // update last cursor information
            lastCursorX          = screenCanvas.GetCursorX();
            lastCursorY          = screenCanvas.GetCursorY();
            lastCursorVisibility = screenCanvas.GetCursorVisibility();

            lastWidth  = screenCanvas.GetWidth();
            lastHeight = screenCanvas.GetHeight();
        }
    }
}

bool WindowsTerminal::IsEventAvailable()
{
    DWORD eventsRead = 0;
    INPUT_RECORD ir;
    CHECK(PeekConsoleInput(hstdIn, &ir, 1, &eventsRead), false, "");
    return (eventsRead > 0);
}

bool WindowsTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    // Windows terminal supports all special character set types
    return true;
}
} // namespace AppCUI::Internal