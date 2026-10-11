#include "BrowserTerminal.hpp"
#include "../TerminalFactory.hpp"
#include <emscripten/emscripten.h>
#include <string>

// C++ -> JS imports. Numeric arguments only (no marshalling, no allocation). uint32 values and pointers reach JS as
// signed i32 -> the JS side always normalises them with ">>> 0".
// clang-format off
EM_JS(uint32_t, appcui_js_initial_size, (), {
    const host = Module["appcui"];
    return host ? (host.initialSize() >>> 0) : 0;
});
EM_JS(void, appcui_js_present, (const void* header), {
    const host = Module["appcui"];
    if (host) host.present(header >>> 0);
});
EM_JS(void, appcui_js_on_exit, (), {
    const host = Module["appcui"];
    if (host) host.onExit();
});
// Suspends the wasm stack (Asyncify / JSPI) until JS input arrives or the timeout expires (idle path only).
EM_ASYNC_JS(void, appcui_js_wait_for_input, (uint32_t timeoutMs), {
    const host = Module["appcui"];
    if (host) await host.waitForInput(timeoutMs >>> 0);
    else await new Promise((resolve) => setTimeout(resolve, timeoutMs >>> 0));
});
// clang-format on

namespace AppCUI::Internal
{
using namespace Application;

constexpr uint32 FRAME_INTERVAL_MS = 33;   // ~30 fps, same cadence as SDLTerminal / CustomTerminal
constexpr uint32 IDLE_WAIT_MS      = 1000; // modal loop without FPS mode: wake up at least once per second

static BrowserFrameHeader frameHeader{}; // zero-initialised => magic == 0 until a terminal is alive
static BrowserTerminal* activeTerminal = nullptr;
static std::u16string browserClipboard;

BrowserFrameHeader& GetBrowserFrameHeader()
{
    return frameHeader;
}
BrowserTerminal* GetActiveBrowserTerminal()
{
    return activeTerminal;
}

//====================================================================================== BrowserInputQueue ===
BrowserInputQueue::BrowserInputQueue() : items{}, head(0), tail(0), dropped(0)
{
}
bool BrowserInputQueue::Push(const BrowserInputEvent& e)
{
    // consecutive mouse moves are coalesced (the latest position wins) -> the queue stays short while dragging
    if ((e.type == SystemEventType::MouseMove) && (tail != head))
    {
        BrowserInputEvent& last = items[(tail - 1u) & (CAPACITY - 1u)];
        if ((last.type == SystemEventType::MouseMove) && (last.button == e.button) && (last.key == e.key))
        {
            last.x = e.x;
            last.y = e.y;
            return true;
        }
    }
    if ((tail - head) >= CAPACITY)
    {
        if (dropped != 0xFFFFFFFFu)
            dropped++;
        return false;
    }
    items[tail & (CAPACITY - 1u)] = e;
    tail++;
    return true;
}
bool BrowserInputQueue::Pop(BrowserInputEvent& e)
{
    if (head == tail)
        return false;
    e = items[head & (CAPACITY - 1u)];
    head++;
    return true;
}

//======================================================================================== BrowserTerminal ===
BrowserTerminal::BrowserTerminal()
    : lastFramesUpdateMs(0.0), pendingWidth(0), pendingHeight(0), suspendedWaits(0), resizePending(false),
      closePending(false), redrawPending(false), fpsMode(false)
{
}
BrowserTerminal::~BrowserTerminal()
{
    if (activeTerminal == this)
    {
        activeTerminal    = nullptr;
        frameHeader.magic = 0;
    }
}

static uint32 ClampDimension(uint32 value, uint32 maxValue, uint32 fallback)
{
    if (value == 0)
        return fallback;
    return value > maxValue ? maxValue : value;
}

bool BrowserTerminal::OnInit(const InitializationData& initData)
{
    CHECK(activeTerminal == nullptr, false, "Only one BrowserTerminal can be active at a time !");

    // the JS renderer reads a cell as one little endian uint32: code | foreground << 16 | background << 24
    Graphics::Character probe{};
    probe.Code             = 0x1234;
    probe.Color.Foreground = Graphics::Color::Magenta;
    probe.Color.Background = Graphics::Color::Green;
    CHECK(probe.PackedValue == 0x0A051234u, false, "Unexpected Character memory layout (0x%08X)", probe.PackedValue);

    uint32 width  = initData.Width;
    uint32 height = initData.Height;
    if ((width == 0) || (height == 0))
    {
        const uint32 packed = appcui_js_initial_size(); // columns | rows << 16 (0 when no host is attached)
        width               = packed & 0xFFFFu;
        height              = packed >> 16;
    }
    width  = ClampDimension(width, BROWSER_MAX_COLUMNS, BROWSER_DEFAULT_COLUMNS);
    height = ClampDimension(height, BROWSER_MAX_ROWS, BROWSER_DEFAULT_ROWS);

    CHECK(screenCanvas.Create(width, height), false, "Fail to create the screen canvas (%u x %u)", width, height);
    CHECK(originalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas (%u x %u)",
          width,
          height);

    this->fpsMode            = (initData.Flags & InitializationFlags::EnableFPSMode) != InitializationFlags::None;
    this->lastFramesUpdateMs = emscripten_get_now();

    frameHeader            = BrowserFrameHeader{};
    frameHeader.abiVersion = BROWSER_ABI_VERSION;
    PublishLayout();
    frameHeader.magic = BROWSER_ABI_MAGIC; // published last: JS ignores the header until the magic is valid
    activeTerminal    = this;
    return true;
}
void BrowserTerminal::RestoreOriginalConsoleSettings()
{
    // a web page has no "original console": detach first so that the blank "restored" screen flushed next by
    // AbstractTerminal::UnInit is never shown (same policy as CustomTerminal)
    OnUnInit();
}
void BrowserTerminal::OnUnInit()
{
    if (activeTerminal != this)
        return;
    activeTerminal    = nullptr;
    frameHeader.magic = 0;
    browserClipboard.clear();
    browserClipboard.shrink_to_fit(); // a later Application::Init must not inherit the clipboard
    appcui_js_on_exit();
}
void BrowserTerminal::PublishLayout()
{
    const uint32 cellsPtr = static_cast<uint32>(reinterpret_cast<uintptr_t>(screenCanvas.GetCharactersBuffer()));
    const uint32 width    = screenCanvas.GetWidth();
    const uint32 height   = screenCanvas.GetHeight();
    if ((cellsPtr != frameHeader.cellsPtr) || (width != frameHeader.width) || (height != frameHeader.height))
    {
        frameHeader.cellsPtr = cellsPtr;
        frameHeader.width    = width;
        frameHeader.height   = height;
        frameHeader.layoutSeq++;
    }
    frameHeader.droppedEvents = input.GetDroppedCount();
}
void BrowserTerminal::OnFlushToScreen()
{
    if (activeTerminal != this)
        return; // detached (UnInit in progress)
    PublishLayout(); // Canvas::Resize reallocates the cells -> re-publish the pointer before every present
    frameHeader.frameSeq++;
    appcui_js_present(&frameHeader);
}
void BrowserTerminal::OnFlushToScreen(const Graphics::Rect&)
{
    OnFlushToScreen(); // the JS renderer diffs the cells itself
}
bool BrowserTerminal::OnUpdateCursor()
{
    frameHeader.cursorX       = screenCanvas.GetCursorX();
    frameHeader.cursorY       = screenCanvas.GetCursorY();
    frameHeader.cursorVisible = screenCanvas.GetCursorVisibility() ? 1u : 0u;
    return true;
}

bool BrowserTerminal::PushEvent(const BrowserInputEvent& e)
{
    return input.Push(e);
}
void BrowserTerminal::RequestResize(uint32 width, uint32 height)
{
    // never queued -> never dropped; the latest size wins
    pendingWidth  = width;
    pendingHeight = height;
    resizePending = true;
}
void BrowserTerminal::RequestClose()
{
    closePending = true;
}
void BrowserTerminal::RequestRedraw()
{
    redrawPending = true;
}
bool BrowserTerminal::HasPendingInput() const
{
    return closePending || resizePending || redrawPending || !input.IsEmpty();
}

bool BrowserTerminal::Dequeue(SystemEvent& evnt)
{
    if (closePending)
    {
        closePending   = false;
        evnt.eventType = SystemEventType::AppClosed;
        return true;
    }
    if (resizePending)
    {
        resizePending  = false;
        evnt.eventType = SystemEventType::AppResized;
        evnt.newWidth  = pendingWidth;
        evnt.newHeight = pendingHeight;
        return true;
    }
    if (redrawPending)
    {
        redrawPending  = false;
        evnt.eventType = SystemEventType::RequestRedraw;
        return true;
    }
    BrowserInputEvent e;
    if (!input.Pop(e))
        return false;
    evnt.eventType        = e.type;
    evnt.mouseX           = e.x;
    evnt.mouseY           = e.y;
    evnt.mouseButton      = e.button;
    evnt.mouseWheel       = e.wheel;
    evnt.keyCode          = e.key;
    evnt.unicodeCharacter = e.unicodeChar;
    return true;
}
uint32 BrowserTerminal::MillisecondsUntilNextFrame() const
{
    const double elapsed = emscripten_get_now() - this->lastFramesUpdateMs;
    if (!(elapsed >= 0.0)) // also catches NaN
        return FRAME_INTERVAL_MS;
    if (elapsed >= static_cast<double>(FRAME_INTERVAL_MS))
        return 0;
    return FRAME_INTERVAL_MS - static_cast<uint32>(elapsed); // elapsed is in [0, 33) -> no overflow
}
void BrowserTerminal::StampFrameUpdate(SystemEvent& evnt)
{
    if (!this->fpsMode)
        return;
    const double now = emscripten_get_now();
    if ((now - this->lastFramesUpdateMs) >= static_cast<double>(FRAME_INTERVAL_MS))
    {
        evnt.updateFrames        = true;
        this->lastFramesUpdateMs = now;
    }
}
void BrowserTerminal::GetSystemEvent(Internal::SystemEvent& evnt)
{
    evnt.eventType        = SystemEventType::None;
    evnt.keyCode          = Input::Key::None;
    evnt.unicodeCharacter = 0;
    evnt.updateFrames     = false;

    for (;;)
    {
        const bool hasEvent = Dequeue(evnt);
        StampFrameUpdate(evnt);
        if (hasEvent || evnt.updateFrames || this->nonBlockingEvents)
            return;
        // Modal (nested) loop with nothing to do: hand the thread back to the browser. The C++ stack - including
        // the dialog objects that live on it - is suspended here and resumed when JS resolves the wait. There is
        // no lost wake-up: JS handlers cannot run between the Dequeue above and this call (single thread).
        this->suspendedWaits++;
        appcui_js_wait_for_input(this->fpsMode ? MillisecondsUntilNextFrame() : IDLE_WAIT_MS);
        this->suspendedWaits--;
    }
}
bool BrowserTerminal::IsEventAvailable()
{
    return HasPendingInput();
}
bool BrowserTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    // the page renders with JuliaMono (resources/JuliaMono-Regular.ttf) -> full box drawing support
    return type != Application::SpecialCharacterSetType::Auto;
}

bool BrowserTerminal::ClipboardClear()
{
    browserClipboard.clear();
    return true;
}
bool BrowserTerminal::ClipboardHasText()
{
    return !browserClipboard.empty();
}
bool BrowserTerminal::ClipboardSetText(const ConstString& text)
{
    Utils::UnicodeStringBuilder unicode;
    CHECK(unicode.Set(text), false, "Fail to convert ConstString into unicode buffer !");
    browserClipboard.assign(unicode.ToStringView());
    return true;
}
bool BrowserTerminal::ClipboardGetText(Utils::UnicodeStringBuilder& text)
{
    return text.Set(std::u16string_view(browserClipboard));
}

// AppCUI::Frontend::Browser -> FrontendType::Browser, and FrontendType::Default (the only native frontend of a
// wasm32-emscripten build)
static const TerminalRegistrar browserTerminalRegistrar(
      Application::FrontendType::Browser, CreateTerminal<BrowserTerminal>, DefaultFrontendPriority::Native);
} // namespace AppCUI::Internal
