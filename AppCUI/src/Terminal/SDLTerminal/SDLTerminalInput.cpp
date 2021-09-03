#include "Terminal/SDLTerminal/SDLTerminal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Input;

static Key getShiftState(const SDL_Keymod keyModifiers)
{
    Key currentShiftState = Key::None;
    if (keyModifiers & KMOD_ALT)
    {
        currentShiftState |= Key::Alt;
    }
    if (keyModifiers & KMOD_CTRL)
    {
        currentShiftState |= Key::Ctrl;
    }
    if (keyModifiers & KMOD_SHIFT)
    {
        currentShiftState |= Key::Shift;
    }
    return currentShiftState;
}

bool SDLTerminal::initInput(const InitializationData& initData)
{
    oldShiftState = Key::None;

    for (size_t i = 0; i < 26; i++)
    {
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_A + i)] = static_cast<Key>(((unsigned int) Key::A) + i);
    }

    for (size_t i = 0; i < 10; i++)
    {
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_0 + i)] = static_cast<Key>(((unsigned int) Key::N0) + i);
    }

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + i)] = static_cast<Key>(((unsigned int) Key::F1) + i);
    }

    KeyTranslation[SDL_SCANCODE_RETURN]    = Key::Enter;
    KeyTranslation[SDL_SCANCODE_ESCAPE]    = Key::Escape;
    KeyTranslation[SDL_SCANCODE_INSERT]    = Key::Insert;
    KeyTranslation[SDL_SCANCODE_DELETE]    = Key::Delete;
    KeyTranslation[SDL_SCANCODE_BACKSPACE] = Key::Backspace;
    KeyTranslation[SDL_SCANCODE_TAB]       = Key::Tab;
    KeyTranslation[SDL_SCANCODE_LEFT]      = Key::Left;
    KeyTranslation[SDL_SCANCODE_UP]        = Key::Up;
    KeyTranslation[SDL_SCANCODE_DOWN]      = Key::Down;
    KeyTranslation[SDL_SCANCODE_RIGHT]     = Key::Right;
    KeyTranslation[SDL_SCANCODE_PAGEUP]    = Key::PageUp;
    KeyTranslation[SDL_SCANCODE_PAGEDOWN]  = Key::PageDown;
    KeyTranslation[SDL_SCANCODE_HOME]      = Key::Home;
    KeyTranslation[SDL_SCANCODE_END]       = Key::End;
    KeyTranslation[SDL_SCANCODE_SPACE]     = Key::Space;
    return true;
}

void SDLTerminal::handleMouse(SystemEvent& evt, const SDL_Event& eSdl)
{
    if (eSdl.type == SDL_MOUSEBUTTONDOWN || eSdl.type == SDL_MOUSEBUTTONUP)
    {
        if (eSdl.button.button == SDL_BUTTON_LEFT)
        {
            evt.mouseButton = MouseButton::Left;
        }
        else if (eSdl.button.button == SDL_BUTTON_RIGHT)
        {
            evt.mouseButton = MouseButton::Right;
        }
        else if (eSdl.button.button == SDL_BUTTON_MIDDLE)
        {
            evt.mouseButton = MouseButton::Center;
        }
    }

    if (eSdl.type == SDL_MOUSEBUTTONDOWN)
    {
        evt.eventType = SystemEventType::MouseDown;
        evt.mouseX    = eSdl.button.x / charWidth;
        evt.mouseY    = eSdl.button.y / charHeight;
    }
    else if (eSdl.type == SDL_MOUSEBUTTONUP)
    {
        evt.eventType = SystemEventType::MouseUp;
        evt.mouseX    = eSdl.button.x / charWidth;
        evt.mouseY    = eSdl.button.y / charHeight;
    }
    else if (eSdl.type == SDL_MOUSEMOTION)
    {
        evt.eventType = SystemEventType::MouseMove;
        evt.mouseX    = eSdl.motion.x / charWidth;
        evt.mouseY    = eSdl.motion.y / charHeight;
    }
}

void SDLTerminal::handleKeyUp(SystemEvent& evt, const SDL_Event& eSdl)
{
    const SDL_Keymod keyModifiers = static_cast<SDL_Keymod>(eSdl.key.keysym.mod);
    auto currentShiftState        = getShiftState(keyModifiers);
    if (currentShiftState != oldShiftState)
    {
        evt.eventType = SystemEventType::ShiftStateChanged;
        evt.keyCode   = currentShiftState;
    }
    oldShiftState = currentShiftState;
}

void SDLTerminal::handleKeyDown(SystemEvent& evt, const SDL_Event& eSdl)
{
    evt.eventType = SystemEventType::KeyPressed;

    const SDL_Keymod keyModifiers = static_cast<SDL_Keymod>(eSdl.key.keysym.mod);
    const SDL_Scancode virtualKey = eSdl.key.keysym.scancode;
    const SDL_Keycode keyCode     = eSdl.key.keysym.sym;

    if (KeyTranslation.count(virtualKey))
    {
        evt.keyCode = KeyTranslation[virtualKey];
    }

    if (keyCode >= 32 && keyCode <= 127)
    {
        evt.unicodeCharacter = keyCode;
    }

    auto currentShiftState = getShiftState(keyModifiers);

    if (keyModifiers & (KMOD_ALT | KMOD_CTRL))
    {
        evt.unicodeCharacter = 0;
    }
    if (keyModifiers & KMOD_SHIFT)
    {
        if (islower(evt.unicodeCharacter))
        {
            evt.unicodeCharacter = toupper(evt.unicodeCharacter);
        }
    }

    if (evt.keyCode == Key::None)
    {
        if (currentShiftState != oldShiftState)
        {
            evt.eventType = SystemEventType::ShiftStateChanged;
        }
    }

    evt.keyCode |= currentShiftState;
    oldShiftState = currentShiftState;
}

void SDLTerminal::GetSystemEvent(AppCUI::Internal::SystemEvent& evnt)
{
    evnt.eventType = SystemEventType::None;
    evnt.keyCode   = Key::None;
    evnt.unicodeCharacter = 0;

    SDL_Event e;
    // wait 30 ms max for the next event
    if (!SDL_WaitEventTimeout(&e, 30))
    {
        return;
    }

    switch (e.type)
    {
    case SDL_QUIT:
        evnt.eventType = SystemEventType::AppClosed;
        break;
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        handleMouse(evnt, e);
        break;
    case SDL_KEYDOWN:
        handleKeyDown(evnt, e);
        break;
    case SDL_KEYUP:
        handleKeyUp(evnt, e);
        break;
    case SDL_WINDOWEVENT:
        if (e.window.windowID == windowID)
        {
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                evnt.eventType = SystemEventType::AppResized;
                evnt.newWidth  = e.window.data1 / charWidth;
                evnt.newHeight = e.window.data2 / charHeight;
            }
            else if (e.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                e.type = SDL_QUIT;
                SDL_PushEvent(&e);
            }
        }
        break;
    case SDL_MOUSEWHEEL:
        evnt.eventType = SystemEventType::MouseWheel;
        if (e.wheel.y < 0)
        {
            evnt.mouseWheel = AppCUI::Input::MouseWheel::Up;
        }
        else if (e.wheel.y > 0)
        {
            evnt.mouseWheel = AppCUI::Input::MouseWheel::Down;
        }
        else if (e.wheel.x > 0)
        {
            evnt.mouseWheel = AppCUI::Input::MouseWheel::Right;
        }
        else if (e.wheel.x < 0)
        {
            evnt.mouseWheel = AppCUI::Input::MouseWheel::Left;
        }
        break;
    default:
        break;
    }
}

bool SDLTerminal::IsEventAvailable()
{
    NOT_IMPLEMENTED(false);
}

void SDLTerminal::uninitInput()
{
}
