#include "Terminal/WindowsTerminal/WindowsTerminal.hpp"
#include <string.h>

using namespace AppCUI::Internal;
using namespace AppCUI::Application;
using namespace AppCUI::Graphics;

WindowsTerminal::WindowsTerminal()
{
    ConsoleBufferCount      = 0;
    this->fpsMode           = false;
    this->lastMousePosition = { 0xFFFFFFFFu, 0xFFFFFFFFu };
}
WindowsTerminal::~WindowsTerminal()
{
    ConsoleBufferCount = 0;
}
bool WindowsTerminal::ResizeConsoleBuffer(unsigned int width, unsigned int height)
{
    const unsigned int newCount = ((width * height) | 0xFF) + 1;
    if (newCount <= ConsoleBufferCount)
        return true; // no need to resize
    this->ConsoleBuffer.reset(new CHAR_INFO[newCount]);
    CHECK(ConsoleBuffer.get(),
          false,
          "Fail to allocate: %d characters for a %dx%d sized terminal",
          newCount,
          width,
          height);
    this->ConsoleBufferCount = newCount;
    return true;
}
bool WindowsTerminal::CopyOriginalScreenBuffer(
      unsigned int width, unsigned int height, unsigned int mouseX, unsigned int mouseY)
{
    CHECK(this->OriginalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          width,
          height);
    SMALL_RECT BufRect = { 0, 0, (SHORT) (width - 1), (SHORT) (height - 1) };

    // allocate memory for a console buffer
    CHAR_INFO* temp = new CHAR_INFO[(size_t) width * (size_t) height];
    CHECK(temp, false, "Fail to allocate %dx%d buffer to store the original screem buffer !", width, height);

    // make a copy of the original screem buffer into the canvas
    while (true)
    {
        CHECKBK(
              ReadConsoleOutput(this->hstdOut, temp, { (SHORT) width, (SHORT) height }, { 0, 0 }, &BufRect),
              "Unable to make a copy of the initial screen buffer !");
        CHAR_INFO* p = temp;
        for (unsigned int y = 0; y < (unsigned int) height; y++)
        {
            for (unsigned int x = 0; x < (unsigned int) width; x++, p++)
            {
                this->OriginalScreenCanvas.WriteCharacter(
                      x,
                      y,
                      p->Char.UnicodeChar,
                      AppCUI::Graphics::ColorPair{ static_cast<AppCUI::Graphics::Color>(p->Attributes & 0x0F),
                                                   static_cast<AppCUI::Graphics::Color>((p->Attributes & 0xF0) >> 4) });
            }
        }
        delete[] temp;
        temp = nullptr;
        // copy current cursor position
        CONSOLE_CURSOR_INFO cInfo;
        CHECKBK(GetConsoleCursorInfo(this->hstdOut, &cInfo), "Unable to read console cursor informations !");
        if (cInfo.bVisible)
            this->OriginalScreenCanvas.HideCursor();
        else
            this->OriginalScreenCanvas.SetCursor(mouseX, mouseY);

        return true;
    }
    delete[] temp;
    return false;
}
bool WindowsTerminal::ResizeConsoleScreenBufferSize(unsigned int width, unsigned int height)
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
bool WindowsTerminal::ResizeConsoleWindowSize(unsigned int width, unsigned int height)
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
bool WindowsTerminal::ComputeCharacterSize(const AppCUI::Application::InitializationData& initData)
{
    if (initData.CharSize == CharacterSize::Default)
        return true; // leave the settings as they are
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize                = sizeof(CONSOLE_FONT_INFOEX);
    cfi.FontWeight            = FW_NORMAL;
    cfi.dwFontSize.X          = 0;
    cfi.nFont                 = 0;
    cfi.FontFamily            = FF_DONTCARE;
    std::string_view fontName = initData.FontName;
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
        RETURNERROR(false, "Invalid/unknwon value for character size: %d", (unsigned int) initData.CharSize);
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
    for (unsigned int tr = 0; tr < KEYTRANSLATION_MATRIX_SIZE; tr++)
        this->KeyTranslationMatrix[tr] = AppCUI::Input::Key::None;
    for (unsigned int tr = 0; tr < 12; tr++)
        this->KeyTranslationMatrix[VK_F1 + tr] =
              static_cast<AppCUI::Input::Key>(((unsigned int) AppCUI::Input::Key::F1) + tr);
    for (unsigned int tr = 'A'; tr <= 'Z'; tr++)
        this->KeyTranslationMatrix[tr] =
              static_cast<AppCUI::Input::Key>(((unsigned int) AppCUI::Input::Key::A) + (tr - 'A'));
    for (unsigned int tr = '0'; tr <= '9'; tr++)
        this->KeyTranslationMatrix[tr] =
              static_cast<AppCUI::Input::Key>(((unsigned int) AppCUI::Input::Key::N0) + (tr - '0'));

    this->KeyTranslationMatrix[VK_RETURN] = AppCUI::Input::Key::Enter;
    this->KeyTranslationMatrix[VK_ESCAPE] = AppCUI::Input::Key::Escape;
    this->KeyTranslationMatrix[VK_INSERT] = AppCUI::Input::Key::Insert;
    this->KeyTranslationMatrix[VK_BACK]   = AppCUI::Input::Key::Backspace;
    this->KeyTranslationMatrix[VK_TAB]    = AppCUI::Input::Key::Tab;
    this->KeyTranslationMatrix[VK_DELETE] = AppCUI::Input::Key::Delete;
    this->KeyTranslationMatrix[VK_LEFT]   = AppCUI::Input::Key::Left;
    this->KeyTranslationMatrix[VK_UP]     = AppCUI::Input::Key::Up;
    this->KeyTranslationMatrix[VK_RIGHT]  = AppCUI::Input::Key::Right;
    this->KeyTranslationMatrix[VK_DOWN]   = AppCUI::Input::Key::Down;
    this->KeyTranslationMatrix[VK_PRIOR]  = AppCUI::Input::Key::PageUp;
    this->KeyTranslationMatrix[VK_NEXT]   = AppCUI::Input::Key::PageDown;
    this->KeyTranslationMatrix[VK_HOME]   = AppCUI::Input::Key::Home;
    this->KeyTranslationMatrix[VK_END]    = AppCUI::Input::Key::End;
    this->KeyTranslationMatrix[VK_SPACE]  = AppCUI::Input::Key::Space;

    this->shiftState = AppCUI::Input::Key::None;
}
bool WindowsTerminal::OnInit(const AppCUI::Application::InitializationData& initData)
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
    CHECK(this->ScreenCanvas.Create(terminalSize.Width, terminalSize.Height),
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
void WindowsTerminal::OnUninit()
{
}
void WindowsTerminal::OnFlushToScreen()
{
    unsigned int w = this->ScreenCanvas.GetWidth();
    unsigned int h = this->ScreenCanvas.GetHeight();
    COORD winSize  = { (SHORT) w, (SHORT) h };
    SMALL_RECT sr  = { 0, 0, winSize.X, winSize.Y };
    // copy the entire buffer
    // LOG_INFO("Flushing a buffer of size: %dx%d = %d chars, allocated = %d ",w,h,w*h,this->ConsoleBufferCount)
    AppCUI::Graphics::Character* c = this->ScreenCanvas.GetCharactersBuffer();
    AppCUI::Graphics::Character* e = c + ((size_t) w * (size_t) h);
    CHAR_INFO* d                   = this->ConsoleBuffer.get();
    while (c < e)
    {
        d->Char.UnicodeChar = c->Code;
        d->Attributes       = ((unsigned char) c->Color.Foreground) | (((unsigned char) (c->Color.Background)) << 4);
        d++;
        c++;
    }
    WriteConsoleOutputW(this->hstdOut, this->ConsoleBuffer.get(), winSize, { 0, 0 }, &sr);
}
bool WindowsTerminal::OnUpdateCursor()
{
    if (this->ScreenCanvas.GetCursorVisibility())
    {
        const COORD c = { (SHORT) this->ScreenCanvas.GetCursorX(), (SHORT) this->ScreenCanvas.GetCursorY() };
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
void WindowsTerminal::GetSystemEvent(AppCUI::Internal::SystemEvent& evnt)
{
    DWORD nrread;
    INPUT_RECORD ir;
    AppCUI::Input::Key eventShiftState;

    evnt.eventType    = SystemEventType::None;
    evnt.updateFrames = false;
    if (this->fpsMode)
    {
        DWORD cTime = GetTickCount();
        DWORD diff  = 33;
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
                    this->startTime   = GetTickCount();
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
        if ((ir.Event.KeyEvent.uChar.UnicodeChar >= 32) && (ir.Event.KeyEvent.bKeyDown))
            evnt.unicodeCharacter = ir.Event.KeyEvent.uChar.UnicodeChar;
        else
            evnt.unicodeCharacter = 0;
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
        if ((((unsigned int) eventShiftState) &
             ((unsigned int) (AppCUI::Input::Key::Alt | AppCUI::Input::Key::Ctrl))) != 0)
            evnt.unicodeCharacter = 0;

        if (evnt.keyCode == AppCUI::Input::Key::None)
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
        evnt.mouseButton = AppCUI::Input::MouseButton::None;
        if (ir.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
            evnt.mouseButton |= AppCUI::Input::MouseButton::Left;
        else if (ir.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
            evnt.mouseButton |= AppCUI::Input::MouseButton::Right;
        else if (ir.Event.MouseEvent.dwButtonState > 0)
            evnt.mouseButton |= AppCUI::Input::MouseButton::Center;

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
            evnt.mouseButton |= AppCUI::Input::MouseButton::DoubleClicked;
            break;
        case MOUSE_MOVED:
            evnt.eventType = SystemEventType::MouseMove;
            return;
        case MOUSE_WHEELED:
            evnt.eventType = SystemEventType::MouseWheel;
            if (ir.Event.MouseEvent.dwButtonState >= 0x80000000)
                evnt.mouseWheel = AppCUI::Input::MouseWheel::Down;
            else
                evnt.mouseWheel = AppCUI::Input::MouseWheel::Up;
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

bool WindowsTerminal::IsEventAvailable()
{
    DWORD eventsRead = 0;
    INPUT_RECORD ir;

    if (PeekConsoleInput(this->hstdIn, &ir, 1, &eventsRead) == FALSE)
        return false;
    return (eventsRead > 0);
}
