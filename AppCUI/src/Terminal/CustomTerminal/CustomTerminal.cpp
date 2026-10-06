#include "CustomTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;

constexpr uint32 FRAME_INTERVAL_MS  = 33; // ~30 fps (same as the other frontends)
constexpr uint32 IDLE_WAIT_MS       = 1000;
constexpr uint32 MAX_FRONTEND_WIDTH = 0x7FFF;

CustomTerminal::CustomTerminal() : frontend(nullptr), fpsMode(false)
{
}
CustomTerminal::~CustomTerminal()
{
}

bool CustomTerminal::OnInit(const InitializationData& initData)
{
    CHECK(initData.CustomFrontend,
          false,
          "FrontendType::Custom requires a valid InitializationData::CustomFrontend object !");
    this->frontend = initData.CustomFrontend;

    uint32 width  = initData.Width;
    uint32 height = initData.Height;
    CHECK(this->frontend->OnInit(width, height), false, "Custom frontend failed to initialize !");
    CHECK((width > 0) && (height > 0) && (width <= MAX_FRONTEND_WIDTH) && (height <= MAX_FRONTEND_WIDTH),
          false,
          "Invalid size for a custom frontend (%u x %u)",
          width,
          height);
    CHECK(screenCanvas.Create(width, height), false, "Fail to create the screen canvas (%u x %u)", width, height);
    CHECK(originalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas (%u x %u)",
          width,
          height);

    this->fpsMode          = (initData.Flags & InitializationFlags::EnableFPSMode) != InitializationFlags::None;
    this->lastFramesUpdate = std::chrono::steady_clock::now();
    return true;
}
void CustomTerminal::RestoreOriginalConsoleSettings()
{
    // first step of AbstractTerminal::UnInit: a custom frontend has no "original console" to restore -> detach it now
    // so that it never receives the blank "restored" screen that UnInit flushes next
    OnUnInit();
}
void CustomTerminal::OnUnInit()
{
    if (this->frontend)
        this->frontend->OnUnInit();
    this->frontend = nullptr;
}
void CustomTerminal::OnFlushToScreen()
{
    if (this->frontend)
        this->frontend->OnFlushToScreen(
              screenCanvas.GetCharactersBuffer(), screenCanvas.GetWidth(), screenCanvas.GetHeight());
}
void CustomTerminal::OnFlushToScreen(const Graphics::Rect&)
{
    // the frontend always receives the entire screen (it decides what has changed)
    OnFlushToScreen();
}
bool CustomTerminal::OnUpdateCursor()
{
    if (this->frontend)
        this->frontend->OnUpdateCursor(
              screenCanvas.GetCursorX(), screenCanvas.GetCursorY(), screenCanvas.GetCursorVisibility());
    return true;
}
void CustomTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    evnt.eventType        = SystemEventType::None;
    evnt.keyCode          = Input::Key::None;
    evnt.unicodeCharacter = 0;
    evnt.updateFrames     = false;
    if (!this->frontend)
        return;

    uint32 timeoutMs = IDLE_WAIT_MS;
    if (this->fpsMode)
    {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - this->lastFramesUpdate)
                                   .count();
        timeoutMs = (elapsed >= FRAME_INTERVAL_MS) ? 0 : FRAME_INTERVAL_MS - static_cast<uint32>(elapsed);
    }

    FrontendEvent e;
    const bool hasEvent = this->frontend->WaitForEvent(e, timeoutMs);

    if (this->fpsMode)
    {
        const auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastFramesUpdate).count() >=
            FRAME_INTERVAL_MS)
        {
            evnt.updateFrames      = true;
            this->lastFramesUpdate = now;
        }
    }
    if (!hasEvent)
        return;

    switch (e.Type)
    {
    case FrontendEventType::KeyPressed:
        evnt.eventType        = SystemEventType::KeyPressed;
        evnt.keyCode          = e.Key;
        evnt.unicodeCharacter = e.UnicodeChar;
        break;
    case FrontendEventType::ShiftStateChanged:
        evnt.eventType = SystemEventType::ShiftStateChanged;
        evnt.keyCode   = e.Key;
        break;
    case FrontendEventType::MouseDown:
    case FrontendEventType::MouseUp:
    case FrontendEventType::MouseMove:
        evnt.eventType   = e.Type == FrontendEventType::MouseDown ? SystemEventType::MouseDown
                           : e.Type == FrontendEventType::MouseUp ? SystemEventType::MouseUp
                                                                  : SystemEventType::MouseMove;
        evnt.mouseX      = e.X;
        evnt.mouseY      = e.Y;
        evnt.mouseButton = e.Button;
        evnt.keyCode     = e.Key;
        break;
    case FrontendEventType::MouseWheel:
        evnt.eventType  = SystemEventType::MouseWheel;
        evnt.mouseX     = e.X;
        evnt.mouseY     = e.Y;
        evnt.mouseWheel = e.Wheel;
        evnt.keyCode    = e.Key;
        break;
    case FrontendEventType::Resized:
        if ((e.Width > 0) && (e.Height > 0) && (e.Width <= MAX_FRONTEND_WIDTH) && (e.Height <= MAX_FRONTEND_WIDTH))
        {
            evnt.eventType = SystemEventType::AppResized;
            evnt.newWidth  = e.Width;
            evnt.newHeight = e.Height;
        }
        break;
    case FrontendEventType::Closed:
        evnt.eventType = SystemEventType::AppClosed;
        break;
    case FrontendEventType::RedrawRequested:
        evnt.eventType = SystemEventType::RequestRedraw;
        break;
    default:
        break;
    }
}
bool CustomTerminal::IsEventAvailable()
{
    return false;
}
bool CustomTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    return this->frontend ? this->frontend->HasSupportFor(type) : false;
}
} // namespace AppCUI::Internal
