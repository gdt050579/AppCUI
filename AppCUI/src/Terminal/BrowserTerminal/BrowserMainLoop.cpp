#include "BrowserTerminal.hpp"
#include <emscripten/emscripten.h>

namespace AppCUI::Internal
{
#if defined(APPCUI_WASM_JSPI)

// JSPI can only suspend inside a promising export - main() - and an emscripten main-loop callback is not one. The
// root loop therefore simply blocks: waiting for input suspends main() exactly like a modal loop does, and
// Application::Run() returns normally once the application closes.
bool BrowserTerminal::RunRootEventLoop(ApplicationImpl&, void (*)())
{
    return false;
}

#else

constexpr uint32 MAX_STEPS_PER_TICK = 64; // bounds a tick even if a control keeps requesting repaints
// setTimeout cadence, not requestAnimationFrame: rAF stops while a page is not composited (occluded / minimized
// windows, some embedders and automation hosts) although input keeps arriving, which would freeze the application.
// Painting does not need rAF either - the host draws from a microtask after every flush.
constexpr int ROOT_LOOP_FPS = 60;

static EventLoopFrame rootFrame{}; // static storage: survives the unwinding of main()
static ApplicationImpl* rootApp      = nullptr;
static BrowserTerminal* rootTerminal = nullptr;
static void (*rootShutdown)()        = nullptr;

// One tick of the ROOT loop (Asyncify). Modal loops opened from inside a tick run nested: their GetSystemEvent
// suspends the stack while idle and Asyncify pauses this main loop meanwhile; the guard below is a second line of
// defence that keeps the root frame from being stepped while a modal frame is still alive.
static void BrowserMainLoopTick()
{
    if ((rootApp == nullptr) || (rootTerminal->IsSuspendedInModalLoop()))
        return;
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

// Never returns on success: simulate_infinite_loop unwinds main(), so code placed after Application::Run() does not
// run - exactly like a native Run() that only returns once the application ends.
bool BrowserTerminal::RunRootEventLoop(ApplicationImpl& app, void (*shutdown)())
{
    CHECK(rootApp == nullptr, false, "The browser root loop is already running !");
    CHECK(shutdown != nullptr, false, "Expecting a valid shutdown callback !");
    CHECK(app.terminal.get() == this, false, "The root loop must be driven by the terminal of the application !");

    rootFrame             = EventLoopFrame{};
    rootFrame.ctrl        = nullptr;
    rootFrame.nonBlocking = true;
    CHECK(app.EnterEventLoop(rootFrame, false), false, "Fail to enter the root event loop !");

    rootApp      = &app;
    rootTerminal = this;
    rootShutdown = shutdown;
    emscripten_set_main_loop(BrowserMainLoopTick, ROOT_LOOP_FPS, true /* simulate infinite loop */);
    return true; // unreachable
}

#endif
} // namespace AppCUI::Internal
