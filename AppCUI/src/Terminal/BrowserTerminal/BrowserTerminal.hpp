#pragma once

#include "../../Internal.hpp"

#ifndef __EMSCRIPTEN__
#    error "BrowserTerminal can only be built for wasm32-emscripten"
#endif

#include <type_traits>

namespace AppCUI::Internal
{
constexpr uint32 BROWSER_ABI_MAGIC       = 0x49554341; // "ACUI" read as a little endian uint32
constexpr uint32 BROWSER_ABI_VERSION     = 1;          // bump on ANY change of the structures / exports below
constexpr uint32 BROWSER_MAX_COLUMNS     = 1024;       // 1024 x 512 cells x 4 bytes = 2 MB per canvas
constexpr uint32 BROWSER_MAX_ROWS        = 512;
constexpr uint32 BROWSER_DEFAULT_COLUMNS = 80;
constexpr uint32 BROWSER_DEFAULT_ROWS    = 25;

// Shared ABI with the JavaScript host (appcui-host.js): 16 little endian uint32 words, read through
// HEAPU32[(ptr >>> 2) + i]. Lives in static storage -> stable address for the lifetime of the module.
struct BrowserFrameHeader
{
    uint32 magic;         // BROWSER_ABI_MAGIC while a BrowserTerminal is alive, 0 otherwise
    uint32 abiVersion;    // BROWSER_ABI_VERSION
    uint32 frameSeq;      // +1 after every flush (wraps; JS only compares for equality)
    uint32 layoutSeq;     // +1 whenever cellsPtr / width / height change (JS must rebind its views)
    uint32 cellsPtr;      // byte address of Character[0] (4-byte aligned)
    uint32 width;         // columns
    uint32 height;        // rows
    uint32 cursorX;
    uint32 cursorY;
    uint32 cursorVisible; // 0 / 1
    uint32 droppedEvents; // input events rejected because the queue was full (diagnostics)
    uint32 reserved[5];
};
static_assert(sizeof(BrowserFrameHeader) == 64, "BrowserFrameHeader is a JS ABI (16 x uint32)");
static_assert(std::is_standard_layout_v<BrowserFrameHeader>);
static_assert(std::is_trivially_copyable_v<BrowserFrameHeader>);
static_assert(sizeof(Graphics::Character) == sizeof(uint32), "a screen cell must be one uint32 for the JS renderer");
static_assert(sizeof(void*) == sizeof(uint32), "wasm32 only (MEMORY64 is not supported by the browser ABI)");

struct BrowserInputEvent
{
    SystemEventType type;
    int32 x;
    int32 y;
    Input::Key key;
    Input::MouseButton button;
    Input::MouseWheel wheel;
    char16 unicodeChar;
};
static_assert(std::is_trivially_copyable_v<BrowserInputEvent>);

// Fixed capacity ring buffer (never allocates). Producer: JS event handlers through the exports; consumer:
// GetSystemEvent. Both run on the browser main thread and never interleave -> no atomics.
class BrowserInputQueue
{
  public:
    static constexpr uint32 CAPACITY = 256;
    static_assert((CAPACITY & (CAPACITY - 1)) == 0, "CAPACITY must be a power of two");

    BrowserInputQueue();
    bool Push(const BrowserInputEvent& e);
    bool Pop(BrowserInputEvent& e);
    inline bool IsEmpty() const
    {
        return head == tail;
    }
    inline uint32 GetDroppedCount() const
    {
        return dropped;
    }

  private:
    BrowserInputEvent items[CAPACITY];
    uint32 head;    // next slot to read  - free running (unsigned wrap is well defined)
    uint32 tail;    // next slot to write - free running; (tail - head) is always in [0, CAPACITY]
    uint32 dropped; // saturating
};

class BrowserTerminal : public AbstractTerminal
{
    BrowserInputQueue input;
    double lastFramesUpdateMs;
    uint32 pendingWidth, pendingHeight;
    uint32 suspendedWaits; // > 0 while a modal loop is suspended inside appcui_js_wait_for_input
    bool resizePending;
    bool closePending;
    bool redrawPending;
    bool fpsMode;

    bool Dequeue(SystemEvent& evnt);
    void StampFrameUpdate(SystemEvent& evnt);
    uint32 MillisecondsUntilNextFrame() const;
    void PublishLayout();

  public:
    BrowserTerminal();
    ~BrowserTerminal() override;

    // producer side (called by the extern "C" exports, after validation)
    bool PushEvent(const BrowserInputEvent& e);
    void RequestResize(uint32 width, uint32 height);
    void RequestClose();
    void RequestRedraw();
    bool HasPendingInput() const;
    inline bool IsSuspendedInModalLoop() const
    {
        return suspendedWaits != 0;
    }

    bool OnInit(const Application::InitializationData& initData) override;
    void RestoreOriginalConsoleSettings() override;
    void OnUnInit() override;
    void OnFlushToScreen() override;
    void OnFlushToScreen(const Graphics::Rect& r) override;
    bool OnUpdateCursor() override;
    void GetSystemEvent(Internal::SystemEvent& evnt) override;
    bool IsEventAvailable() override;
    bool HasSupportFor(Application::SpecialCharacterSetType type) override;
    bool RunRootEventLoop(ApplicationImpl& app, void (*shutdown)()) override; // BrowserMainLoop.cpp

    // clipboard: process-private (same policy as CustomTerminal)
    bool ClipboardClear() override;
    bool ClipboardHasText() override;
    bool ClipboardSetText(const ConstString& text) override;
    bool ClipboardGetText(Utils::UnicodeStringBuilder& text) override;
};

// the single live instance (AppCUI is a process-wide singleton); nullptr when there is none
BrowserTerminal* GetActiveBrowserTerminal();
BrowserFrameHeader& GetBrowserFrameHeader();
} // namespace AppCUI::Internal
