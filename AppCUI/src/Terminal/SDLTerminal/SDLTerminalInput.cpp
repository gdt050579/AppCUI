#include "SDLTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Internal;
using namespace Input;

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

bool SDLTerminal::InitInput(const Application::InitializationData&)
{
    oldShiftState = Key::None;

    for (size_t i = 0; i < 26; i++)
    {
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_A + i)] = static_cast<Key>(((uint32) Key::A) + i);
    }

    KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_0)] = static_cast<Key>((uint32) Key::N0);
    for (size_t i = 1; i < 10; i++)
    {
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_1 + i)] = static_cast<Key>(((uint32) Key::N1) + i);
    }

    for (size_t i = 0; i < 12; i++)
    {
        // F(x) + shift => F(12) + x
        KeyTranslation[static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + i)] = static_cast<Key>(((uint32) Key::F1) + i);
    }

    KeyTranslation[SDL_SCANCODE_RETURN]    = Key::Enter;
    KeyTranslation[SDL_SCANCODE_ESCAPE]    = Key::Escape;
    KeyTranslation[SDL_SCANCODE_BACKSPACE] = Key::Backspace;
    KeyTranslation[SDL_SCANCODE_TAB]       = Key::Tab;
    KeyTranslation[SDL_SCANCODE_SPACE]     = Key::Space;

    KeyTranslation[SDL_SCANCODE_INSERT]    = Key::Insert;
    KeyTranslation[SDL_SCANCODE_HOME]      = Key::Home;
    KeyTranslation[SDL_SCANCODE_PAGEUP]    = Key::PageUp;
    KeyTranslation[SDL_SCANCODE_DELETE]    = Key::Delete;
    KeyTranslation[SDL_SCANCODE_END]       = Key::End;

    KeyTranslation[SDL_SCANCODE_PAGEDOWN]  = Key::PageDown;
    KeyTranslation[SDL_SCANCODE_RIGHT]     = Key::Right;
    KeyTranslation[SDL_SCANCODE_LEFT]      = Key::Left;
    KeyTranslation[SDL_SCANCODE_DOWN]      = Key::Down;
    KeyTranslation[SDL_SCANCODE_UP]        = Key::Up;

    lastFramesUpdate = std::chrono::high_resolution_clock::now();
    return true;
}

void SDLTerminal::HandleMouse(SystemEvent& evt, const SDL_Event& eSdl)
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

void SDLTerminal::HandleKeyUp(SystemEvent& evt, const SDL_Event& eSdl)
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

void SDLTerminal::HandleKeyDown(SystemEvent& evt, const SDL_Event& eSdl)
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

void SDLTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    using namespace std::chrono_literals;

    evnt.eventType        = SystemEventType::None;
    evnt.keyCode          = Key::None;
    evnt.unicodeCharacter = 0;
    evnt.updateFrames     = false;

    SDL_Event e;
    // wait 33 ms max for the next event, equates to roughly 30 fps
    if (!SDL_WaitEventTimeout(&e, 33))
    {
        if (autoRedraw)
        {
            evnt.updateFrames = true;
        }
        return;
    }

    if (autoRedraw)
    {
        auto pollTime = std::chrono::high_resolution_clock::now();
        auto diffMS   = std::chrono::duration_cast<std::chrono::milliseconds>(pollTime - lastFramesUpdate);
        if (diffMS >= 33ms)
        {
            evnt.updateFrames = true;
            lastFramesUpdate  = pollTime;
        }
    }

    switch (e.type)
    {
    case SDL_QUIT:
        evnt.eventType = SystemEventType::AppClosed;
        break;
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        HandleMouse(evnt, e);
        break;
    case SDL_KEYDOWN:
        HandleKeyDown(evnt, e);
        break;
    case SDL_KEYUP:
        HandleKeyUp(evnt, e);
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
            evnt.mouseWheel = Input::MouseWheel::Up;
        }
        else if (e.wheel.y > 0)
        {
            evnt.mouseWheel = Input::MouseWheel::Down;
        }
        else if (e.wheel.x > 0)
        {
            evnt.mouseWheel = Input::MouseWheel::Right;
        }
        else if (e.wheel.x < 0)
        {
            evnt.mouseWheel = Input::MouseWheel::Left;
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

void SDLTerminal::UnInitInput()
{
}
}