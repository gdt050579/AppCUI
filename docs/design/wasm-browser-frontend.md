# AppCUI on WebAssembly — Browser Frontend Design & Implementation Blueprint

| | |
|---|---|
| **Target** | `wasm32-emscripten`, evergreen browsers (Chromium, Firefox, Safari) |
| **Status** | Implemented and verified (Emscripten 6.0.12). Section 0.1 lists where the implementation differs from this blueprint and why |
| **Builds on** | `f7604518`: *Split libAppCUI into AppCUI::Core + one CMake component per frontend* (frontend registry, clipboard owned by the terminal) |
| **Scope** | New `BrowserTerminal` backend, JS ABI, HTML5 canvas renderer, event-loop adaptation, build |
| **Out of scope (v1)** | OS clipboard bridge, PNG/JPG decoding, file-system persistence, multi-threading |
| **Non-negotiables** | No new warnings (`-Wall -Wextra` plus the project's extra flags) · no allocations on hot paths (C++ *and* JS) · strict integer safety at every JS→C++ boundary |

---

## 0. TL;DR — the decisions

| # | Decision | Why (code evidence) |
|---|---|---|
| D1 | New **internal** backend `BrowserTerminal : AbstractTerminal`, shipped as its own component **`AppCUI::Frontend::Browser`**, plus a new public `FrontendType::Browser = 6`. It registers itself through the existing `TerminalRegistrar` with `DefaultFrontendPriority::Native`, so `FrontendType::Default` resolves to it in a browser build without any `#ifdef` in the core. It is **not** built on `CustomFrontendInterface`. | `CustomFrontendInterface::WaitForEvent(evnt, timeoutMs)` is blocking by contract ([AppCUI.hpp:5635](../../AppCUI/include/AppCUI.hpp)). The browser needs a *polled* mode and a *yielding* mode, and only an internal backend can see the event-loop frame. Shipping it as a component follows the target layout introduced by `f7604518` ([AppCUI/CMakeLists.txt](../../AppCUI/CMakeLists.txt), [TerminalFactory.hpp](../../AppCUI/src/Terminal/TerminalFactory.hpp)). |
| D2 | **C++→JS:** `EM_JS` with numeric arguments only. **JS→C++:** `extern "C" EMSCRIPTEN_KEEPALIVE` functions with numeric arguments only. **Screen:** a fixed 64-byte `BrowserFrameHeader` in static storage, which JS reads with `HEAPU32`. **No embind / `emscripten::val`** anywhere. | Every `emscripten::val` is a refcounted handle in the JS emval table, so it allocates on each call. `EM_ASM` marshals varargs through an argument array. `EM_JS` is a direct wasm import with no marshalling. |
| D3 | The renderer diffs against a **JS-side shadow copy** of the 32-bit cells and draws background runs plus per-cell `fillText(…, maxWidth)` in **integer device pixels**. Block and light-line glyphs are drawn procedurally so there are no seams. | `AbstractTerminal::Update()` always calls the **full** `OnFlushToScreen()` and never passes a rect ([AbstractTerminal.cpp](../../AppCUI/src/Application/AbstractTerminal.cpp)). Diffing 4-byte cells in JS costs about 10 µs for a 200×60 screen. |
| D4 | Split `ExecuteEventLoop` into **Enter / Step / Leave**. The native loop stays `while (Step())` and behaves exactly as it does today. A new virtual `AbstractTerminal::RunRootEventLoop` (default: "not handled") lets the Browser component take over the **root** loop, which runs via `emscripten_set_main_loop` on requestAnimationFrame. The core never names the browser. **Nested (modal) loops** yield through `EM_ASYNC_JS` + **Asyncify**. | `emscripten_set_main_loop` **alone cannot work**. `Window::Show()` re-enters `ExecuteEventLoop` synchronously ([Window.cpp:1541](../../AppCUI/src/Controls/Window.cpp)), dialogs live on the C++ stack (`FileDialogWindow dlg` at [FileDialog.cpp:609](../../AppCUI/src/Dialogs/FileDialog.cpp)), and modals run *before* `Application::Run()` ([Examples/MessageBox/main.cpp](../../Examples/MessageBox/main.cpp)). Those stacks have to survive while the browser runs, and only stack switching (Asyncify/JSPI) or a worker can do that. |
| D5 | **Asyncify (`-sASYNCIFY=1`)** is the default. JSPI is an opt-in build flag, used only after it passes milestone M1. | Emscripten's Asyncify `handleSleep` explicitly **pauses `MainLoop` during a sleep and resumes it afterwards**. Its JSPI `handleSleep` does not touch `MainLoop` ([libasync.js](https://github.com/emscripten-core/emscripten/blob/main/src/lib/libasync.js)). That pause/resume is the property D4 relies on. |
| D6 | The Emscripten build reuses the component layout: `AppCUI::Core` + `AppCUI::Frontend::Browser`, with the SDL/ncurses components switched off, `libAppCUI` built **static** and linked whole-archive, and no vcpkg. Only three Emscripten-specific branches remain: component defaults, the core's image codecs, and `stdc++fs`/`dl`. | After the split, SDL, FreeType, Brotli, BZip2, X11 and ncurses are already confined to their components. Under `emcmake`, CMake sets `UNIX=1`, so the Ncurses component would default **ON** and has to be defaulted OFF. The core still `find_package`s PNG/JPEG as `REQUIRED`. The static registrars must not be dropped by the archive linker, which is the same problem the split avoided by using OBJECT libraries. |

### 0.1 As built: deviations from the blueprint, and why

Implementing and testing the design in a real browser forced the following changes. Where they conflict, the code
is authoritative over the rest of this document.

| Area | Blueprint | As built | Why |
|---|---|---|---|
| Root loop cadence | `emscripten_set_main_loop(…, 0)` on requestAnimationFrame | `emscripten_set_main_loop(…, 60)`, a 16 ms `setTimeout` cadence ([BrowserMainLoop.cpp](../../AppCUI/src/Terminal/BrowserTerminal/BrowserMainLoop.cpp)) | rAF stops whenever the page is not composited (occluded or minimized windows, embedders, automation hosts) while input keeps arriving, which froze the app in testing. Painting never needed rAF: the host draws from a microtask after each flush. |
| Exception model (Asyncify) | `-fwasm-exceptions` | `-fexceptions` (JS-based) | emcc warns *"ASYNCIFY=1 is not compatible with -fwasm-exceptions"*, which breaks the zero-warnings rule. Binaryen's Asyncify cannot unwind through native wasm EH. The JSPI build keeps `-fwasm-exceptions`. |
| JSPI root loop | Main loop plus a re-entry guard (§6.6) | The root loop **blocks** (`RunRootEventLoop` returns false), so every idle wait suspends `main()` | An emscripten main-loop callback is not a `promising` export, and in testing a modal opened from a tick threw `SuspendError: trying to suspend without WebAssembly.promising`. Under JSPI the whole app runs inside `main()`, so blocking is both correct and simpler, and `Application::Run()` returns normally. |
| `startAppCUI` readiness | `await createModule(...)` | Resolves on `onRuntimeInitialized` (after `attach`) | Under JSPI the module promise waits for `main()`, which can be suspended inside a modal shown before `Run()`. |
| Host language | TypeScript | Plain ES module with `// @ts-check` and JSDoc types ([AppCUI/web/appcui-host.js](../../AppCUI/web/appcui-host.js)) | No TypeScript toolchain gets added to a C++ repository. It is still type-checkable. |
| Box drawing | Light lines and blocks procedural; double lines from the font | Double lines (`═ ║ ╔ ╗ ╚ ╝ ╠ ╣ ╦ ╩ ╬`) are procedural too | Font double lines showed seams at cell edges, and every AppCUI window border uses them. |
| Running the examples | "Serve the folder over HTTP" | `bin/wasm/serve.py <Example>` (copied next to every example) serves with explicit MIME types and opens the browser. A page opened from `file://` shows how to run it | Browsers refuse ES modules and wasm from `file://`. On Windows, `python -m http.server` serves `.mjs` as `text/plain` (the registry overrides `mimetypes`), which browsers also refuse. Single-file builds (base64 wasm plus an embedded font, ~5 MB per example, no streaming compile) were rejected. |
| Control code points | Rendered with the font | C0 controls, DEL and C1 controls are mapped to CP437 symbols (same table as the SDL frontend) | The font renders them as tofu boxes (visible in `CharacterTable`). |

Core fixes that the browser work surfaced (all native-safe, all verified):

1. **`std::char_traits<Graphics::Character>`** ([AppCUI.hpp](../../AppCUI/include/AppCUI.hpp)). libc++ 19+ removed the generic `char_traits<T>` that `Utils::CharacterView` relied on, so every core file failed to compile under Emscripten, and a recent Apple/LLVM libc++ would fail the same way. The specialization keeps the old semantics (compare by `Code`) and is ABI-neutral: same mangling, same `string_view` layout.
2. **`UpdateCommandBar()` never cleared `cmdBarUpdate` when the app has no command bar.** Every event-loop iteration of such apps forced a full repaint: one per event natively, and a busy repaint loop (~2,200 flushes/s) in the polled browser loop. After the fix: 0 flushes/s when idle. The screen hashes are identical before and after (below).
3. **`Whereami.cpp`** had no Emscripten branch (`#error unsupported platform`). It now reports a virtual `/app.wasm` at the MEMFS root.
4. **`PlatformFolders.cpp`.** Emscripten runs as uid 0, so `$HOME` was ignored and `getpwuid_r` threw an uncaught exception that killed `FileDialog`. It also dereferenced a null `passwd*` when no entry exists (UB on native too); that is fixed as well.

**Measured results**

| Check | Result |
|---|---|
| Native M0 gate: `TestScript` hashes | Identical to the parent commit `f7604518`, including its existing failure on the second hash |
| Native M0 gate: nested modal scenario (depth 1 and 2, with and without a command bar) | 7/7 checkpoints identical to the parent commit, in both variants |
| Native full build (all examples, Tests and docs snippets, MSVC Debug) | 0 errors; no new warnings |
| wasm build, all 66 examples | 0 errors; the Browser component builds with `-Werror`; no warning in any touched file |
| `MessageBox.wasm` size | 717 KB (Asyncify), 394 KB (JSPI) |
| M1: modals before `Run()`, modals from a root tick, close and resume, app shutdown | Pass (Asyncify and JSPI) |
| M1: `FileDialog` (stack-allocated window, MEMFS browsing, double-click navigation, cancel result) | Pass |
| Idle cost | 0 flushes/s at the root and inside an open modal |
| Live resize | Pass: buffer reallocated, `layoutSeq` bumped, host rebound |
| Input: text (ASCII and BMP non-ASCII), cursor, right-click context menu, wheel | Pass |
| Export fuzz: 100,000 calls with random 32-bit values, NaN, ±Infinity, 2^32+n | 0 traps; all 13 targeted invalid inputs rejected; app alive |
| FPS mode | Animates (Snake, Tetris). The exact ~30/s cadence could not be confirmed: the test pane throttled `setTimeout(16)` to 36 ticks/s |

Not verified: DPR ≠ 1 rendering, Firefox/Safari, ResizeObserver delivery (it does not fire in a non-rendering pane, so the resize was triggered directly), and the OS-level keyboard path (key events were dispatched as DOM events through the host).

---

## 1. Facts from the codebase that constrain the design

1. **The `AbstractTerminal` contract** ([Internal.hpp:209](../../AppCUI/src/Internal.hpp)): `OnInit`, `RestoreOriginalConsoleSettings`, `OnUnInit`, `OnFlushToScreen()`, `OnFlushToScreen(Rect)`, `OnUpdateCursor`, `GetSystemEvent`, `IsEventAvailable`, `HasSupportFor`. `GetSystemEvent` is expected to *block* (SDL waits up to 33 ms, `CustomTerminal` waits up to 1000 ms).
2. **Screen memory layout.** `Graphics::Character` is `union { struct { char16 Code; ColorPair Color; }; uint32 PackedValue; }` and `ColorPair` is `{ uint8 Foreground; uint8 Background; }`. wasm32 is little-endian, so each cell is one `uint32`: `code | fg << 16 | bg << 24`. `Color` values are 0…15, and `Transparent = 0x10` can appear in theory.
3. **The screen buffer moves.** `Canvas::Resize` allocates a new `Character[]` and frees the old one ([Canvas.cpp:48](../../AppCUI/src/Graphics/Canvas.cpp)). JS must therefore **never cache the cells pointer across a layout change**. Resizing is a cold path, so the allocation there is allowed.
4. **The event loop** ([Application.cpp:1286](../../AppCUI/src/Application/Application.cpp)) has three parts: a prologue (modal-stack push, command bar), a `while (loopStatus == Normal)` body (deferred deletes → command bar → paint → `terminal->Update()` → `GetSystemEvent` → dispatch), and an epilogue (modal pop, `StopCurrent` → `Normal`, `CheckIfAppShouldClose`). It is **re-entrant**: every modal window runs a nested copy of it.
5. **The teardown lives inside `Run()`.** `Application::Run()` and `RunSingleApp()` call `UnInit` and `delete app` *after* the loop returns. In the browser that code has to move into the last main-loop tick.
6. **Already removed by the split (`f7604518`):**
   - `Clipboard.cpp` no longer includes SDL. The clipboard is now four virtuals on `AbstractTerminal` (`ClipboardClear/HasText/SetText/GetText`).
   - The per-OS factories (`Unix.cpp` / `Windows.cpp`, which hard-included ncurses and SDL headers) were replaced by a registry ([Factory.cpp](../../AppCUI/src/Terminal/Factory/Factory.cpp)) that components fill through `TerminalRegistrar`.
   - SDL2, SDL2_ttf, FreeType, Brotli, BZip2, X11 and the embedded font now belong to `AppCUI::Frontend::SDL` only.
7. **Remaining build blockers:**
   - `AppCUI::Core` still `find_package`s PNG, JPEG and libjpeg-turbo as `REQUIRED`: `JPGLoader.cpp` needs `turbojpeg.h` and `PNGLoader.cpp` needs `libpng16/png.h`.
   - The core links `stdc++fs` and `${CMAKE_DL_LIBS}` under `UNIX AND NOT APPLE`, which is true under Emscripten.
   - `APPCUI_FRONTEND_NCURSES` defaults to ON under `UNIX`, and `APPCUI_FRONTEND_SDL` defaults to ON everywhere.
   - `libAppCUI` is `SHARED`; Emscripten needs `STATIC`.
   - The registry has `FRONTEND_TYPES_COUNT = Custom + 1`, and `Properties.cpp` accepts frontend values only up to `Custom`.
   - There are 16 `catch` sites (FileDialog, DateTime, MemoryFile, …), so exceptions must be enabled.
8. **Existing hooks to reuse:**
   - The process-private clipboard implementation in `CustomTerminal` ([CustomTerminal.cpp](../../AppCUI/src/Terminal/CustomTerminal/CustomTerminal.cpp)), which `BrowserTerminal` mirrors for v1.
   - The `[AppCUI] Keyboard.Ctrl / Keyboard.Alt` modifier profile (`LoadKeyboardSettings`), which can work around browser-reserved shortcuts such as Ctrl+W/T/N.

---

## 2. Architecture

```
 ┌──────────────────────────── Browser main thread ───────────────────────────────┐
 │                                                                                │
 │  DOM events ──► AppCUIHost (TS) ──► _appcui_browser_key/mouse/wheel/resize(…)  │
 │  (keydown, pointer*, wheel,           numbers only, validated in C++           │
 │   ResizeObserver, dpr change)                │                                 │
 │                                              ▼                                 │
 │                    ┌──────────── wasm linear memory ────────────────┐          │
 │                    │ BrowserInputQueue  (256 POD slots, ring)       │          │
 │                    │ g_frameHeader      (16 × uint32, static)       │◄── JS reads
 │                    │ screenCanvas cells (w*h × uint32, moves on     │    HEAPU32
 │                    │                     resize → layoutSeq++)      │          │
 │                    └────────────────────────────────────────────────┘          │
 │                                              ▲                                 │
 │  rAF ─► MainLoop ─► BrowserMainLoopTick ─► StepEventLoop(rootFrame) ×N         │
 │                                              │  paint → Update() →             │
 │                                              │  OnFlushToScreen → appcui_js_present ─► queueMicrotask(draw)
 │                                              │                                 │
 │                              Window::Show() ─┴─► ExecuteEventLoop (nested)     │
 │                                                  GetSystemEvent: queue empty → │
 │                                                  EM_ASYNC_JS wait (Asyncify    │
 │                                                  unwinds; MainLoop paused)     │
 └────────────────────────────────────────────────────────────────────────────────┘
```

**Threading model:** everything runs on one thread. "Producer/consumer" here means JS event handlers and wasm code, which never run at the same time, so no atomics are needed.

---

## 3. Part 1 — `BrowserTerminal`

### 3.1 Public API change (`AppCUI/include/AppCUI.hpp`)

```cpp
    enum class FrontendType : uint32
    {
        Default        = 0,
        SDL            = 1,
        Terminal       = 2,
        WindowsConsole = 3,
        Tests          = 4,
        Custom         = 5, // implemented by the application (see CustomFrontendInterface)
        Browser        = 6, // HTML5 canvas inside a web page (wasm32-emscripten builds only)
    };
```

No `#ifdef __EMSCRIPTEN__` is needed in the core. `FrontendType::Default` is resolved by the registry ([Factory.cpp](../../AppCUI/src/Terminal/Factory/Factory.cpp), `FindDefaultFrontend`) to the highest-priority *linked* frontend. A browser build links only the Browser component, which registers as `Native`. Three small core edits follow from the new enum value:

```cpp
// Terminal/Factory/Factory.cpp - the registry grows by one slot
constexpr uint32 FRONTEND_TYPES_COUNT = static_cast<uint32>(FrontendType::Browser) + 1;
static_assert(static_cast<uint32>(FrontendType::Browser) == 6);
TerminalEntry registeredTerminals[FRONTEND_TYPES_COUNT] = {
    /* ...existing six entries unchanged... */
    { nullptr, DefaultFrontendPriority::None },  // Browser (AppCUI::Frontend::Browser)
};
// names[] gains "Browser"

// Application/Properties.cpp - accept and list the new value
if (frontEndValue > (uint32) Application::FrontendType::Browser)        // was: Custom
"Default=0,SDL=1,Terminal=2,WindowsConsole=3,Tests=4,Custom=5,Browser=6"
```

**Guard:** add `static_assert(FRONTEND_TYPES_COUNT == static_cast<uint32>(FrontendType::Browser) + 1)` next to the enum's last value, so a future `FrontendType` cannot silently fall outside the registry. The `Properties.cpp` limit already lagged once: it rejected `Custom` until the split fixed it.

### 3.2 `AbstractTerminal` addition (`Internal.hpp`)

```cpp
        // true -> GetSystemEvent must return immediately (SystemEventType::None when nothing is queued).
        // The event loop sets it before every GetSystemEvent call; only frontends driven by an external
        // loop (BrowserTerminal) read it - every other frontend keeps blocking.
        bool nonBlockingEvents = false;
```

A default member initializer leaves the existing constructor initializer list untouched, so it adds no `-Wreorder` noise.

The second addition is the root-loop hook (§6.3). It keeps `Application::Run()` free of any browser knowledge:

```cpp
    struct ApplicationImpl; // forward declaration (defined below AbstractTerminal in Internal.hpp)

        // A frontend that must not block the thread it runs on (BrowserTerminal) drives the ROOT event loop itself.
        // Returns false (default) -> the caller runs the classic blocking loop. A frontend that takes over calls
        // `shutdown` when the loop ends and may never return to the caller.
        virtual bool RunRootEventLoop(ApplicationImpl& app, void (*shutdown)());
```

```cpp
// Application/AbstractTerminal.cpp
bool AbstractTerminal::RunRootEventLoop(ApplicationImpl&, void (*)())
{
    return false;
}
```

On native builds this adds one virtual call per `Run()`, once per process.

### 3.3 `AppCUI/src/Terminal/BrowserTerminal/BrowserTerminal.hpp`

```cpp
#pragma once

#include "../../Internal.hpp"

#ifndef __EMSCRIPTEN__
#    error "BrowserTerminal can only be built for wasm32-emscripten"
#endif

#include <type_traits>

namespace AppCUI::Internal
{
constexpr uint32 BROWSER_ABI_MAGIC   = 0x49554341; // "ACUI" read as a little endian uint32
constexpr uint32 BROWSER_ABI_VERSION = 1;          // bump on ANY change of the structures / exports below
constexpr uint32 BROWSER_MAX_COLUMNS = 1024;       // 1024 x 512 x 4 bytes = 2 MB per canvas (x2 with original)
constexpr uint32 BROWSER_MAX_ROWS    = 512;
constexpr uint32 BROWSER_DEFAULT_COLUMNS = 80;
constexpr uint32 BROWSER_DEFAULT_ROWS    = 25;

// Shared ABI with the JavaScript renderer: 16 little endian uint32 words, read through HEAPU32[(ptr >>> 2) + i].
// Lives in static storage (stable address for the lifetime of the module).
struct BrowserFrameHeader
{
    uint32 magic;         // BROWSER_ABI_MAGIC while a BrowserTerminal is alive, 0 otherwise
    uint32 abiVersion;    // BROWSER_ABI_VERSION
    uint32 frameSeq;      // +1 after every flush (wraps modulo 2^32 - JS compares for equality only)
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

// Fixed capacity ring buffer - no allocation, ever. Producer = JS event handlers (via the exports),
// consumer = GetSystemEvent. Both run on the browser main thread and never interleave -> no atomics.
class BrowserInputQueue
{
  public:
    static constexpr uint32 CAPACITY = 256;
    static_assert((CAPACITY & (CAPACITY - 1)) == 0, "CAPACITY must be a power of two");

    BrowserInputQueue() : head(0), tail(0), dropped(0)
    {
    }
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
    uint32 head;    // next slot to read  - free running, unsigned wrap is well defined
    uint32 tail;    // next slot to write - free running; (tail - head) is always in [0, CAPACITY]
    uint32 dropped; // saturating
};

class BrowserTerminal : public AbstractTerminal
{
    BrowserInputQueue input;
    double lastFramesUpdateMs;
    uint32 pendingWidth, pendingHeight;
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

    // ---- called from the extern "C" exports (already validated there) ----
    bool PushEvent(const BrowserInputEvent& e);
    void RequestResize(uint32 width, uint32 height);
    void RequestClose();
    void RequestRedraw();
    bool HasPendingInput() const;

    // ---- AbstractTerminal ----
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

    // clipboard: process-private in v1 (same policy as CustomTerminal); v2 bridges DOM copy/paste events
    bool ClipboardClear() override;
    bool ClipboardHasText() override;
    bool ClipboardSetText(const ConstString& text) override;
    bool ClipboardGetText(Utils::UnicodeStringBuilder& text) override;
};

// the single live instance (AppCUI is a process-wide singleton); nullptr when none
BrowserTerminal* GetActiveBrowserTerminal();
BrowserFrameHeader& GetBrowserFrameHeader();
} // namespace AppCUI::Internal
```

### 3.4 `BrowserTerminal.cpp`

```cpp
#include "BrowserTerminal.hpp"
#include "../TerminalFactory.hpp"
#include <emscripten/emscripten.h>

// ---------------------------------------------------------------------------------------------------------------
// C++ -> JS imports. Numeric arguments only (no marshalling, no allocation). Every uint32 / pointer reaches JS as a
// signed i32 -> the JS side always normalises with ">>> 0".
// ---------------------------------------------------------------------------------------------------------------
EM_JS(uint32, appcui_js_initial_size, (), {
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
// Suspends the wasm stack (Asyncify) until JS input arrives or the timeout expires. Idle path only.
EM_ASYNC_JS(void, appcui_js_wait_for_input, (uint32 timeoutMs), {
    const host = Module["appcui"];
    if (host) await host.waitForInput(timeoutMs >>> 0);
    else await new Promise((resolve) => setTimeout(resolve, timeoutMs >>> 0));
});

namespace AppCUI::Internal
{
using namespace Application;

constexpr uint32 FRAME_INTERVAL_MS = 33;   // ~30 fps, same cadence as SDLTerminal / CustomTerminal
constexpr uint32 IDLE_WAIT_MS      = 1000; // modal loop without FPS mode: wake at least once per second

static BrowserFrameHeader frameHeader{}; // zero-initialised => magic == 0 until a terminal is alive
static BrowserTerminal* activeTerminal = nullptr;

BrowserFrameHeader& GetBrowserFrameHeader()
{
    return frameHeader;
}
BrowserTerminal* GetActiveBrowserTerminal()
{
    return activeTerminal;
}

// ------------------------------------------------ BrowserInputQueue ------------------------------------------------
bool BrowserInputQueue::Push(const BrowserInputEvent& e)
{
    // coalesce consecutive mouse moves (latest position wins) - keeps the queue short while dragging
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

// ------------------------------------------------ BrowserTerminal -------------------------------------------------
BrowserTerminal::BrowserTerminal()
    : lastFramesUpdateMs(0.0), pendingWidth(0), pendingHeight(0), resizePending(false), closePending(false),
      redrawPending(false), fpsMode(false)
{
}
BrowserTerminal::~BrowserTerminal()
{
    if (activeTerminal == this)
        activeTerminal = nullptr;
    frameHeader.magic = 0;
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

    // self-test of the packed cell layout the JS renderer relies on (cold path, once)
    Graphics::Character probe{};
    probe.Code             = 0x1234;
    probe.Color.Foreground = Graphics::Color::Magenta;   // 0x05
    probe.Color.Background = Graphics::Color::Green;     // 0x0A
    CHECK(probe.PackedValue == 0x0A051234u, false, "Unexpected Character memory layout (0x%08X)", probe.PackedValue);

    uint32 width  = initData.Width;
    uint32 height = initData.Height;
    if ((width == 0) || (height == 0))
    {
        const uint32 packed = appcui_js_initial_size(); // cols | rows << 16 (0 when no host is attached)
        width  = packed & 0xFFFFu;
        height = packed >> 16;
    }
    width  = ClampDimension(width, BROWSER_MAX_COLUMNS, BROWSER_DEFAULT_COLUMNS);
    height = ClampDimension(height, BROWSER_MAX_ROWS, BROWSER_DEFAULT_ROWS);

    CHECK(screenCanvas.Create(width, height), false, "Fail to create the screen canvas (%u x %u)", width, height);
    CHECK(originalScreenCanvas.Create(width, height), false, "Fail to create the original canvas (%u x %u)", width, height);

    this->fpsMode            = (initData.Flags & InitializationFlags::EnableFPSMode) != InitializationFlags::None;
    this->lastFramesUpdateMs = emscripten_get_now();

    frameHeader            = BrowserFrameHeader{};
    frameHeader.abiVersion = BROWSER_ABI_VERSION;
    PublishLayout();
    frameHeader.magic = BROWSER_ABI_MAGIC; // published last: JS ignores the header until magic is valid
    activeTerminal    = this;
    return true;
}
void BrowserTerminal::RestoreOriginalConsoleSettings()
{
    // the page has no "original console"; detach first so the blank "restored" screen that
    // AbstractTerminal::UnInit flushes next is never shown (same policy as CustomTerminal)
    OnUnInit();
}
void BrowserTerminal::OnUnInit()
{
    if (activeTerminal != this)
        return;
    activeTerminal    = nullptr;
    frameHeader.magic = 0;
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
    PublishLayout();          // the canvas may have been resized since the last flush
    frameHeader.frameSeq++;   // well-defined unsigned wrap
    appcui_js_present(&frameHeader);
}
void BrowserTerminal::OnFlushToScreen(const Graphics::Rect&)
{
    OnFlushToScreen(); // the renderer diffs cells itself
}
bool BrowserTerminal::OnUpdateCursor()
{
    frameHeader.cursorX       = screenCanvas.GetCursorX();
    frameHeader.cursorY       = screenCanvas.GetCursorY();
    frameHeader.cursorVisible = screenCanvas.GetCursorVisibility() ? 1u : 0u;
    return true;
}

// ---- producer side (exports) ----
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

// ---- consumer side ----
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
    if (!(elapsed >= 0.0))                 // also catches NaN
        return FRAME_INTERVAL_MS;
    if (elapsed >= static_cast<double>(FRAME_INTERVAL_MS))
        return 0;
    return FRAME_INTERVAL_MS - static_cast<uint32>(elapsed); // elapsed in [0, 33) -> no overflow
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
        // Nested (modal) loop with nothing to do: hand the thread back to the browser. Asyncify unwinds the whole
        // C++ stack here (including the dialog objects that live on it) and rewinds it when JS resolves the wait.
        // No lost wake-up: JS handlers cannot run between the Dequeue above and this call (single thread).
        appcui_js_wait_for_input(this->fpsMode ? MillisecondsUntilNextFrame() : IDLE_WAIT_MS);
    }
}
bool BrowserTerminal::IsEventAvailable()
{
    return HasPendingInput();
}
bool BrowserTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    // the page ships JuliaMono (resources/JuliaMono-Regular.ttf) -> full box drawing support
    return type != Application::SpecialCharacterSetType::Auto;
}

// ---- clipboard (cold path: user-initiated copy / paste only) ----
static std::u16string browserClipboard;

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

// AppCUI::Frontend::Browser -> FrontendType::Browser (and FrontendType::Default: it is the only native frontend of a
// wasm32-emscripten build)
static const TerminalRegistrar browserTerminalRegistrar(
      Application::FrontendType::Browser, CreateTerminal<BrowserTerminal>, DefaultFrontendPriority::Native);
} // namespace AppCUI::Internal
```

The registrar follows the same pattern as [WindowsTerminal.cpp](../../AppCUI/src/Terminal/WindowsTerminal/WindowsTerminal.cpp).

**Hot-path audit (C++):** `GetSystemEvent`, `Dequeue`, `Push`/`Pop`, `OnFlushToScreen` and `PublishLayout` allocate nothing. They do POD copies, a few stores, one `EM_JS` call per flush and one `emscripten_get_now()` per event in FPS mode. The only allocations are `Canvas::Create` and `Canvas::Resize`, which are init and resize paths.

### 3.5 Frontend selection: no new factory code

The split's registry already does everything a per-platform factory used to do:

| Requested frontend | wasm build (Core + Browser component) |
|---|---|
| `Default` | `FindDefaultFrontend()` → `Browser`, the only `Native` registration linked |
| `Browser` | `BrowserTerminal` |
| `Tests` | `TestTerminal` (core). `RunTestScript` keeps working, e.g. headless CI under Node |
| `Custom` | `CustomTerminal` (core) |
| `SDL`, `Terminal`, `WindowsConsole` | Clear `Init` error: *"Frontend '…' is not available: its backend is not linked into this build of AppCUI"* |

Native builds are unaffected: they never link the Browser component, so `Browser` gets the same clear error there.

---

## 4. Part 2 — Emscripten export ABI and memory access

### 4.1 Rules

| Rule | Rationale |
|---|---|
| **No embind, no `emscripten::val`.** | Each `val` is a JS-heap handle with refcounting, and embind adds about 100 KB and RTTI-based dispatch. None of that is needed when every value crossing the boundary is a number. |
| **No `EM_ASM` on hot paths.** | `EM_ASM` packs varargs into an argument array that JS reads back. `EM_JS` is a direct typed wasm import. |
| **Numbers only across the boundary.** | A JS number passed to an `i32` parameter goes through `ToInt32`, which truncates, wraps modulo 2³² and maps NaN to 0. **All validation therefore happens in C++ on the `uint32` value**, never in JS. |
| **JS never writes into wasm memory.** | JS only *reads* the header and the cells. Every write goes through a validated export, so memory corruption from JS is impossible. |
| **Pointers reach JS as unsigned via `>>> 0`.** | Above 2 GB (`-sMAXIMUM_MEMORY=4GB`) a pointer comes through as a negative i32. |
| **Never cache a typed-array view across `layoutSeq` changes or memory growth.** | `Canvas::Resize` moves the buffer, and `ALLOW_MEMORY_GROWTH` detaches the old `ArrayBuffer`. |

### 4.2 `BrowserExports.cpp`

```cpp
#include "BrowserTerminal.hpp"
#include <emscripten/emscripten.h>

namespace AppCUI::Internal
{
// The TypeScript key table (appcui-host.ts) is pinned to these values. A change here must bump BROWSER_ABI_VERSION.
static_assert(static_cast<uint32>(Input::Key::F1) == 1);
static_assert(static_cast<uint32>(Input::Key::Enter) == 13);
static_assert(static_cast<uint32>(Input::Key::Left) == 19);
static_assert(static_cast<uint32>(Input::Key::Space) == 27);
static_assert(static_cast<uint32>(Input::Key::A) == 28);
static_assert(static_cast<uint32>(Input::Key::N0) == 54);
static_assert(static_cast<uint32>(Input::Key::Count) == 64);
static_assert(static_cast<uint32>(Input::Key::Alt) == 0x1000);
static_assert(static_cast<uint32>(Input::Key::Ctrl) == 0x2000);
static_assert(static_cast<uint32>(Input::Key::Shift) == 0x4000);
static_assert(static_cast<uint32>(Input::MouseButton::DoubleClicked) == 0x08);
static_assert(static_cast<uint32>(Input::MouseWheel::Right) == 4);

constexpr uint32 KEY_MODIFIERS_MASK = 0x7000u;
constexpr uint32 KEY_CODE_MASK      = 0x0FFFu;
constexpr uint32 MOUSE_BUTTONS_MASK = 0x0Fu;

static bool IsValidKey(uint32 key)
{
    return ((key & ~(KEY_CODE_MASK | KEY_MODIFIERS_MASK)) == 0) &&
           ((key & KEY_CODE_MASK) < static_cast<uint32>(Input::Key::Count));
}
static bool IsValidBmpCharacter(uint32 ch)
{
    return (ch <= 0xFFFFu) && ((ch < 0xD800u) || (ch > 0xDFFFu)); // lone surrogates are rejected
}
static int32 ClampCoordinate(int32 value, uint32 size)
{
    // size is in [1, BROWSER_MAX_*] -> size - 1 always fits in int32
    if (value < 0)
        return 0;
    const int32 maxValue = static_cast<int32>(size - 1u);
    return value > maxValue ? maxValue : value;
}
static BrowserInputEvent MakeEvent(SystemEventType type)
{
    BrowserInputEvent e{};
    e.type   = type;
    e.key    = Input::Key::None;
    e.button = Input::MouseButton::None;
    e.wheel  = Input::MouseWheel::None;
    return e;
}
} // namespace AppCUI::Internal

using namespace AppCUI;
using namespace AppCUI::Internal;

// Return value convention for every input export: 1 = accepted, 0 = rejected (invalid / no terminal / queue full)
extern "C"
{
EMSCRIPTEN_KEEPALIVE uint32 appcui_browser_abi_version()
{
    return BROWSER_ABI_VERSION;
}
EMSCRIPTEN_KEEPALIVE const BrowserFrameHeader* appcui_browser_frame_header()
{
    return &GetBrowserFrameHeader(); // static storage - stable for the lifetime of the module
}
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_key(uint32 key, uint32 unicodeChar)
{
    auto* term = GetActiveBrowserTerminal();
    if ((term == nullptr) || !IsValidKey(key) || !IsValidBmpCharacter(unicodeChar))
        return 0;
    if ((key & KEY_CODE_MASK) == 0 && unicodeChar == 0)
        return 0; // nothing to report
    auto e        = MakeEvent(SystemEventType::KeyPressed);
    e.key         = static_cast<Input::Key>(key);
    e.unicodeChar = static_cast<char16>(unicodeChar);
    return term->PushEvent(e) ? 1 : 0;
}
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_shift_state(uint32 modifiers)
{
    auto* term = GetActiveBrowserTerminal();
    if ((term == nullptr) || ((modifiers & ~KEY_MODIFIERS_MASK) != 0))
        return 0;
    auto e = MakeEvent(SystemEventType::ShiftStateChanged);
    e.key  = static_cast<Input::Key>(modifiers);
    return term->PushEvent(e) ? 1 : 0;
}
// type: 0 = down, 1 = up, 2 = move
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_mouse(uint32 type, int32 x, int32 y, uint32 buttons, uint32 modifiers)
{
    auto* term = GetActiveBrowserTerminal();
    if ((term == nullptr) || (type > 2u) || ((buttons & ~MOUSE_BUTTONS_MASK) != 0) ||
        ((modifiers & ~KEY_MODIFIERS_MASK) != 0))
        return 0;
    constexpr SystemEventType types[3] = { SystemEventType::MouseDown, SystemEventType::MouseUp, SystemEventType::MouseMove };
    auto e   = MakeEvent(types[type]);
    e.x      = ClampCoordinate(x, term->screenCanvas.GetWidth());
    e.y      = ClampCoordinate(y, term->screenCanvas.GetHeight());
    e.button = static_cast<Input::MouseButton>(buttons);
    e.key    = static_cast<Input::Key>(modifiers);
    return term->PushEvent(e) ? 1 : 0;
}
// direction: 1 = up, 2 = down, 3 = left, 4 = right (Input::MouseWheel)
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_wheel(int32 x, int32 y, uint32 direction, uint32 modifiers)
{
    auto* term = GetActiveBrowserTerminal();
    if ((term == nullptr) || (direction == 0u) || (direction > 4u) || ((modifiers & ~KEY_MODIFIERS_MASK) != 0))
        return 0;
    auto e  = MakeEvent(SystemEventType::MouseWheel);
    e.x     = ClampCoordinate(x, term->screenCanvas.GetWidth());
    e.y     = ClampCoordinate(y, term->screenCanvas.GetHeight());
    e.wheel = static_cast<Input::MouseWheel>(direction);
    e.key   = static_cast<Input::Key>(modifiers);
    return term->PushEvent(e) ? 1 : 0;
}
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_resize(uint32 columns, uint32 rows)
{
    auto* term = GetActiveBrowserTerminal();
    if ((term == nullptr) || (columns == 0u) || (rows == 0u) || (columns > BROWSER_MAX_COLUMNS) ||
        (rows > BROWSER_MAX_ROWS))
        return 0;
    term->RequestResize(columns, rows);
    return 1;
}
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_close()
{
    auto* term = GetActiveBrowserTerminal();
    if (term == nullptr)
        return 0;
    term->RequestClose();
    return 1;
}
EMSCRIPTEN_KEEPALIVE int32 appcui_browser_redraw()
{
    auto* term = GetActiveBrowserTerminal();
    if (term == nullptr)
        return 0;
    term->RequestRedraw();
    return 1;
}
} // extern "C"
```

> `screenCanvas` is a public member of `AbstractTerminal`, so the exports need no friend declarations. `GetWidth()` is at least 1 whenever a terminal is active (`OnInit` and `appcui_browser_resize` both clamp to ≥ 1), which makes the `size - 1u` in `ClampCoordinate` safe.

### 4.3 Integer-safety matrix

| Value | Source | Check |
|---|---|---|
| `key` | JS `uint32` | No bits outside `0x0FFF \| 0x7000`; code `< Key::Count` |
| `unicodeChar` | JS `uint32` | `≤ 0xFFFF`, not a surrogate (AppCUI is UTF-16 BMP per cell) |
| `x`, `y` | JS `int32` | Clamped to `[0, size-1]` |
| `buttons` | JS `uint32` | `& ~0x0F == 0` |
| `direction` | JS `uint32` | `1…4` |
| `columns`, `rows` | JS `uint32` | `1…1024` × `1…512`. The product is at most 2¹⁹ cells (2 MB), so `w*h*sizeof(Character)` cannot overflow `uint32` |
| `frameSeq`, `layoutSeq` | C++ `uint32` | Unsigned wrap is defined; JS compares with `!==` only |
| `droppedEvents` | C++ `uint32` | Saturating increment |
| FPS timing | `double` ms | NaN/negative guarded; `uint32` produced only from `[0, 33)` |
| Pointers | C++ → JS | `static_assert(sizeof(void*) == 4)`; JS applies `>>> 0` then `>>> 2` (cells are 4-byte aligned, asserted in JS on rebind) |

### 4.4 Memory growth

- Link with `-sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=HEAPU8,HEAPU32`. Emscripten's `updateMemoryViews()` reassigns the exported `Module.HEAPU32` after growth.
- The renderer reads `module.HEAPU32` **every frame**, which is a property load with no allocation. It rebuilds its `cells` subarray **only** when `heap.buffer !== this.boundBuffer` or `layoutSeq` changed. Those are cold events, and the one allocation of a view object happens there.

---

## 5. Part 3 — HTML5 canvas renderer and input host (TypeScript)

### 5.1 Rendering strategy

- **Integer device pixels everywhere.** Cell width and height are measured *in device pixels* at `fontSizeCss × devicePixelRatio` and rounded up to integers. The backing store is `cols·cellW × rows·cellH`, and the CSS size is that value divided by `dpr`. The transform stays identity, which gives no fractional seams, crisp text and exact hit-testing.
- **Diff, then draw.** JS keeps a `shadow: Uint32Array` (reallocated only on layout change). For each row it finds maximal runs of changed cells that share a background, issues one `fillRect` per run, then one `fillText(glyph, x, baseline, cellW)` per non-blank cell. The `maxWidth = cellW` argument stops wide fallback glyphs from bleeding into neighbouring cells, which would leave stale pixels behind with dirty-rect redraws.
- **Seam-free glyphs.** Block elements (U+2580–U+259F) and the light single-line box set (`─│┌┐└┘├┤┬┴┼`) are drawn with `fillRect`. Other glyphs fall back to the font. The project already ships **JuliaMono** ([resources/JuliaMono-Regular.ttf](../../AppCUI/resources)), and the page loads it with `@font-face`.
- **Zero allocations per frame:** a 65,536-slot lazily filled glyph-string cache, 256-entry palette lookup tables (any byte value is safe and `Transparent`/garbage map to defaults), `fillStyle` set only when it changes, and no closures, template strings or `subarray` calls in the frame path.
- **Coalesced presents.** `present()` only sets a flag and calls `queueMicrotask(this.drawBound)`, so all flushes from one tick or modal wake-up produce **one** draw. The draw runs before the browser paints the current frame, so there is no extra frame of latency.

### 5.2 `appcui-host.ts`

```ts
// AppCUI BrowserTerminal host: canvas renderer + DOM input bridge.  Pinned to C++ BROWSER_ABI_VERSION = 1.

export interface AppCUIModule {
  HEAPU32: Uint32Array;
  _appcui_browser_abi_version(): number;
  _appcui_browser_frame_header(): number;
  _appcui_browser_key(key: number, unicodeChar: number): number;
  _appcui_browser_shift_state(modifiers: number): number;
  _appcui_browser_mouse(type: number, x: number, y: number, buttons: number, modifiers: number): number;
  _appcui_browser_wheel(x: number, y: number, direction: number, modifiers: number): number;
  _appcui_browser_resize(columns: number, rows: number): number;
  _appcui_browser_close(): number;
  _appcui_browser_redraw(): number;
}

export interface AppCUIHostOptions {
  fontFamily: string;      // e.g. '"JuliaMono", monospace'
  fontSizeCss: number;     // CSS px
  lineHeight?: number;     // multiplier of ascent+descent, default 1.0
  maxColumns?: number;     // must not exceed the C++ BROWSER_MAX_COLUMNS (1024)
  maxRows?: number;        // must not exceed the C++ BROWSER_MAX_ROWS (512)
}

const ABI_MAGIC = 0x49554341;
const ABI_VERSION = 1;
// header word indexes (BrowserFrameHeader)
const H_MAGIC = 0, H_ABI = 1, H_FRAME_SEQ = 2, H_LAYOUT_SEQ = 3, H_CELLS_PTR = 4, H_WIDTH = 5, H_HEIGHT = 6,
  H_CURSOR_X = 7, H_CURSOR_Y = 8, H_CURSOR_VISIBLE = 9;

const MOD_ALT = 0x1000, MOD_CTRL = 0x2000, MOD_SHIFT = 0x4000;
const MOUSE_DOWN = 0, MOUSE_UP = 1, MOUSE_MOVE = 2;
const BTN_LEFT = 0x01, BTN_CENTER = 0x02, BTN_RIGHT = 0x04, BTN_DOUBLE = 0x08;
const DOUBLE_CLICK_MS = 400;

// Graphics::Color 0..15 (same RGB values as SDLTerminalScreen.cpp); index 16 = Transparent
const PALETTE = [
  '#000000', '#000080', '#13a10e', '#3a96dd', '#c50f1f', '#881798', '#c19c00', '#cccccc',
  '#767676', '#3b78ff', '#16c60c', '#61d6d6', '#e74856', '#b4009e', '#f9f1a5', '#f2f2f2',
];
const FG = new Array<string>(256).fill(PALETTE[7]);  // garbage / Transparent foreground -> Silver
const BG = new Array<string>(256).fill(PALETTE[0]);  // garbage / Transparent background -> Black
for (let i = 0; i < 16; i++) { FG[i] = PALETTE[i]; BG[i] = PALETTE[i]; }

// KeyboardEvent.code -> Input::Key (pinned by static_asserts in BrowserExports.cpp)
const KEY: Record<string, number> = (() => {
  const k: Record<string, number> = {
    Enter: 13, NumpadEnter: 13, Escape: 14, Insert: 15, Delete: 16, Backspace: 17, Tab: 18,
    ArrowLeft: 19, ArrowUp: 20, ArrowDown: 21, ArrowRight: 22, PageUp: 23, PageDown: 24,
    Home: 25, End: 26, Space: 27,
  };
  for (let i = 1; i <= 12; i++) k['F' + i] = i;
  for (let i = 0; i < 26; i++) k['Key' + String.fromCharCode(65 + i)] = 28 + i;
  for (let i = 0; i < 10; i++) { k['Digit' + i] = 54 + i; k['Numpad' + i] = 54 + i; }
  return k;
})();
const MODIFIER_CODES = new Set(['ShiftLeft', 'ShiftRight', 'ControlLeft', 'ControlRight', 'AltLeft', 'AltRight']);

// Light box-drawing set U+2500..U+257F encoded as arms: up=1, right=2, down=4, left=8 (0 = use the font)
const BOX = new Uint8Array(0x80);
BOX[0x00] = 10; BOX[0x02] = 5; BOX[0x0c] = 6; BOX[0x10] = 12; BOX[0x14] = 3; BOX[0x18] = 9;
BOX[0x1c] = 7; BOX[0x24] = 13; BOX[0x2c] = 14; BOX[0x34] = 11; BOX[0x3c] = 15;

export class AppCUIHost {
  private mod: AppCUIModule | null = null;
  private readonly ctx: CanvasRenderingContext2D;
  private readonly opts: Required<AppCUIHostOptions>;

  // metrics (device px, integers)
  private dpr = 1; private cellW = 8; private cellH = 16; private baseline = 12; private lineT = 1;
  private fontCss = '';

  // bound state
  private hdr = 0;                          // header word index into HEAPU32
  private boundBuffer: ArrayBufferLike | null = null;
  private boundLayoutSeq = -1;
  private cells: Uint32Array = new Uint32Array(0);
  private shadow: Uint32Array = new Uint32Array(0);
  private cols = 0; private rows = 0;
  private lastFrameSeq = -1;
  private forceFull = true;
  private curX = -1; private curY = -1; private curVisible = false;
  private lastFill = '';
  private readonly glyphs: (string | undefined)[] = new Array(65536);

  // scheduling
  private drawQueued = false;
  private readonly drawBound = (): void => this.draw();
  private wakeResolve: (() => void) | null = null;
  private wakeTimer = 0;
  private exited = false;

  // input state
  private mods = 0;
  private lastCellX = -1; private lastCellY = -1; private lastButtons = -1;
  private lastDownTime = 0; private lastDownX = -1; private lastDownY = -1;
  private gridCols = 0; private gridRows = 0;

  constructor(private readonly canvas: HTMLCanvasElement, private readonly container: HTMLElement,
              options: AppCUIHostOptions) {
    this.opts = { lineHeight: 1.0, maxColumns: 1024, maxRows: 512, ...options };
    const ctx = canvas.getContext('2d', { alpha: false, desynchronized: true });
    if (!ctx) throw new Error('2D canvas context not available');
    this.ctx = ctx;
    this.measure();
    this.computeGrid();
    this.installInput();
    new ResizeObserver(() => this.onContainerResize()).observe(container);
    this.watchDpr();
  }

  // ------------------------------------------------------------------ called from C++ (EM_JS)
  attach(mod: AppCUIModule): void {
    if (mod._appcui_browser_abi_version() !== ABI_VERSION) throw new Error('AppCUI browser ABI mismatch');
    this.mod = mod;
    this.hdr = (mod._appcui_browser_frame_header() >>> 0) >>> 2;
  }
  initialSize(): number {                       // packed cols | rows << 16
    return (this.gridCols & 0xffff) | ((this.gridRows & 0xffff) << 16);
  }
  present(_headerPtr: number): void {
    if (this.drawQueued || this.exited) return;
    this.drawQueued = true;
    queueMicrotask(this.drawBound);             // coalesce every flush of this task into one draw
  }
  waitForInput(timeoutMs: number): Promise<void> {  // idle path only (modal loop with nothing to do)
    return new Promise<void>((resolve) => {
      this.wakeResolve = resolve;
      this.wakeTimer = window.setTimeout(() => this.wake(), timeoutMs);
    });
  }
  onExit(): void {
    this.exited = true;
    this.wake();
  }

  // ------------------------------------------------------------------ rendering
  private draw(): void {
    this.drawQueued = false;
    const mod = this.mod;
    if (!mod || this.exited) return;
    const heap = mod.HEAPU32;                   // re-read: replaced on memory growth
    const h = this.hdr;
    if (heap[h + H_MAGIC] !== ABI_MAGIC || heap[h + H_ABI] !== ABI_VERSION) return;

    const layoutSeq = heap[h + H_LAYOUT_SEQ];
    if (layoutSeq !== this.boundLayoutSeq || heap.buffer !== this.boundBuffer) this.rebind(heap, layoutSeq);

    // cursor moved/hidden -> the cell under the old cursor must be repainted
    const cx = heap[h + H_CURSOR_X], cy = heap[h + H_CURSOR_Y], cv = heap[h + H_CURSOR_VISIBLE] !== 0;
    if (cx !== this.curX || cy !== this.curY || cv !== this.curVisible) {
      this.invalidateCell(this.curX, this.curY);
      this.curX = cx; this.curY = cy; this.curVisible = cv;
      this.invalidateCell(cx, cy);
    }
    this.lastFrameSeq = heap[h + H_FRAME_SEQ];
    this.paintCells();
    if (cv && cx < this.cols && cy < this.rows) this.paintCursor(cx, cy);
  }

  private rebind(heap: Uint32Array, layoutSeq: number): void {   // cold path (resize / memory growth)
    const h = this.hdr;
    const ptr = heap[h + H_CELLS_PTR] >>> 0;
    const w = heap[h + H_WIDTH] >>> 0, r = heap[h + H_HEIGHT] >>> 0;
    if ((ptr & 3) !== 0 || w === 0 || r === 0 || w > this.opts.maxColumns || r > this.opts.maxRows ||
        (ptr >>> 2) + w * r > heap.length) {
      throw new Error('AppCUI: invalid frame header');
    }
    this.cells = heap.subarray(ptr >>> 2, (ptr >>> 2) + w * r);
    if (w !== this.cols || r !== this.rows) {
      this.cols = w; this.rows = r;
      this.shadow = new Uint32Array(w * r);
      this.canvas.width = w * this.cellW;          // resets the 2D context state
      this.canvas.height = r * this.cellH;
      this.canvas.style.width = (w * this.cellW) / this.dpr + 'px';
      this.canvas.style.height = (r * this.cellH) / this.dpr + 'px';
      this.applyContextState();
    }
    this.boundBuffer = heap.buffer;
    this.boundLayoutSeq = layoutSeq;
    this.forceFull = true;
  }

  private paintCells(): void {
    const cells = this.cells, shadow = this.shadow, ctx = this.ctx;
    const cols = this.cols, rows = this.rows, cw = this.cellW, ch = this.cellH;
    const full = this.forceFull;
    this.forceFull = false;
    for (let y = 0; y < rows; y++) {
      const row = y * cols;
      const py = y * ch;
      let x = 0;
      while (x < cols) {
        const v = cells[row + x];
        if (!full && v === shadow[row + x]) { x++; continue; }
        const bg = v >>> 24;
        let end = x + 1;
        while (end < cols) {
          const w = cells[row + end];
          if ((!full && w === shadow[row + end]) || (w >>> 24) !== bg) break;
          end++;
        }
        this.setFill(BG[bg]);
        ctx.fillRect(x * cw, py, (end - x) * cw, ch);
        for (let i = x; i < end; i++) {
          const c = cells[row + i];
          shadow[row + i] = c;
          const code = c & 0xffff;
          if (code === 0x20 || code === 0) continue;
          const fg = (c >>> 16) & 0xff;
          if (!this.drawProcedural(code, FG[fg], i * cw, py)) {
            this.setFill(FG[fg]);
            ctx.fillText(this.glyph(code), i * cw, py + this.baseline, cw);
          }
        }
        x = end;
      }
    }
  }

  private paintCursor(cx: number, cy: number): void {
    const c = this.cells[cy * this.cols + cx];
    this.setFill(FG[(c >>> 16) & 0xff]);
    const t = Math.max(2, this.lineT * 2);
    this.ctx.fillRect(cx * this.cellW, (cy + 1) * this.cellH - t, this.cellW, t);
  }

  private invalidateCell(x: number, y: number): void {
    if (x >= 0 && y >= 0 && x < this.cols && y < this.rows) {
      const i = y * this.cols + x;
      this.shadow[i] = ~this.cells[i] >>> 0;     // guaranteed different -> repainted next pass
    }
  }

  // Block elements and light lines drawn with rectangles: no gaps between cells at any dpr / zoom.
  private drawProcedural(code: number, color: string, x: number, y: number): boolean {
    const ctx = this.ctx, cw = this.cellW, ch = this.cellH;
    if (code >= 0x2580 && code <= 0x259f) {
      const hw = cw >> 1, hh = ch >> 1;
      switch (code) {
        case 0x2588: this.setFill(color); ctx.fillRect(x, y, cw, ch); return true;            // █
        case 0x2580: this.setFill(color); ctx.fillRect(x, y, cw, hh); return true;            // ▀
        case 0x2584: this.setFill(color); ctx.fillRect(x, y + hh, cw, ch - hh); return true;  // ▄
        case 0x258c: this.setFill(color); ctx.fillRect(x, y, hw, ch); return true;            // ▌
        case 0x2590: this.setFill(color); ctx.fillRect(x + hw, y, cw - hw, ch); return true;  // ▐
        case 0x2591: case 0x2592: case 0x2593: {                                              // ░ ▒ ▓
          this.setFill(color);
          ctx.globalAlpha = code === 0x2591 ? 0.25 : code === 0x2592 ? 0.5 : 0.75;
          ctx.fillRect(x, y, cw, ch);
          ctx.globalAlpha = 1;
          return true;
        }
        default: return false;
      }
    }
    if (code >= 0x2500 && code <= 0x257f) {
      const arms = BOX[code - 0x2500];
      if (arms === 0) return false;
      const t = this.lineT;
      const mx = x + ((cw - t) >> 1), my = y + ((ch - t) >> 1);
      this.setFill(color);
      if (arms & 1) ctx.fillRect(mx, y, t, my - y + t);
      if (arms & 4) ctx.fillRect(mx, my, t, y + ch - my);
      if (arms & 8) ctx.fillRect(x, my, mx - x + t, t);
      if (arms & 2) ctx.fillRect(mx, my, x + cw - mx, t);
      return true;
    }
    return false;
  }

  private glyph(code: number): string {
    let s = this.glyphs[code];
    if (s === undefined) {
      s = code >= 0xd800 && code <= 0xdfff ? '�' : String.fromCharCode(code);
      this.glyphs[code] = s;                     // filled once per distinct code point
    }
    return s;
  }

  private setFill(style: string): void {
    if (style !== this.lastFill) { this.ctx.fillStyle = style; this.lastFill = style; }
  }

  // ------------------------------------------------------------------ metrics, resize, DPR
  private measure(): void {
    this.dpr = window.devicePixelRatio || 1;
    const px = Math.max(6, Math.round(this.opts.fontSizeCss * this.dpr));
    this.fontCss = px + 'px ' + this.opts.fontFamily;
    this.ctx.font = this.fontCss;
    const m = this.ctx.measureText('M');
    const asc = Math.ceil(m.fontBoundingBoxAscent || px * 0.8);
    const desc = Math.ceil(m.fontBoundingBoxDescent || px * 0.2);
    this.cellW = Math.max(1, Math.ceil(m.width));
    this.cellH = Math.max(1, Math.ceil((asc + desc) * this.opts.lineHeight));
    this.baseline = asc + ((this.cellH - asc - desc) >> 1);
    this.lineT = Math.max(1, Math.round(this.dpr));
  }
  private applyContextState(): void {
    this.ctx.font = this.fontCss;
    this.ctx.textBaseline = 'alphabetic';
    this.ctx.textAlign = 'left';
    this.lastFill = '';
  }
  private computeGrid(): boolean {
    const r = this.container.getBoundingClientRect();
    const cols = Math.min(this.opts.maxColumns, Math.max(1, Math.floor((r.width * this.dpr) / this.cellW)));
    const rows = Math.min(this.opts.maxRows, Math.max(1, Math.floor((r.height * this.dpr) / this.cellH)));
    const changed = cols !== this.gridCols || rows !== this.gridRows;
    this.gridCols = cols; this.gridRows = rows;
    return changed;
  }
  private onContainerResize(): void {
    if (this.computeGrid() && this.mod && !this.exited) {
      this.mod._appcui_browser_resize(this.gridCols, this.gridRows);
      this.wake();
    }
  }
  private watchDpr(): void {
    const mq = window.matchMedia('(resolution: ' + this.dpr + 'dppx)');
    mq.addEventListener('change', () => {
      this.measure();
      this.cols = 0; this.rows = 0; this.boundLayoutSeq = -1;   // force a rebind with the new cell size
      this.onContainerResize();
      if (this.mod) { this.mod._appcui_browser_redraw(); this.wake(); }
      this.watchDpr();
    }, { once: true });
  }

  // ------------------------------------------------------------------ input
  private wake(): void {
    if (this.wakeTimer) { clearTimeout(this.wakeTimer); this.wakeTimer = 0; }
    const r = this.wakeResolve;
    if (r) { this.wakeResolve = null; r(); }
  }
  private modsOf(e: KeyboardEvent | MouseEvent): number {
    return (e.altKey ? MOD_ALT : 0) | (e.ctrlKey ? MOD_CTRL : 0) | (e.shiftKey ? MOD_SHIFT : 0);
  }
  private cellOf(e: MouseEvent): void {
    this.lastCellX = Math.floor((e.offsetX * this.dpr) / this.cellW) | 0;
    this.lastCellY = Math.floor((e.offsetY * this.dpr) / this.cellH) | 0;
  }
  private static buttonsOf(domButtons: number): number {        // DOM: 1 left, 2 right, 4 middle
    return ((domButtons & 1) ? BTN_LEFT : 0) | ((domButtons & 4) ? BTN_CENTER : 0) | ((domButtons & 2) ? BTN_RIGHT : 0);
  }
  private static buttonOf(domButton: number): number {          // DOM: 0 left, 1 middle, 2 right
    return domButton === 0 ? BTN_LEFT : domButton === 1 ? BTN_CENTER : domButton === 2 ? BTN_RIGHT : 0;
  }
  private updateMods(next: number): void {
    if (next !== this.mods && this.mod) { this.mods = next; this.mod._appcui_browser_shift_state(next); this.wake(); }
  }

  private installInput(): void {
    const c = this.canvas;
    c.tabIndex = 0;
    c.style.outline = 'none';

    c.addEventListener('keydown', (e) => {
      if (!this.mod || this.exited) return;
      e.preventDefault();
      const mods = this.modsOf(e);
      if (MODIFIER_CODES.has(e.code)) { this.updateMods(mods); return; }
      this.mods = mods;
      const altGr = e.getModifierState('AltGraph');
      const code = KEY[e.code] ?? 0;
      let ch = 0;
      if (e.key.length === 1 && (altGr || (mods & (MOD_CTRL | MOD_ALT)) === 0)) ch = e.key.charCodeAt(0);
      // AltGr arrives as Ctrl+Alt on Windows: keep only Shift so AltGr text is not reported as a Ctrl+Alt chord
      const key = code === 0 ? 0 : (code | (altGr ? (mods & MOD_SHIFT) : mods));
      if (this.mod._appcui_browser_key(key >>> 0, ch >>> 0)) this.wake();
    });
    c.addEventListener('keyup', (e) => { if (MODIFIER_CODES.has(e.code)) this.updateMods(this.modsOf(e)); });
    c.addEventListener('blur', () => this.updateMods(0));            // never leave a stuck Ctrl/Alt/Shift

    c.addEventListener('pointerdown', (e) => {
      if (!this.mod || this.exited) return;
      c.focus();
      c.setPointerCapture(e.pointerId);                              // keep receiving moves while dragging out
      this.cellOf(e);
      let btn = AppCUIHost.buttonOf(e.button);
      const now = e.timeStamp;
      if (btn === BTN_LEFT && now - this.lastDownTime < DOUBLE_CLICK_MS &&
          this.lastCellX === this.lastDownX && this.lastCellY === this.lastDownY) {
        btn |= BTN_DOUBLE; this.lastDownTime = 0;
      } else {
        this.lastDownTime = now; this.lastDownX = this.lastCellX; this.lastDownY = this.lastCellY;
      }
      if (this.mod._appcui_browser_mouse(MOUSE_DOWN, this.lastCellX, this.lastCellY, btn, this.modsOf(e))) this.wake();
      e.preventDefault();
    });
    c.addEventListener('pointerup', (e) => {
      if (!this.mod || this.exited) return;
      this.cellOf(e);
      if (this.mod._appcui_browser_mouse(MOUSE_UP, this.lastCellX, this.lastCellY,
          AppCUIHost.buttonOf(e.button), this.modsOf(e))) this.wake();
    });
    c.addEventListener('pointermove', (e) => {
      if (!this.mod || this.exited) return;
      const px = this.lastCellX, py = this.lastCellY;
      this.cellOf(e);
      const buttons = AppCUIHost.buttonsOf(e.buttons);
      if (px === this.lastCellX && py === this.lastCellY && buttons === this.lastButtons) return; // same cell
      this.lastButtons = buttons;
      if (this.mod._appcui_browser_mouse(MOUSE_MOVE, this.lastCellX, this.lastCellY, buttons, this.modsOf(e))) this.wake();
    });
    c.addEventListener('wheel', (e) => {
      if (!this.mod || this.exited) return;
      e.preventDefault();
      this.cellOf(e);
      const dir = e.deltaY > 0 ? 2 : e.deltaY < 0 ? 1 : e.deltaX > 0 ? 4 : e.deltaX < 0 ? 3 : 0;
      if (dir && this.mod._appcui_browser_wheel(this.lastCellX, this.lastCellY, dir, this.modsOf(e))) this.wake();
    }, { passive: false });
    c.addEventListener('contextmenu', (e) => e.preventDefault());
  }
}
```

**Wheel direction:** DOM `deltaY > 0` (content scrolls down) maps to `MouseWheel::Down`. `SDLTerminalInput.cpp` maps `wheel.y < 0 → Up`, so SDL looks inverted relative to the DOM. Validate both against `WindowsTerminal` with the `ListView` example before freezing this, and add a host option if needed.

**Key semantics** follow `SDLTerminalInput.cpp`: a translated key ORed with the modifiers; the unicode character is zeroed when Ctrl/Alt is held; a pure modifier change sends `ShiftStateChanged`. The browser additionally passes **any BMP character** (SDL limits this to ASCII 32–127) and treats **AltGr** as text input.

### 5.3 Bootstrap (`index.html` excerpt)

```html
<style>
  @font-face { font-family: "JuliaMono"; src: url("JuliaMono-Regular.ttf") format("truetype"); font-display: block; }
  html, body { margin: 0; height: 100%; background: #000; }
  #term-host { position: fixed; inset: 0; overflow: hidden; }
  #term { display: block; image-rendering: pixelated; }
</style>
<div id="term-host"><canvas id="term"></canvas></div>
<script type="module">
  import createAppCUIModule from './MessageBox.mjs';
  import { AppCUIHost } from './appcui-host.js';

  await document.fonts.load('16px "JuliaMono"');   // metrics must come from the real font
  const host = new AppCUIHost(document.getElementById('term'), document.getElementById('term-host'),
                              { fontFamily: '"JuliaMono", monospace', fontSizeCss: 16 });
  const moduleArg = {
    appcui: host,                                   // read by the EM_JS imports as Module["appcui"]
    onRuntimeInitialized() { host.attach(moduleArg); },   // MODULARIZE uses moduleArg as `Module`
  };
  await createAppCUIModule(moduleArg);              // main() runs right after onRuntimeInitialized
  document.getElementById('term').focus();
</script>
```

---

## 6. Part 4 — event-loop adaptation

### 6.1 Why `emscripten_set_main_loop` alone is not enough

`emscripten_set_main_loop` turns *one* loop into a callback, and only works for code that can return to the browser between iterations. AppCUI has three loop shapes, and two of them cannot do that:

| Shape | Example | Can it return to the browser? |
|---|---|---|
| Root loop | `Application::Run()` → `ExecuteEventLoop()` | **Yes**, if split into steps |
| Modal loop inside an event handler | `MessageBox::ShowOkCancel()` → `Window::Show()` → nested `ExecuteEventLoop`, which returns `Dialogs::Result` *synchronously* | **No.** The caller's stack frame, including `FileDialogWindow dlg` on that stack, must stay alive until the dialog closes. |
| Modal loop before `Run()` | `main()` → `MessageBox::ShowWarning(...)` → `Run()` | **No**, and there is no root loop yet |

Turning every `Show()` into a callback would break the public API and every application that uses it, so it is not an option. The stack has to be **suspended**. There are three ways to do that:

| Mechanism | Cost | Verdict |
|---|---|---|
| **Asyncify** (`-sASYNCIFY=1`) | +30–50 % code size, small slowdown from instrumented functions; unwind/rewind only at suspend points | **Default.** Works in every browser. Emscripten pauses `MainLoop` during the sleep and resumes it after the rewind. |
| **JSPI** (`-sJSPI`) | Near-zero overhead, native stack switching | **Opt-in flag** (`APPCUI_WASM_JSPI=ON`). The JSPI `handleSleep` does *not* pause `MainLoop`, so a modal opened inside a tick needs extra guarding (see 6.6). It is enabled by default only after M1 passes on all target browsers. |
| **Worker + `Atomics.wait`** | Needs `SharedArrayBuffer`, which needs COOP/COEP headers; the renderer reads a double-buffered screen | **Rejected for v1.** Hosting requirements and buffer synchronisation cost more than they save. It stays the documented fallback if Asyncify size becomes a problem. |

### 6.2 Refactor: Enter / Step / Leave (`Internal.hpp`, `Application.cpp`)

```cpp
    // one activation of the event loop (root or modal); owned by the caller of ExecuteEventLoop
    struct EventLoopFrame
    {
        Controls::Control* ctrl; // modal control (nullptr for the root loop)
        SystemEvent evnt;        // kept per frame: exactly the lifetime the old local variable had
        bool nonBlocking;        // true only for the browser root loop (driven by requestAnimationFrame)
    };

    // inside struct ApplicationImpl
        bool EnterEventLoop(EventLoopFrame& frame, bool resetState); // prologue of the old ExecuteEventLoop
        bool StepEventLoop(EventLoopFrame& frame);                   // ONE iteration; false when the loop ends
        void LeaveEventLoop(EventLoopFrame& frame);                  // epilogue of the old ExecuteEventLoop
        bool HasPendingWork() const;                                 // deferred deletes / repaint / command bar
```

```cpp
bool ApplicationImpl::ExecuteEventLoop(Control* ctrl, bool resetState)
{
    EventLoopFrame frame{};
    frame.ctrl        = ctrl;
    frame.nonBlocking = false;
    if (!EnterEventLoop(frame, resetState))
        return false; // EnterEventLoop already logged the reason (same CHECKs as before)
    while (StepEventLoop(frame))
    {
    }
    LeaveEventLoop(frame);
    return true;
}

bool ApplicationImpl::EnterEventLoop(EventLoopFrame& frame, bool resetState)
{
    CHECK(app->Inited, false, "Application has not been corectly initialized !");
    this->RepaintStatus      = REPAINT_STATUS_ALL;
    this->MouseLockedControl = nullptr;
    this->mouseLockedObject  = MouseLockedObject::None;
    if (resetState)
        this->loopStatus = LoopStatus::Normal;
    if (this->MouseOverControl)
    {
        ((ControlContext*) (MouseOverControl->Context))->MouseIsOver = false;
        this->MouseOverControl                                       = nullptr;
    }
    PackControl(true);
    if (frame.ctrl != nullptr)
    {
        CHECK(ModalControlsCount < MAX_MODAL_CONTROLS_STACK, false, "Too many modal calls !");
        ModalControlsStack[ModalControlsCount] = frame.ctrl;
        ModalControlsCount++;
    }
    UpdateCommandBar();
    return true;
}

bool ApplicationImpl::StepEventLoop(EventLoopFrame& frame)
{
    if (loopStatus != LoopStatus::Normal)
        return false;

    // ---- body of the old while loop, moved verbatim; only `evnt` became `frame.evnt` ----
    //      (deferred deletes, command bar update, ComputePositions/Paint, terminal->Update())
    //      ...
    this->terminal->nonBlockingEvents = frame.nonBlocking;
    this->terminal->GetSystemEvent(frame.evnt);
    //      ... modifier map, updateFrames, switch (frame.evnt.eventType) { ... }  (verbatim)

    return loopStatus == LoopStatus::Normal;
}

void ApplicationImpl::LeaveEventLoop(EventLoopFrame& frame)
{
    // epilogue of the old ExecuteEventLoop, verbatim, with `ctrl` -> `frame.ctrl`
}

bool ApplicationImpl::HasPendingWork() const
{
    return (!toDelete.empty()) || cmdBarUpdate || (RepaintStatus != REPAINT_STATUS_NONE);
}
```

**Equivalence argument (native builds):** `while (StepEventLoop(f)) {}` runs the body exactly as often as `while (loopStatus == Normal) { body }`, because the guard is evaluated before every body and after the last one. `SystemEvent` keeps the lifetime of the old local, one per activation and stale fields included. `nonBlockingEvents` is always `false` outside the browser root loop, and other backends never read it. **The existing `TestTerminal` screen-hash scripts (`Tests/Tester`) are the regression gate**: every hash must stay bit-identical.

### 6.3 Browser root loop (`Terminal/BrowserTerminal/BrowserMainLoop.cpp`, part of `AppCUI::Frontend::Browser`)

The core only learns that a terminal *may* drive the root loop (`AbstractTerminal::RunRootEventLoop`, §3.2). Everything Emscripten-specific lives in the Browser component.

```cpp
#include "BrowserTerminal.hpp"
#include <emscripten/emscripten.h>

namespace AppCUI::Internal
{
constexpr uint32 MAX_STEPS_PER_TICK = 64; // bounds a tick even if a control keeps requesting repaints

static EventLoopFrame rootFrame{};          // static storage: survives the unwinding of main()
static ApplicationImpl* rootApp      = nullptr;
static BrowserTerminal* rootTerminal = nullptr;
static void (*rootShutdown)()        = nullptr;

static void BrowserMainLoopTick()
{
    uint32 steps = 0;
    bool running = true;
    do
    {
        running = rootApp->StepEventLoop(rootFrame); // paints first, then consumes at most one event
        steps++;
    } while (running && (steps < MAX_STEPS_PER_TICK) && (rootTerminal->HasPendingInput() || rootApp->HasPendingWork()));

    if (!running)
    {
        emscripten_cancel_main_loop();
        rootApp->LeaveEventLoop(rootFrame);
        auto shutdown = rootShutdown;
        rootApp       = nullptr;
        rootTerminal  = nullptr;
        rootShutdown  = nullptr;
        shutdown(); // the tail of Application::Run / RunSingleApp: UnInit, Log::Unit, delete app
    }
}

// Never returns on success: simulate_infinite_loop unwinds main() (code after Application::Run() does not run,
// exactly like a native Run() that only returns when the application ends).
bool BrowserTerminal::RunRootEventLoop(ApplicationImpl& app, void (*shutdown)())
{
    CHECK(rootApp == nullptr, false, "The browser root loop is already running !");
    CHECK(shutdown != nullptr, false, "Expecting a valid shutdown callback !");
    CHECK(app.terminal.get() == this, false, "The root loop must be driven by the application's own terminal !");

    rootFrame             = EventLoopFrame{};
    rootFrame.ctrl        = nullptr;
    rootFrame.nonBlocking = true;
    CHECK(app.EnterEventLoop(rootFrame, false), false, "Fail to enter the root event loop !");

    rootApp      = &app;
    rootTerminal = this;
    rootShutdown = shutdown;
    emscripten_set_main_loop(BrowserMainLoopTick, 0 /* requestAnimationFrame */, true /* simulate infinite loop */);
    return true; // unreachable
}
} // namespace AppCUI::Internal
```

> **Failure semantics:** if a `CHECK` fails *before* `EnterEventLoop`, the hook returns `false` and `Run()` falls back to the blocking loop. In a browser that would freeze the tab, so the browser override never fails "softly" after `EnterEventLoop` succeeds. The only `false` returns come before any state has changed, and they all indicate programming errors that are logged.

`Application.cpp` factors the existing teardown into two `static` functions and asks the terminal first. There is no `#ifdef`:

```cpp
static void ShutdownAfterRun()        // the existing tail of Application::Run, verbatim
{
    LOG_INFO("Uninit text field/area default menu ");
    Controls::UninitTextFieldDefaultMenu();
    Controls::UninitTextAreaDefaultMenu();
    LOG_INFO("Starting to un-init AppCUI ...");
    app->UnInit();
    Log::Unit();
    LOG_INFO("Uninit successfully");
    delete app;
    app = nullptr;
}

bool Application::Run()
{
    CHECK(app, false, "Application has not been initialized !");
    CHECK(app->Inited, false, "Application has not been corectly initialized !");
    app->loopStatus = LoopStatus::Normal;
    if (app->terminal->RunRootEventLoop(*app, ShutdownAfterRun))
        return true; // the frontend drives the loop (and calls ShutdownAfterRun when it ends)
    app->ExecuteEventLoop();
    ShutdownAfterRun();
    return true;
}
// RunSingleApp: same pattern with its own (slightly different) existing tail -> ShutdownAfterSingleApp
```

### 6.4 How each scenario now executes

| Scenario | What happens |
|---|---|
| Idle root loop | Every rAF: one `StepEventLoop`. Nothing to paint and an empty queue means `GetSystemEvent` returns `None` immediately because `nonBlocking == true`. Cost is a handful of branches per frame. |
| Burst of input | One tick drains up to 64 steps while input or repaint is pending. All flushes in the tick are coalesced into one microtask draw. |
| Click → `MessageBox::ShowOkCancel` | Inside the tick, `Window::Show` → `ExecuteEventLoop` (nested frame, `nonBlocking = false`). With an empty queue, `GetSystemEvent` calls `appcui_js_wait_for_input`. Asyncify unwinds the stack up to the tick and `MainLoop.pause()` runs. Input arrives, `wake()` resolves the promise, the stack rewinds, and the loop continues. When the dialog closes, `Show()` returns its result to the original caller, the tick finishes, and `MainLoop.resume()` runs. |
| Modal before `Run()` | The same mechanism, suspending `main()` itself. `Run()` is reached after the dialog closes. |
| `EnableFPSMode` (Tetris, Snake, BadApple) | Root: `StampFrameUpdate` sets `updateFrames` every 33 ms, sampled on 60 Hz rAF ticks. Modal: the wait timeout is `MillisecondsUntilNextFrame()`, so animations keep running under dialogs. |
| Tab hidden | rAF stops, so the root loop pauses (the native SDL equivalent is a minimised window). Modal loops keep their `setTimeout` cadence, which browsers throttle to ≥ 1 s. That is acceptable. |
| Window close / reload | Optional: `pagehide` → `_appcui_browser_close()`. In practice the page is torn down. |

### 6.5 Asyncify configuration

```
-sASYNCIFY=1
-sASYNCIFY_STACK_SIZE=131072        # deep control trees + nested modals; tune by measuring with ASSERTIONS=2
-sSTACK_SIZE=1048576
```

- `EM_ASYNC_JS` imports are added to `ASYNCIFY_IMPORTS` automatically. Do **not** use `ASYNCIFY_ONLY`: any virtual `OnEvent`/`OnKeyEvent`/command handler can call `Show()`, so the set of functions on the stack during a suspend is effectively all UI code.
- Optional size tuning: `-sASYNCIFY_REMOVE=['AppCUI::Graphics::*']` (painting never opens a modal). Validate it with `-sASSERTIONS=2`, which traps if a removed function is ever on the stack during a suspend.
- **Exceptions:** use `-fwasm-exceptions`. Binaryen's Asyncify cannot unwind *from inside a `catch` block*, so add a rule: **never call `Window::Show()` (or anything modal) inside a `catch` handler.** None of the 16 current catch sites do this, but add a review note. If the pinned emsdk rejects `ASYNCIFY` + `-fwasm-exceptions`, fall back to `-fexceptions` (JS-based EH, slower only inside `try` regions, none of which are hot).

### 6.6 JSPI variant (opt-in)

JSPI's `handleSleep` does not pause `MainLoop`. If a modal suspends inside a rAF tick, the next rAF would call `BrowserMainLoopTick` *re-entrantly*, stepping the root frame while a nested frame is live, which would corrupt the modal stack. Guard it:

```cpp
static uint32 suspendedModalDepth = 0; // ++ before appcui_js_wait_for_input, -- after (BrowserTerminal)
// in BrowserMainLoopTick: if (suspendedModalDepth != 0) return;   // root is frozen while a modal waits
```

With that guard, JSPI is behaviourally identical to Asyncify. It still ships behind `APPCUI_WASM_JSPI=ON` until M1 passes.

---

## 7. Build integration (on top of the component layout from `f7604518`)

The browser build is **the same `AppCUI/CMakeLists.txt` with one more component and three Emscripten branches**. There is no separate CMake path and no early `return()`.

### 7.1 Component options

```cmake
# defaults: a browser build gets the Browser frontend and nothing that needs a native window system / tty
if (EMSCRIPTEN)
    set(APPCUI_FRONTEND_SDL_DEFAULT OFF)
    set(APPCUI_FRONTEND_NCURSES_DEFAULT OFF)   # UNIX is TRUE under emcmake -> must be overridden explicitly
    set(APPCUI_FRONTEND_BROWSER_DEFAULT ON)
else()
    set(APPCUI_FRONTEND_SDL_DEFAULT ON)
    set(APPCUI_FRONTEND_BROWSER_DEFAULT OFF)
    # APPCUI_FRONTEND_NCURSES_DEFAULT / _WINDOWS_CONSOLE_DEFAULT: unchanged
endif()
option(APPCUI_FRONTEND_SDL "Link the SDL2 frontend (AppCUI::Frontend::SDL) into libAppCUI" ${APPCUI_FRONTEND_SDL_DEFAULT})
option(APPCUI_FRONTEND_BROWSER "Link the HTML5 canvas frontend (AppCUI::Frontend::Browser) into libAppCUI" ${APPCUI_FRONTEND_BROWSER_DEFAULT})

if (APPCUI_FRONTEND_BROWSER AND NOT EMSCRIPTEN)
    message(FATAL_ERROR "${PROJECT_NAME} => APPCUI_FRONTEND_BROWSER is only available for wasm32-emscripten")
endif()
if (EMSCRIPTEN AND (APPCUI_FRONTEND_SDL OR APPCUI_FRONTEND_NCURSES OR APPCUI_FRONTEND_WINDOWS_CONSOLE))
    message(FATAL_ERROR "${PROJECT_NAME} => only AppCUI::Frontend::Browser is supported for wasm32-emscripten")
endif()
```

### 7.2 The new component

```cmake
# ---------------------------------------------------------------------------------------------------------------------
# AppCUI::Frontend::Browser
# ---------------------------------------------------------------------------------------------------------------------
if (APPCUI_FRONTEND_BROWSER)
    add_library(AppCUIFrontendBrowser OBJECT)
    add_library(AppCUI::Frontend::Browser ALIAS AppCUIFrontendBrowser)
    appcui_configure_component(AppCUIFrontendBrowser)
    target_link_libraries(AppCUIFrontendBrowser PRIVATE AppCUICore)
    # new sources are held to zero warnings from day one
    target_compile_options(AppCUIFrontendBrowser PRIVATE -Werror)
endif()
```

```cmake
# src/Terminal/CMakeLists.txt (next to the other components)
if (TARGET AppCUIFrontendBrowser)
    add_subdirectory(BrowserTerminal)
endif()

# src/Terminal/BrowserTerminal/CMakeLists.txt
target_sources(AppCUIFrontendBrowser PRIVATE BrowserTerminal.cpp BrowserExports.cpp BrowserMainLoop.cpp)
```

Add `Browser` to the `foreach(frontend SDL Ncurses WindowsConsole)` loop that links the components into `libAppCUI`, and to the MSVC `source_group` list for symmetry.

### 7.3 Emscripten branches in `AppCUI::Core`

| Today (`f7604518`) | Emscripten change | Why |
|---|---|---|
| `find_package(JPEG/libjpeg-turbo/PNG REQUIRED)` | New option `APPCUI_IMAGE_CODECS` (default ON, **OFF under Emscripten**). When OFF, skip the `find_package` calls and compile `Graphics/ImageCodecsStub.cpp` instead of `PNGLoader.cpp`/`JPGLoader.cpp`. The stub's `LoadPNGToImage`/`LoadJPGToImage` return `RETURNERROR(false, "… not available in this build")`. | BMP/DIB keeps working. Phase 2: `-sUSE_LIBPNG=1`, a turbojpeg port, or decoding via `createImageBitmap` |
| `if (UNIX AND NOT APPLE) … ${CMAKE_DL_LIBS}` and `stdc++fs` | Add `AND NOT EMSCRIPTEN` | Emscripten's libc++ includes `<filesystem>`, so `-lstdc++fs` does not exist. `dlopen` stays as a link-time stub (`Library.cpp` fails at runtime with a logged error) |
| `add_library(AppCUI SHARED)` | `STATIC` under Emscripten, consumed **whole-archive** (7.4) | Emscripten links one module; there are no shared libraries |

### 7.4 Keeping the registrar alive (whole-archive)

The split chose OBJECT libraries precisely because *"an archive (STATIC library) member that nothing references would be silently dropped"*. In a static `libAppCUI.a`, nothing references `BrowserTerminal.o`, because the factory only finds it through its static `TerminalRegistrar`. The linker would drop it, and `Default` would fail with "no native frontend is linked". So consumers link the archive whole:

```cmake
if (EMSCRIPTEN)
    add_library(AppCUIStatic STATIC)
    # ... same sources / components / properties that the SHARED target receives today ...

    # wasm-ld understands --whole-archive; declare the CMake link feature in case the toolchain does not
    if (NOT CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED)
        set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:--whole-archive" "<LINK_ITEM>" "LINKER:--no-whole-archive")
        set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
    endif()

    add_library(AppCUI INTERFACE)          # what examples / consumers link, exactly as before
    target_link_libraries(AppCUI INTERFACE "$<LINK_LIBRARY:WHOLE_ARCHIVE,AppCUIStatic>")
    target_include_directories(AppCUI INTERFACE "${INCLUDE_DIR}")
endif()
```

`$<LINK_LIBRARY>` needs **CMake ≥ 3.24**. Put `cmake_minimum_required(VERSION 3.24)` inside the `if (EMSCRIPTEN)` branch only, so the native minimum stays at 3.13.

As a belt-and-braces check, the smoke test in §8 calls `Application::Init()` with `FrontendType::Default`. If a registrar ever gets dropped, it fails loudly with the registry's own error message.

### 7.5 Runtime contract (link options inherited by every executable)

```cmake
option(APPCUI_WASM_JSPI "Use JSPI instead of Asyncify for modal loops" OFF)
if (APPCUI_WASM_JSPI)
    set(APPCUI_WASM_SUSPEND -sJSPI=1)
else()
    set(APPCUI_WASM_SUSPEND -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=131072)
endif()

target_compile_options(AppCUIStatic PUBLIC -fwasm-exceptions)
target_link_options(AppCUI INTERFACE
    -fwasm-exceptions
    ${APPCUI_WASM_SUSPEND}
    -sSTACK_SIZE=1048576
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=33554432
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createAppCUIModule
    -sENVIRONMENT=web
    -sEXPORTED_RUNTIME_METHODS=HEAPU8,HEAPU32
    -sFILESYSTEM=1)                       # std::filesystem -> MEMFS (FileDialog browses an in-memory FS)
```

`-fwasm-exceptions` must also reach the OBJECT components, because every translation unit has to agree on the exception model. Apply it in `appcui_configure_component()` under `if (EMSCRIPTEN)`.

### 7.6 Examples and the top level

| File | Change |
|---|---|
| Top-level `CMakeLists.txt` | `set(CMAKE_TOOLCHAIN_FILE … vcpkg … CACHE STRING …)` is already a no-op under `emcmake`, because the `-DCMAKE_TOOLCHAIN_FILE=<emsdk>/Emscripten.cmake` cache entry wins and vcpkg is never engaged. Under `EMSCRIPTEN`, add only a curated example list (skip examples that need `dlopen` or real files) |
| `cmake/example.cmake` | `if (EMSCRIPTEN)`: `SUFFIX ".mjs"` and copy `appcui-host.js`, `index.html` and `JuliaMono-Regular.ttf` next to the output |

```bash
emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build-wasm --target MessageBox
```

**Warnings gate:** `-Werror` is applied to the Browser component only (7.2). CI builds the wasm target with the project's full warning set and diffs the warning count against `main` for the core sources compiled by Emscripten's Clang.

---

## 8. Verification plan

| Milestone | Acceptance |
|---|---|
| **M0: refactor only** (native, no wasm) | Enter/Step/Leave split, `nonBlockingEvents` and the `RunRootEventLoop` hook land. All `Tests/Tester` screen-hash scripts are bit-identical on Windows and Linux. Zero new warnings on MSVC `/W3` and GCC/Clang with the project flags. |
| **M0.5: link smoke test** | The wasm build of `MessageBox` links. Under Node (`-sENVIRONMENT=node` test build), `Application::Init()` with `FrontendType::Default` resolves to `Browser`, proving the whole-archive link kept the registrar. A native build with `-DAPPCUI_FRONTEND_BROWSER=ON` fails at configure time with the intended message. |
| **M1: suspension spike** | `Examples/MessageBox` in Chrome, Firefox and Safari: (a) modal before `Run()`, (b) modal opened from a click inside a rAF tick, (c) modal opened from a modal (depth 3), (d) `FileDialog` (stack-allocated window) returns a path, (e) closing a dialog resumes the root loop. Repeat with `APPCUI_WASM_JSPI=ON`. |
| **M2: renderer** | Snapshot test in headless Chromium (Playwright): render `Examples/CharacterTable` at dpr 1, 1.5 and 2 and pixel-diff against goldens. No seams in `Examples/Rectangles` box/block art. The DevTools Performance panel shows **0 B allocated per frame** while idle and **0 GC events** during a 10 s drag in `WindowManager`. |
| **M3: input parity** | `Examples/KeySelector` shows the same `Key` values as the Windows console for the full key matrix (F-keys, Ctrl/Alt/Shift combos, AltGr text, BMP non-ASCII). Mouse double-click opens items in `ListView`/`TreeView`. |
| **M4: FPS mode** | `Tetris`/`Snake` keep ~30 fps of `OnFrameUpdate` while a modal is open (timer cadence) and while idle (rAF cadence). |
| **Fuzz** | A Node harness drives every export with random 32-bit values (`NaN`, `±Infinity`, `2**32+n`, negatives) and checks for no crash, no ASan report (`-fsanitize=address` wasm build) and return value 0 for every invalid input. |

---

## 9. Risks and open questions

| Risk | Mitigation |
|---|---|
| Asyncify code size (+30–50 %) | `ASYNCIFY_REMOVE` for `Graphics::*` (validated with `ASSERTIONS=2`), `-Oz` and `wasm-opt`. Move to JSPI once the M1 matrix is green. |
| `ASYNCIFY` + `-fwasm-exceptions` rejected by the pinned emsdk | Fall back to `-fexceptions`. Pin the emsdk version in CI. |
| Browser-reserved shortcuts (Ctrl+W/T/N, Ctrl+Tab) never reach the page | Document them. Ship a `[AppCUI] Keyboard.Ctrl` profile for web builds, which the existing `LoadKeyboardSettings` already supports, to remap AppCUI's Ctrl bindings. |
| Clipboard | v1 uses the process-private clipboard, as `Custom` already does. v2 bridges DOM `copy`/`paste` events, which grant synchronous `clipboardData` access, into a fixed C++ buffer through one more export. |
| `fillText` throughput on 4K screens with full redraws (theme change, resize) | Full redraws are rare. If profiling shows a problem, add a glyph atlas (`OffscreenCanvas`, keyed by code and fg, drawn with `drawImage`) behind the same `paintCells` loop. |
| `std::filesystem` on MEMFS shows an empty tree in `FileDialog` | Expected in v1. Phase 2: an IDBFS mount or OPFS-backed WasmFS. |
| SDL wheel direction looks inverted relative to the DOM | Resolve in M3 against `WindowsTerminal`. |

---

## 10. Implementation order

0. **Done:** the frontend split (`f7604518`) gives the component layout, the registry and the terminal-owned clipboard.
1. **M0:** the Enter/Step/Leave refactor, `nonBlockingEvents` and the `RunRootEventLoop` hook (native only, a standalone PR, gated by the test hashes).
2. **Build path:** `FrontendType::Browser` (+ registry slot and `Properties.cpp` limit), the `APPCUI_FRONTEND_BROWSER` component, the Emscripten defaults, the `APPCUI_IMAGE_CODECS` option, the `stdc++fs`/`dl` guards and the static whole-archive `libAppCUI`. Exit criterion: M0.5.
3. The `AppCUI::Frontend::Browser` sources: `BrowserTerminal`, `BrowserExports`, `BrowserMainLoop`.
4. `appcui-host.ts` + `index.html` + example packaging.
5. M1–M4 verification, the fuzz harness, and a CI job that builds `wasm32` and runs Playwright snapshots.
