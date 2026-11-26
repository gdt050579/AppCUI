#include "NcursesTerminal.hpp"
#include "TerminalKeyDecoding.hpp"
#include "../TerminalFactory.hpp"
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
    int nchar           = static_cast<int>(readlink("/proc/self/exe", self, sizeof self - 1));
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

constexpr ColorPair DEFAULT_COMBO_COLOR{ Color::White, Color::DarkBlue };
constexpr ColorPair PRESSED_COMBO_COLOR{ Color::Green, Color::Red };

bool NcursesTerminal::OnInit(const Application::InitializationData& initData)
{
    fpsMode          = (initData.Flags & InitializationFlags::EnableFPSMode) != InitializationFlags::None;
    lastFramesUpdate = std::chrono::steady_clock::now();
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
        if (const auto lastSlash = path.find_last_of('/'); lastSlash != std::string::npos)
        {
            path.resize(lastSlash);
        }
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

        screenCanvas.FillRect(comboBoxLeft, comboBoxTop, comboBoxRight, comboBoxBottom, ' ', DEFAULT_COMBO_COLOR);
        screenCanvas.DrawRect(
              comboBoxLeft, comboBoxTop, comboBoxRight, comboBoxBottom, DEFAULT_COMBO_COLOR, LineType::Double);
        DrawModifiers(comboBoxLeft, comboBoxTop, comboBoxRight, comboBoxBottom, DEFAULT_COMBO_COLOR, PRESSED_COMBO_COLOR);
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
            cchar_t t{};
            t.chars[0] = static_cast<wchar_t>(ch.Code);
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
    evnt.updateFrames     = false;
    // select on stdin with timeout, should  translate to about ~30 fps
    pollfd readFD;
    readFD.fd     = STDIN_FILENO;
    readFD.events = POLLIN | POLLERR;
    // poll for 30 milliseconds
    poll(&readFD, 1, 30);

    if (fpsMode)
    {
        // same frame rate as the other frontends (~30 fps)
        const auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFramesUpdate).count() >= 33)
        {
            evnt.updateFrames = true;
            lastFramesUpdate  = now;
        }
    }

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
        int32 width  = 0;
        int32 height = 0;
        getmaxyx(stdscr, height, width);
        CHECKRET(height != ERR || width != ERR, "Failed to get window sizes");
        
        evnt.newWidth = static_cast<uint32>(width);
        evnt.newHeight = static_cast<uint32>(height);
        evnt.eventType = Internal::SystemEventType::AppResized;
        
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
        RETURNERROR(false, "Unknown special character set --> this is a fallback case, it should not be reached !");
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

    int32 width  = 0;
    int32 height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height != ERR || width != ERR, false, "Failed to get window sizes");

    CHECK(screenCanvas.Create(width, height),
          false,
          "Fail to create an internal canvas of %d x %d size",
          static_cast<uint32>(width),
          static_cast<uint32>(height));
    
    CHECK(originalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          static_cast<uint32>(width),
          static_cast<uint32>(height));

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
#ifdef NCURSES_EXT_FUNCS
    use_extended_names(TRUE); // needed for the modified-key capabilities (kUP5, kDC3, ...)
#endif

    BuildKeyTranslationTable();

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

namespace
{
#ifdef NCURSES_EXT_FUNCS
    // xterm modifier parameter (CSI 1;<n>X) -> AppCUI modifiers
    constexpr Key XTERM_MODIFIERS[9] = {
        Key::None,                         // 0 (unused)
        Key::None,                         // 1 (no modifier)
        Key::Shift,                        // 2
        Key::Alt,                          // 3
        Key::Alt | Key::Shift,             // 4
        Key::Ctrl,                         // 5
        Key::Ctrl | Key::Shift,            // 6
        Key::Ctrl | Key::Alt,              // 7
        Key::Ctrl | Key::Alt | Key::Shift, // 8
    };
#endif
    inline bool IsValidCapability(const char* value)
    {
        return (value != nullptr) && (value != reinterpret_cast<const char*>(-1));
    }
} // namespace

void NcursesTerminal::BuildKeyTranslationTable()
{
    keyTranslationMatrix.clear();

    // function keys: terminfo (xterm convention) exposes modified F-keys as blocks of 12 extra function keys
    constexpr Key F_KEY_BLOCKS[] = { Key::None, Key::Shift, Key::Ctrl, Key::Ctrl | Key::Shift, Key::Alt, Key::Alt | Key::Shift };
    for (uint32 block = 0; block < sizeof(F_KEY_BLOCKS) / sizeof(F_KEY_BLOCKS[0]); block++)
    {
        for (uint32 i = 0; i < 12; i++)
        {
            const uint32 fn = block * 12 + i + 1;
            if (fn > 63) // ncurses defines KEY_F(0) .. KEY_F(63)
                break;
            keyTranslationMatrix[KEY_F(fn)] = F_KEY_BLOCKS[block] | static_cast<Key>(static_cast<uint32>(Key::F1) + i);
        }
    }
#ifdef NCURSES_EXT_FUNCS
    // Shift+F5 .. Shift+F12 of macOS Terminal: VT220 codes the xterm descriptions do not define (TerminalKeyDecoding.hpp).
    // Only a sequence the terminal description does not know is added (rxvt / linux define them as other keys).
    for (const auto& shifted : TerminalKeys::MAC_SHIFTED_FUNCTION_KEYS)
    {
        const std::string sequence(shifted.sequence);
        if (key_defined(sequence.c_str()) == 0)
            define_key(sequence.c_str(), KEY_F(TerminalKeys::ShiftedFunctionKeyNumber(shifted.functionKey)));
    }
#endif

    // Ctrl + letter is delivered as the control codes 1..26
    for (uint32 i = 0; i < 26; i++)
        keyTranslationMatrix[static_cast<int>(i + 1)] = Key::Ctrl | static_cast<Key>(static_cast<uint32>(Key::A) + i);
    keyTranslationMatrix[0] = Key::Ctrl | Key::Space; // Ctrl+Space / Ctrl+@

    // control codes that terminals use for dedicated keys (override the Ctrl+letter entries above)
    keyTranslationMatrix[KEY_TAB]   = Key::Tab;   // Ctrl+I
    keyTranslationMatrix[10]        = Key::Enter; // Ctrl+J
    keyTranslationMatrix[13]        = Key::Enter; // Ctrl+M
    keyTranslationMatrix[KEY_ENTER] = Key::Enter;
    // 0x08 is Backspace only when the Backspace key sends ^H, otherwise it is Ctrl+H. The erase character of the terminal
    // (what Backspace really sends) decides before kbs: macOS Terminal sends DEL while its xterm-256color says kbs=^H
    const char* kbs       = tigetstr(const_cast<char*>("kbs"));
    const bool kbsIsCtrlH = IsValidCapability(kbs) && (kbs[0] == '\b') && (kbs[1] == 0);
    if (TerminalKeys::IsCtrlHBackspace(kbsIsCtrlH, static_cast<unsigned char>(erasechar())))
        keyTranslationMatrix[8] = Key::Backspace;
#ifdef NCURSES_EXT_FUNCS
    else if (kbsIsCtrlH)
        define_key("\b", 0); // with the keypad on, ncurses would return KEY_BACKSPACE for ^H (kbs): it stays Ctrl+H
#endif
    keyTranslationMatrix[KEY_DELETE]    = Key::Backspace; // 0x7F (DEL) is what most terminals send for Backspace
    keyTranslationMatrix[KEY_BACKSPACE] = Key::Backspace;
    keyTranslationMatrix[KEY_ESCAPE]    = Key::Escape;

    // navigation / editing keys
    keyTranslationMatrix[KEY_LEFT]  = Key::Left;
    keyTranslationMatrix[KEY_UP]    = Key::Up;
    keyTranslationMatrix[KEY_DOWN]  = Key::Down;
    keyTranslationMatrix[KEY_RIGHT] = Key::Right;
    keyTranslationMatrix[KEY_PPAGE] = Key::PageUp;
    keyTranslationMatrix[KEY_NPAGE] = Key::PageDown;
    keyTranslationMatrix[KEY_HOME]  = Key::Home;
    keyTranslationMatrix[KEY_END]   = Key::End;
    keyTranslationMatrix[KEY_DC]    = Key::Delete;
    keyTranslationMatrix[KEY_IC]    = Key::Insert;
    keyTranslationMatrix[KEY_BTAB]  = Key::Shift | Key::Tab;

    // shifted variants with standard terminfo capabilities
    keyTranslationMatrix[KEY_SLEFT]     = Key::Shift | Key::Left;
    keyTranslationMatrix[KEY_SRIGHT]    = Key::Shift | Key::Right;
    keyTranslationMatrix[KEY_SR]        = Key::Shift | Key::Up;
    keyTranslationMatrix[KEY_SF]        = Key::Shift | Key::Down;
    keyTranslationMatrix[KEY_SHOME]     = Key::Shift | Key::Home;
    keyTranslationMatrix[KEY_SEND]      = Key::Shift | Key::End;
    keyTranslationMatrix[KEY_SPREVIOUS] = Key::Shift | Key::PageUp;
    keyTranslationMatrix[KEY_SNEXT]     = Key::Shift | Key::PageDown;
    keyTranslationMatrix[KEY_SDC]       = Key::Shift | Key::Delete;
    keyTranslationMatrix[KEY_SIC]       = Key::Shift | Key::Insert;

#ifdef NCURSES_EXT_FUNCS
    // other modifier combinations are exposed (xterm-like terminals) as extended capabilities: kUP5 = Ctrl+Up, ...
    struct ExtendedKey
    {
        const char* name;
        Key key;
    };
    constexpr ExtendedKey EXTENDED_KEYS[] = {
        { "kUP", Key::Up },   { "kDN", Key::Down },    { "kLFT", Key::Left },     { "kRIT", Key::Right }, { "kHOM", Key::Home },
        { "kEND", Key::End }, { "kPRV", Key::PageUp }, { "kNXT", Key::PageDown }, { "kDC", Key::Delete }, { "kIC", Key::Insert },
    };
    char capName[16];
    for (const auto& ek : EXTENDED_KEYS)
    {
        for (uint32 modifier = 3; modifier <= 8; modifier++)
        {
            snprintf(capName, sizeof(capName), "%s%u", ek.name, modifier);
            const char* sequence = tigetstr(capName);
            if (!IsValidCapability(sequence))
                continue;
            const int code = key_defined(sequence);
            if (code > 0)
                keyTranslationMatrix[code] = XTERM_MODIFIERS[modifier] | ek.key;
        }
    }
    // modified arrows the terminal description does not list (an older terminfo; the macOS Terminal profile edit that
    // makes Option+Left / Option+Right send \033[1;3D / \033[1;3C): the xterm form, under codes of AppCUI's own
    struct ArrowKey
    {
        char final;
        Key key;
    };
    constexpr ArrowKey ARROW_KEYS[] = { { 'A', Key::Up }, { 'B', Key::Down }, { 'C', Key::Right }, { 'D', Key::Left } };
    int privateCode = TerminalKeys::PRIVATE_KEY_CODE_BASE;
    for (const auto& arrow : ARROW_KEYS)
    {
        for (uint32 modifier = 2; modifier <= 8; modifier++)
        {
            const std::string sequence = TerminalKeys::XtermModifiedKeySequence(modifier, arrow.final);
            if (key_defined(sequence.c_str()) != 0) // known (the extended names above) or a prefix of another sequence
                continue;
            if (define_key(sequence.c_str(), privateCode) == OK)
                keyTranslationMatrix[privateCode] = XTERM_MODIFIERS[modifier] | arrow.key;
            privateCode++;
        }
    }
#endif
}

bool NcursesTerminal::TranslateKey(const int c, Key& keyCode, char16& unicodeCharacter)
{
    const auto it = keyTranslationMatrix.find(c);
    if (it != keyTranslationMatrix.end())
    {
        keyCode = it->second;
        return true;
    }
    if ((c >= 32) && (c < 127))
    {
        unicodeCharacter = static_cast<char16>(c);
        if (islower(c))
            keyCode = static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'a'));
        else if (isupper(c))
            keyCode = static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'A'));
        else if (isdigit(c))
            keyCode = static_cast<Key>(static_cast<uint32>(Key::N0) + (c - '0'));
        else if (c == ' ')
            keyCode = Key::Space;
        else
            keyCode = Key::None; // punctuation: only the unicode character is available
        return true;
    }
    return false;
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

    if (c == KEY_ESCAPE)
    {
        // terminals send Alt/Meta + key (e.g. "Use Option as Meta key" on macOS) as ESC followed by the key, in the
        // same write -> if another byte is already available it is an Alt combination, otherwise a plain Escape
        const int next = getch(); // non-blocking (nodelay)
        if ((next >= 0x80) && (next <= 0xFF))
        {
            // Meta + a character typed in UTF-8 (Option+ș with "Use Option as Meta key"): there is no Alt+<character> key,
            // the character is delivered (never an Escape that would close a dialog, then the character)
            char16 character = 0;
            if (ReadUtf8Character(next, character))
                evt.unicodeCharacter = character;
            else
                evt.eventType = SystemEventType::None;
            return;
        }
        if (next != ERR)
        {
            Key key        = Key::None;
            char16 unicode = 0;
            if ((next != KEY_ESCAPE) && (next != KEY_COMBO_MODE) && (TranslateKey(next, key, unicode)) && (key != Key::None))
            {
                if ((unicode >= 'A') && (unicode <= 'Z'))
                    key |= Key::Shift;
                evt.keyCode = Key::Alt | key;
                return;
            }
            ungetch(next);
        }
        evt.keyCode = Key::Escape;
        return;
    }

    if ((c >= 0x80) && (c <= 0xFF))
    {
        // UTF-8 input: a letter of the keyboard layout (s with comma, a with breve, ...) or, on macOS without "Use Option as
        // Meta key", Option+letter (f with hook, ...): delivered as the typed character
        char16 character = 0;
        if (ReadUtf8Character(c, character))
            evt.unicodeCharacter = character;
        else
            evt.eventType = SystemEventType::None; // malformed, a stray continuation byte, or beyond the BMP (char16)
        return;
    }

    if (!TranslateKey(c, evt.keyCode, evt.unicodeCharacter))
        evt.eventType = SystemEventType::None;
}

// The rest of a UTF-8 sequence that starts with `lead` (the terminal sends it in one write; a short wait covers a sequence
// split between two reads, e.g. over ssh). False for a malformed sequence or a character beyond the BMP.
bool NcursesTerminal::ReadUtf8Character(const int lead, char16& character)
{
    unsigned char bytes[4]    = { static_cast<unsigned char>(lead), 0, 0, 0 };
    const unsigned int length = TerminalKeys::Utf8SequenceLength(bytes[0]);
    if (length == 0)
        return false;
    unsigned int count = 1;
    unsigned int waits = 0;
    while (count < length)
    {
        const int next = getch(); // non-blocking (nodelay)
        if (next == ERR)
        {
            if (waits++ >= 3)
                break;
            pollfd readFD{ STDIN_FILENO, POLLIN, 0 };
            poll(&readFD, 1, 10);
            continue;
        }
        if ((next < 0x80) || (next > 0xBF))
        {
            ungetch(next); // not part of the sequence: it is the next key
            break;
        }
        bytes[count++] = static_cast<unsigned char>(next);
    }
    char32_t codePoint = 0;
    if ((count != length) || !TerminalKeys::DecodeUtf8(bytes, count, codePoint) || (codePoint > 0xFFFF))
        return false;
    character = static_cast<char16>(codePoint);
    return true;
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

    // ASCII only: islower / isdigit of a byte above 0x7F depend on the locale, and of a KEY_* code are undefined
    if ((c >= 'a') && (c <= 'z'))
    {
        evt.keyCode |= static_cast<Key>(comboKeysMask) | static_cast<Key>(static_cast<uint32>(Key::A) + (c - 'a'));
    }
    else if ((c >= '0') && (c <= '9'))
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

// AppCUI::Frontend::Ncurses -> FrontendType::Terminal (and FrontendType::Default on Unix / macOS)
static const TerminalRegistrar ncursesTerminalRegistrar(
      Application::FrontendType::Terminal, CreateTerminal<NcursesTerminal>, DefaultFrontendPriority::Native);
} // namespace AppCUI::Internal
