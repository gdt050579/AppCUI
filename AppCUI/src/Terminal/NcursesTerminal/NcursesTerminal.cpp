#include "NcursesTerminal.hpp"
#include <poll.h>
#include <stdio.h>
#if __APPLE__
#    include <mach-o/dyld.h>
#    include <sys/syslimits.h>
#elif __linux__
#    include <linux/limits.h>
#endif

#if __linux__
static std::string get_self_path()
{
    char self[PATH_MAX] = { 0 };
    int nchar           = readlink("/proc/self/exe", self, sizeof self);
    if (nchar < 0 || nchar >= PATH_MAX)
    {
        return self;
    }

    return self;
}
#elif HAVE_WINDOWS_H
static std::wstring get_self_path()
{
    wchar_t self[MAX_PATH] = { 0 };
    SetLastError(0);
    DWORD nchar = GetModuleFileNameW(NULL, self, MAX_PATH);

    if (nchar == 0 ||
        (nchar == MAX_PATH && ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) || (self[MAX_PATH - 1] != 0))))
    {
        return self;
    }

    return self;
}
#elif __APPLE__
static std::string get_self_path()
{
    char self[PATH_MAX] = { 0 };
    uint32_t size       = sizeof self;

    if (_NSGetExecutablePath(self, &size) != 0)
    {
        return self;
    }

    return self;
}
#elif HAVE_GETEXECNAME
static std::string get_self_path()
{
    std::string execname = getexecname();
    if (car(execname) == chr('/'))
    {
        return execname;
    }

    return scat3(getcwd_wrap(), chr('/'), execname);
}
#else
static val get_self_path(void)
{
    char self[PATH_MAX];
    if (argv[0] && realpath(argv[0], self))
    {
        q return self;
    }

    return lit(HARD_INSTALLATION_PATH);
}
#endif

namespace AppCUI::Internal
{
using namespace Input;
using namespace Application;
using namespace AppCUI::Graphics;

constexpr int KEY_DELETE          = 0x7F;
constexpr int KEY_COMBO_MODE      = 0x60; // ` key
constexpr int KEY_LOCK_COMBO_MODE = ' ';
constexpr int KEY_ESCAPE          = '\x1B'; // ESC key
constexpr int KEY_TAB             = '\t';

const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

const static ColorPair defaultColorPair = { Color::White, Color::DarkBlue };
const static ColorPair pressedColorPair = { Color::Green, Color::Red };

bool NcursesTerminal::OnInit(const Application::InitializationData&)
{
    bool setTerminInfo = false;
    if (const char* terminfo = std::getenv("TERMINFO"))
    {
        std::cout << "Your TERMINFO is: " << terminfo << '\n';

        struct stat myStat;
        if ((stat(terminfo, &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR))
        {
            std::cout << "TERMINFO folder exists!\n";
        }
        else
        {
            std::cout << "TERMINFO folder does not exists!\n";
            setTerminInfo = true;
        }
    }

    if (setTerminInfo)
    {
        auto path = get_self_path();
        path.substr(0, path.find_last_of('/'));
        path += "/terminfo";
        std::cout << "Your terminfo path is: " << path << '\n';
        CHECK(setenv("TERMINFO", path.c_str(), true), false, "");
    }
    else
    {
        std::cout << "Your terminfo path is all set!\n";
    }

    if (!InitScreen())
        return false;
    if (!InitInput())
        return false;
    return true;
}

void NcursesTerminal::OnUnInit()
{
    UnInitInput();
    UnInitScreen();
}

void NcursesTerminal::OnFlushToScreen()
{
    Graphics::Character* charsBuffer = this->screenCanvas.GetCharactersBuffer();
    const size_t width               = this->screenCanvas.GetWidth();
    const size_t height              = this->screenCanvas.GetHeight();
    const size_t comboBoxTop         = height - COMBO_DLG_ROW;
    const size_t comboBoxLeft        = width - COMBO_DLG_COL;
    const size_t comboBoxBottom      = height - 1;
    const size_t comboBoxRight       = width - 1;

    cchar_t t;
    if (mode == TerminalMode::TerminalCombo)
    {
        if (canvasState == nullptr)
        {
            canvasState = new Character[COMBO_DLG_ROW * COMBO_DLG_COL];
        }
        for (size_t x = comboBoxLeft; x <= comboBoxRight; x++)
        {
            for (size_t y = comboBoxTop; y <= comboBoxBottom; y++)
            {
                const Character ch                                                  = charsBuffer[y * width + x];
                canvasState[(y - comboBoxTop) * COMBO_DLG_COL + (x - comboBoxLeft)] = ch;
            }
        }

        screenCanvas.FillRect(comboBoxLeft, comboBoxTop, comboBoxRight, comboBoxBottom, ' ', defaultColorPair);
        screenCanvas.DrawRect(
              comboBoxLeft, comboBoxTop, comboBoxRight, comboBoxBottom, defaultColorPair, LineType::Double);
        DrawModifiers(comboBoxLeft, comboBoxTop, comboBoxRight, comboBoxBottom, defaultColorPair, pressedColorPair);
    }
    else if (canvasState != nullptr)
    {
        for (size_t x = comboBoxLeft; x <= comboBoxRight; x++)
        {
            for (size_t y = comboBoxTop; y <= comboBoxBottom; y++)
            {
                const Character ch         = canvasState[(y - comboBoxTop) * COMBO_DLG_COL + (x - comboBoxLeft)];
                charsBuffer[y * width + x] = ch;
            }
        }
        delete[] canvasState;
        canvasState = nullptr;
    }

    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const Graphics::Character ch = charsBuffer[y * width + x];
            t                            = { 0, { ch.Code, 0 } };
            colors.SetColor(ch.Color.Foreground, ch.Color.Background);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(ch.Color.Foreground, ch.Color.Background);
        }
    }

    move(lastCursorY, lastCursorX);
    refresh();
}

void NcursesTerminal::OnFlushToScreen(const Graphics::Rect& /*r*/)
{
    // No implementation for the moment, copy the entire screen
    OnFlushToScreen();
}

bool NcursesTerminal::OnUpdateCursor()
{
    if (screenCanvas.GetCursorVisibility())
    {
        curs_set(1);
        move(screenCanvas.GetCursorY(), screenCanvas.GetCursorX());
    }
    else
    {
        curs_set(0);
    }
    refresh();
    return true;
}

void NcursesTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    evnt.eventType        = SystemEventType::None;
    evnt.keyCode          = Key::None;
    evnt.unicodeCharacter = 0;
    // select on stdin with timeout, should  translate to about ~30 fps
    pollfd readFD;
    readFD.fd     = STDIN_FILENO;
    readFD.events = POLLIN | POLLERR;
    // poll for 30 milliseconds
    poll(&readFD, 1, 30);

    int c = getch();
    if (c == ERR)
    {
        return;
    }
    else if (c == KEY_MOUSE)
    {
        HandleMouse(evnt, c);
        return;
    }
    else if (c == KEY_RESIZE)
    {
        // one day, but this day is not today
        // evnt.eventType = SystemEvents::APP_RESIZED;
        return;
    }
    else
    {
        HandleKey(evnt, c);
        return;
    }
    refresh();
}
bool NcursesTerminal::IsEventAvailable()
{
    NOT_IMPLEMENTED(false);
}

void NcursesTerminal::RestoreOriginalConsoleSettings()
{
}

bool NcursesTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    switch (type)
    {
    case AppCUI::Application::SpecialCharacterSetType::Unicode:
    {
        auto term = getenv("TERM");
        if ((term) && (strcmp(term, "linux") == 0))
            return false; // we are in a real linux tty and as such this mode will not be supported
        // otherwise we are in an "X" mode terminal that has a font that propertly supports some characters
        return true;
    }
    case AppCUI::Application::SpecialCharacterSetType::LinuxTerminal:
        // Linux terminal always work (this is a subset of unicode characters so it will be available for both TTY and
        // "X" mode terminals)
        return true;
    case AppCUI::Application::SpecialCharacterSetType::Ascii:
        // ascii always works
        return true;
    default:
        RETURNERROR(false, "Unknwon special character set --> this is a fallback case, it should not be reached !");
        break;
    }
}

bool NcursesTerminal::InitScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    clear();

    colors.Init();

    size_t width  = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height < MAX_TTY_ROW || width < MAX_TTY_COL, false, "Failed to get window sizes");
    // create canvases
    CHECK(screenCanvas.Create(width, height),
          false,
          "Fail to create an internal canvas of %d x %d size",
          width,
          height);
    CHECK(originalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          width,
          height);

    return true;
}

bool NcursesTerminal::InitInput()
{
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    nonl();
    raw();
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
    mouseinterval(0);
    set_escdelay(0);

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        keyTranslationMatrix[KEY_F(i + 1)] = static_cast<Key>(static_cast<uint32>(Key::F1) + i);

        // If we press F1 + shift => it generates F13
        keyTranslationMatrix[KEY_F(i + 13)] = static_cast<Key>(static_cast<uint32>(Key::F1) + i) | Key::Shift;
    }

    keyTranslationMatrix[KEY_ENTER]     = Key::Enter;
    keyTranslationMatrix[13]            = Key::Enter;
    keyTranslationMatrix[10]            = Key::Enter;
    keyTranslationMatrix[KEY_ESCAPE]    = Key::Escape;
    keyTranslationMatrix[KEY_DELETE]    = Key::Delete;
    keyTranslationMatrix[KEY_BACKSPACE] = Key::Backspace;
    keyTranslationMatrix[KEY_TAB]       = Key::Tab;
    keyTranslationMatrix[KEY_LEFT]      = Key::Left;
    keyTranslationMatrix[KEY_UP]        = Key::Up;
    keyTranslationMatrix[KEY_DOWN]      = Key::Down;
    keyTranslationMatrix[KEY_RIGHT]     = Key::Right;
    keyTranslationMatrix[KEY_PPAGE]     = Key::PageUp;
    keyTranslationMatrix[KEY_NPAGE]     = Key::PageDown;
    keyTranslationMatrix[KEY_HOME]      = Key::Home;
    keyTranslationMatrix[KEY_END]       = Key::End;
    keyTranslationMatrix[KEY_DELETE]    = Key::Backspace;

    mode = TerminalMode::TerminalNormal;

    return true;
}

void NcursesTerminal::UnInitScreen()
{
    endwin();
}

void NcursesTerminal::UnInitInput()
{
}

void NcursesTerminal::HandleMouse(SystemEvent& evt, const int)
{
    MEVENT mouseEvent;
    if (getmouse(&mouseEvent) == OK)
    {
        evt.mouseX        = mouseEvent.x;
        evt.mouseY        = mouseEvent.y;
        const auto& state = mouseEvent.bstate;

        if (state & BUTTON1_PRESSED)
        {
            evt.eventType = SystemEventType::MouseDown;
        }
        else if (state & BUTTON1_RELEASED)
        {
            evt.eventType = SystemEventType::MouseUp;
        }
        else if (state & REPORT_MOUSE_POSITION)
        {
            evt.eventType = SystemEventType::MouseMove;
        }
    }
}

void NcursesTerminal::HandleKey(SystemEvent& evt, const int c)
{
    evt.eventType = SystemEventType::KeyPressed;
    if (mode == TerminalMode::TerminalCombo)
    {
        HandleKeyComboMode(evt, c);
    }
    else if (mode == TerminalMode::TerminalNormal)
    {
        HandleKeyNormalMode(evt, c);
    }
}

void NcursesTerminal::HandleKeyNormalMode(SystemEvent& evt, const int c)
{
    if (c == KEY_COMBO_MODE)
    {
        comboKeysMask = 0;
        mode          = TerminalMode::TerminalCombo;
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    }

    if (keyTranslationMatrix.find(c) != keyTranslationMatrix.end())
    {
        evt.keyCode = keyTranslationMatrix[c];
        return;
    }

    if ((c >= 32) && (c <= 127))
    {
        evt.unicodeCharacter = c;
        if (islower(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'a'));
        }
        else if (isupper(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'A'));
        }
        else if (isdigit(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<uint32>(Key::N0) + (c - '0'));
        }
        else if (c == ' ')
        {
            evt.keyCode |= Key::Space;
        }
        return;
    }
    evt.eventType = SystemEventType::None;
}

void NcursesTerminal::HandleKeyComboMode(SystemEvent& evt, const int c)
{
    switch (c)
    {
    case KEY_LOCK_COMBO_MODE:
        isComboModeLocked ^= true;
        if (!isComboModeLocked)
        {
            mode = TerminalMode::TerminalNormal;
        }
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    case KEY_ESCAPE:
        isComboModeLocked = false;
        mode              = TerminalMode::TerminalNormal;
        evt.eventType     = SystemEventType::RequestRedraw;
        return;
    case 'a':
        comboKeysMask ^= static_cast<uint32>(Key::Alt);
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    case 'c':
        comboKeysMask ^= static_cast<uint32>(Key::Ctrl);
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    case 's':
        comboKeysMask ^= static_cast<uint32>(Key::Shift);
        evt.eventType = SystemEventType::RequestRedraw;
        return;
    }

    if (islower(c))
    {
        evt.keyCode |= static_cast<Key>(comboKeysMask) | static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'a'));
    }
    else if (isdigit(c))
    {
        int functionKeyIndex = c - '0' - 1;
        if (c == 48)
        {
            functionKeyIndex = 9;
        }
        evt.keyCode |=
              static_cast<Key>(comboKeysMask) | static_cast<Key>(static_cast<uint32>(Key::F1) + functionKeyIndex);
    }
    if (!isComboModeLocked)
    {
        mode = TerminalMode::TerminalNormal;
        OnFlushToScreen();
    }
}

void NcursesTerminal::DrawModifier(
      const size_t x,
      const size_t y,
      const ColorPair& defaultColorPair,
      const ColorPair& pressedColorPair,
      const Key& modifier,
      const char printedModifier)
{
    ColorPair colorPair = defaultColorPair;
    if ((comboKeysMask & static_cast<uint32>(modifier)) == static_cast<uint32>(modifier))
        colorPair = pressedColorPair;
    screenCanvas.WriteCharacter(x, y, printedModifier, colorPair);
}

void NcursesTerminal::DrawModifiers(
      const size_t left,
      const size_t top,
      const size_t right,
      const size_t bottom,
      const ColorPair& defaultColorPair,
      const ColorPair& pressedColorPair)
{
    const size_t modifierFirstX = left + (right - left) / 2 - 1;
    const size_t modifierY      = top + (bottom - top) / 2;

    DrawModifier(modifierFirstX, modifierY, defaultColorPair, pressedColorPair, Key::Alt, 'A');
    DrawModifier(modifierFirstX + 1, modifierY, defaultColorPair, pressedColorPair, Key::Ctrl, 'C');
    DrawModifier(modifierFirstX + 2, modifierY, defaultColorPair, pressedColorPair, Key::Shift, 'S');

    ColorPair colorPair = defaultColorPair;
    if (isComboModeLocked)
        colorPair = pressedColorPair;

    screenCanvas.WriteCharacter(modifierFirstX + 3, modifierY, '_', colorPair);
}

void DebugChar(int y, int c, const char* prefix)
{
    string_view myName = keyname(c);
    move(y, 0);
    clrtoeol();
    addstr((std::string(prefix) + " " + std::to_string(c) + " " + myName.data()).c_str());
}
} // namespace AppCUI::Internal
