#include <string>
#include <poll.h>
#include "ncurses.h"

#include "Terminal/NcursesTerminal/NcursesTerminal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

constexpr int KEY_DELETE = 0x7F;


bool NcursesTerminal::OnInit(const InitializationData& initData)
{
    return initScreen() && initInput();
}

void NcursesTerminal::OnUninit()
{
    uninitInput();
    uninitScreen();
}

void debugChar(int y, int c, const char* prefix)
{
    std::string_view myName = keyname(c);
    move(y, 0);
    clrtoeol();
    addstr((std::string(prefix) + " " + std::to_string(c) + " " + myName.data()).c_str());
}

bool NcursesTerminal::initInput()
{
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    meta(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    raw();
    nonl();

    for (size_t tr = 0; tr < KEY_TRANSLATION_MATRIX_SIZE; tr++)
    {
        this->KeyTranslationMatrix[tr] = Key::None;
    }

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        // If we press F1 + shift => it generates F13
        KeyTranslationMatrix[KEY_F(i+1)] = static_cast<Key>(static_cast<unsigned int>(Key::F1) + i);
        KeyTranslationMatrix[KEY_F(i+13)] = static_cast<Key>(static_cast<unsigned int>(Key::F1) + i) | Key::Shift;
    }

    KeyTranslationMatrix[KEY_ENTER] = Key::Enter;
    KeyTranslationMatrix[13] = Key::Enter;
    KeyTranslationMatrix[10] = Key::Enter;

    KeyTranslationMatrix[KEY_BACKSPACE] = Key::Backspace;
    KeyTranslationMatrix[0x7F] = Key::Backspace;

    
    KeyTranslationMatrix[KEY_UP] = Key::Up;
    KeyTranslationMatrix[KEY_RIGHT] = Key::Right;
    KeyTranslationMatrix[KEY_DOWN] = Key::Down;
    KeyTranslationMatrix[KEY_LEFT] = Key::Left;

    KeyTranslationMatrix[KEY_PPAGE] = Key::PageUp;
    KeyTranslationMatrix[KEY_NPAGE] = Key::PageDown;

    KeyTranslationMatrix[KEY_HOME] = Key::Home;
    KeyTranslationMatrix[KEY_END] = Key::End;

    // KeyTranslationMatrix[] = Key::Tab;
    //KeyTranslationMatrix[KEY_ESCAPE] ? 
    //KeyTranslationMatrix[KEY_INSERT] ? 
    // KeyTranslationMatrix[KEY_DELETE] = Key::Delete;
    return true;
}


void NcursesTerminal::handleMouse(SystemEvents::Event &evt, const int c)
{
    MEVENT mouseEvent;
    if (getmouse(&mouseEvent) == OK)
    {
        evt.mouseX = mouseEvent.x;
        evt.mouseY = mouseEvent.y;
        const auto &state = mouseEvent.bstate;
        
        if (state & BUTTON1_PRESSED) 
        {
            evt.eventType = SystemEvents::MOUSE_DOWN;
        }
        else if (state & BUTTON1_RELEASED)
        {
            evt.eventType = SystemEvents::MOUSE_UP;
        }
        else if (state & REPORT_MOUSE_POSITION) 
        {
            evt.eventType = SystemEvents::MOUSE_MOVE;
        }
    }   
}

void NcursesTerminal::handleKey(SystemEvents::Event &evt, const int c)
{
    // debugChar(0, c, "key");
    evt.eventType = SystemEvents::KEY_PRESSED;

    if (c < KEY_TRANSLATION_MATRIX_SIZE && KeyTranslationMatrix[c] != Key::None)
    {
        evt.keyCode = KeyTranslationMatrix[c];
        return;
    }
    else if ((c >= 32) && (c <= 127))
    {
        evt.asciiCode = c;
        if (islower(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'a'));
        }
        else if (isupper(c))
        {
            evt.keyCode |= Key::Shift | static_cast<Key>(static_cast<unsigned int>(Key::A) + (c - 'A'));
        }
        else if (isdigit(c))
        {
            evt.keyCode |= static_cast<Key>(static_cast<unsigned int>(Key::N0) + (c - '0'));
        }
        else if (c == ' ') 
        {
            evt.keyCode |= Key::Space;
        }
        return;
    }
    else 
    {
        //debugChar(0, c, "unsupported key: ");
        evt.eventType = SystemEvents::NONE;
        return;
    }
}


void NcursesTerminal::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
    evnt.eventType = SystemEvents::NONE;
    evnt.keyCode = Key::None;
    evnt.asciiCode = 0;
    // select on stdin with timeout, should  translate to about ~30 fps
    pollfd readFD;
    readFD.fd = STDIN_FILENO;
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
        handleMouse(evnt, c);
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
        handleKey(evnt, c);
        return;
    }
    refresh();
}
bool NcursesTerminal::IsEventAvailable()
{
	NOT_IMPLEMENTED(false);
}
void NcursesTerminal::uninitInput()
{
}

const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;


bool NcursesTerminal::initScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    clear();

    colors.Init();

    size_t width = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height < MAX_TTY_ROW || width < MAX_TTY_COL, false, "Failed to get window sizes");
    // create canvases
    CHECK(ScreenCanvas.Create(width, height), false, "Fail to create an internal canvas of %d x %d size", width, height);
    CHECK(OriginalScreenCanvas.Create(width, height), false, "Fail to create the original screen canvas of %d x %d size", width, height);
    
    return true;
}

void NcursesTerminal::OnFlushToScreen()
{
    AppCUI::Console::Character* charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const size_t width = ScreenCanvas.GetWidth();
    const size_t height = ScreenCanvas.GetHeight();
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const AppCUI::Console::Character ch = charsBuffer[y * width + x];
            cchar_t t = {0, {ch.Code, 0}};
            colors.SetColor(ch.Color.Forenground, ch.Color.Background);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(ch.Color.Forenground, ch.Color.Background);
        }
    }
    refresh();
}

bool NcursesTerminal::OnUpdateCursor()
{
    if (ScreenCanvas.GetCursorVisibility())
    {
        curs_set(1);
        move(ScreenCanvas.GetCursorY(), ScreenCanvas.GetCursorX());
    }
    else
    {
        curs_set(0);
    }
    refresh();
    return true;
}

void NcursesTerminal::uninitScreen()
{
    endwin();
}
