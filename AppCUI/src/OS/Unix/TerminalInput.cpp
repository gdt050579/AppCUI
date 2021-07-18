#include "os.h"

// See docs/TERMINAL.md

/*
    Some of the keys are not yet supported.
    KeyTranslation is used for low hanging fruit Key command translation.
    Advanced commands that fill more than event.keyCode are processed separately
*/

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

bool Terminal::initInput()
{
    for (size_t i = 0; i < 26; i++)
    {
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_A + i)] = static_cast<Key::Type>(Key::A + i);
    }

    for (size_t i = 0; i < 10; i++)
    {
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_0 + i)] = static_cast<Key::Type>(Key::N0 + i);
    }

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + i)] = static_cast<Key::Type>(Key::F1 + i);
    }

    KeyTranslation[SDL_SCANCODE_RETURN] = Key::Enter;
    KeyTranslation[SDL_SCANCODE_ESCAPE] = Key::Escape;
    KeyTranslation[SDL_SCANCODE_INSERT] = Key::Insert;
    KeyTranslation[SDL_SCANCODE_DELETE] = Key::Delete;
    KeyTranslation[SDL_SCANCODE_BACKSPACE] = Key::Backspace;
    KeyTranslation[SDL_SCANCODE_TAB] = Key::Tab;
    KeyTranslation[SDL_SCANCODE_LEFT] = Key::Left;
    KeyTranslation[SDL_SCANCODE_UP] = Key::Up;
    KeyTranslation[SDL_SCANCODE_DOWN] = Key::Down;
    KeyTranslation[SDL_SCANCODE_RIGHT] = Key::Right;
    KeyTranslation[SDL_SCANCODE_PAGEUP] = Key::PageUp;
    KeyTranslation[SDL_SCANCODE_PAGEDOWN] = Key::PageDown;
    KeyTranslation[SDL_SCANCODE_HOME] = Key::Home;
    KeyTranslation[SDL_SCANCODE_END] = Key::End;
    KeyTranslation[SDL_SCANCODE_SPACE] = Key::Space;

    return true;
}

void Terminal::handleMouse(SystemEvents::Event &evt, const SDL_Event &eSdl)
{
    if (eSdl.type == SDL_MOUSEBUTTONDOWN)
    {
        evt.eventType = SystemEvents::MOUSE_DOWN;
        evt.mouseX = eSdl.button.x / charWidth;
        evt.mouseY = eSdl.button.y / charHeight;
    }
    else if (eSdl.type == SDL_MOUSEBUTTONUP)
    {
        evt.eventType = SystemEvents::MOUSE_UP;
        evt.mouseX = eSdl.button.x / charWidth;
        evt.mouseY = eSdl.button.y / charHeight;
    }
    else if (eSdl.type == SDL_MOUSEMOTION)
    {
        evt.eventType = SystemEvents::MOUSE_MOVE;
        evt.mouseX = eSdl.motion.x / charWidth;
        evt.mouseY = eSdl.motion.y / charHeight;
    }
}

void Terminal::handleKey(SystemEvents::Event &evt, const SDL_Event &eSdl)
{
    evt.eventType = SystemEvents::KEY_PRESSED;

    const SDL_Keymod keyModifiers = static_cast<SDL_Keymod>(eSdl.key.keysym.mod);
    const SDL_Scancode virtualKey = eSdl.key.keysym.scancode;
    const SDL_Keycode keyCode = eSdl.key.keysym.sym;

    if (KeyTranslation.count(virtualKey))
    {
        evt.keyCode = KeyTranslation[virtualKey];
    }

    if (keyCode >= 32 && keyCode <= 127)
    {
        evt.asciiCode = keyCode;
    }

    if (keyModifiers & KMOD_ALT)
    {
        evt.keyCode |= Key::Alt;
        evt.asciiCode = 0;
    }
    if (keyModifiers & KMOD_CTRL)
    {
        evt.keyCode |= Key::Ctrl;
        evt.asciiCode = 0;
    }
    if (keyModifiers & KMOD_SHIFT)
    {
        evt.keyCode |= Key::Shift;
        evt.asciiCode = toupper(evt.asciiCode);
    }
}

void Terminal::GetSystemEvent(AppCUI::Internal::SystemEvents::Event &evnt)
{
    evnt.eventType = SystemEvents::NONE;
    evnt.keyCode = Key::None;
    evnt.asciiCode = 0;

    SDL_Event e;
    // wait 30 ms max for the next event
    if (!SDL_WaitEventTimeout(&e, 30))
    {
        return;
    }
    switch (e.type)
    {
    case SDL_QUIT:
        evnt.eventType = SystemEvents::APP_CLOSE;
        break;
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        handleMouse(evnt, e);
        break;
    case SDL_KEYDOWN:
        handleKey(evnt, e);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        evnt.eventType = SystemEvents::APP_RESIZED;
        evnt.newWidth = e.window.data1;
        evnt.newHeight = e.window.data2;
        break;
    default:
        break;
    }
}

bool Terminal::IsEventAvailable()
{
	NOT_IMPLEMENTED(false);
}

void Terminal::uninitInput()
{
}