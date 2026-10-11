#include "BrowserTerminal.hpp"
#include "../TerminalFactory.hpp"
#include <emscripten/emscripten.h>
#include <cstdlib>
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

// Host file transfer (FileDialog Upload / Download, OS::DownloadFile). Strings cross the boundary as UTF-8 pointers.
EM_JS_DEPS(appcui_browser_files, "$FS,$UTF8ToString,$stringToUTF8,$lengthBytesUTF8");
// Lets the user pick files on the host and writes them into `dir` (MEMFS keeps the bytes in JS memory -> the wasm heap
// does not grow). The names written are kept in Module.appcuiUploaded. Returns 0 (done or cancelled), -1 (write
// error), -2 (a file is larger than maxBytes -> nothing is written), -3 (no host page).
EM_ASYNC_JS(int32_t, appcui_js_upload, (const char* dir, const char* accept, int32_t multiple, uint32_t maxBytes), {
    const host = Module["appcui"];
    const written = [];
    Module["appcuiUploaded"] = written;
    if (!host || !host.pickFiles) return -3;
    const files = await host.pickFiles(UTF8ToString(accept), multiple !== 0);
    for (const file of files) {
        if (file.size > (maxBytes >>> 0)) return -2;
    }
    let folder = UTF8ToString(dir);
    if (!folder.endsWith("/")) folder += "/";
    try {
        for (const file of files) {
            // a single path component: control characters, "/" (47) and backslash (92) become "_" (an EM_JS body
            // must not contain a backslash: the preprocessor would turn it into a C escape sequence)
            let name = "";
            for (const ch of String(file.name)) {
                const code = ch.codePointAt(0);
                name += (code < 32 || code === 47 || code === 92) ? "_" : ch;
            }
            if (name === "" || name === "." || name === "..") name = "upload.bin";
            FS.writeFile(folder + name, new Uint8Array(await file.arrayBuffer()));
            written.push(name);
        }
    } catch (e) {
        console.error("AppCUI: upload failed", e);
        return -1;
    }
    return 0;
});
EM_JS(uint32_t, appcui_js_uploaded_count, (), {
    const written = Module["appcuiUploaded"];
    return written ? written.length : 0;
});
// Copies the index-th uploaded name (NUL terminated UTF-8) into buffer. Returns 0 if it does not exist / does not fit.
EM_JS(uint32_t, appcui_js_uploaded_name, (uint32_t index, char* buffer, uint32_t capacity), {
    const written = Module["appcuiUploaded"];
    const name = written ? written[index >>> 0] : undefined;
    if (name === undefined || lengthBytesUTF8(name) + 1 > (capacity >>> 0)) return 0;
    stringToUTF8(name, buffer, capacity >>> 0);
    return 1;
});
// Reads `path` from MEMFS and hands it to the browser as a download named `name`. Returns 1 on success.
EM_JS(uint32_t, appcui_js_download, (const char* path, const char* name), {
    const host = Module["appcui"];
    if (!host || !host.download) return 0;
    try {
        host.download(UTF8ToString(name), FS.readFile(UTF8ToString(path)));
        return 1;
    } catch (e) {
        console.error("AppCUI: download failed", e);
        return 0;
    }
});
// clang-format on

namespace AppCUI::Internal
{
using namespace Application;

constexpr uint32 FRAME_INTERVAL_MS = 33;   // ~30 fps, same cadence as SDLTerminal / CustomTerminal
constexpr uint32 IDLE_WAIT_MS      = 1000; // modal loop without FPS mode: wake up at least once per second

constexpr uint32 MAX_UPLOAD_BYTES       = 256u * 1024u * 1024u; // per file (MEMFS keeps uploads in the page memory)
constexpr double SAVE_EXPORT_TIMEOUT_MS = 30000.0;              // a Save not written within 30 s is never downloaded

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
    : pendingSaveTime(), pendingSaveSize(0), pendingSaveDeadlineMs(0.0), lastFramesUpdateMs(0.0), pendingWidth(0),
      pendingHeight(0), suspendedWaits(0), resizePending(false), closePending(false), redrawPending(false),
      fpsMode(false), pendingSaveExisted(false)
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

    // Emscripten starts in "/" (dev, proc, tmp, home): unless the application already moved, work from the home
    // folder -> relative paths and the default location of FileDialog are where the uploaded files of the user go
    std::error_code err;
    const char* home = std::getenv("HOME");
    if ((home != nullptr) && (home[0] != 0) && (std::filesystem::current_path(err) == "/") && (!err))
    {
        std::filesystem::create_directories(home, err);
        std::filesystem::current_path(home, err);
        if (err)
            LOG_WARNING("Unable to change the current directory to %s", home);
    }

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
    appcui_js_on_exit(); // first: the header and the cells are still valid -> the host can paint the final frame
    activeTerminal    = nullptr;
    frameHeader.magic = 0;
    browserClipboard.clear();
    browserClipboard.shrink_to_fit(); // a later Application::Init must not inherit the clipboard
    pendingSave.clear();              // ... nor a pending download
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
        // idle: the handler that saved the file (and any modal shown from it) has returned -> the file is complete
        if ((!hasEvent) && (!this->pendingSave.empty()))
            CheckPendingSave();
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

//===================================================================================== host file transfer ===
static bool IsValidUploadedName(std::string_view name)
{
    // the name comes from the host page (untrusted): a single path component, never "." / ".."
    if (name.empty() || (name == ".") || (name == ".."))
        return false;
    return name.find_first_of("/\\") == std::string_view::npos;
}
bool BrowserTerminal::HostFileTransferSupported()
{
    return true;
}
bool BrowserTerminal::UploadFromHost(
      const std::filesystem::path& destFolder,
      std::string_view accept,
      bool multiple,
      vector<std::filesystem::path>& uploaded,
      std::string& error)
{
    uploaded.clear();
    const std::string folder = destFolder.string(); // MEMFS paths are UTF-8
    const std::string acceptFilter(accept);         // NUL terminated copy
    const int32 status = appcui_js_upload(folder.c_str(), acceptFilter.c_str(), multiple ? 1 : 0, MAX_UPLOAD_BYTES);

    // also after a failure: the files written before it are in the folder
    char name[1024];
    const uint32 count = appcui_js_uploaded_count();
    for (uint32 index = 0; index < count; index++)
    {
        if (appcui_js_uploaded_name(index, name, sizeof(name)) == 0)
            continue;
        const std::string_view nameView(name);
        std::error_code err;
        if ((!IsValidUploadedName(nameView)) || (!std::filesystem::is_regular_file(destFolder / nameView, err)))
        {
            LOG_WARNING("Ignoring the uploaded file: %s", name);
            continue;
        }
        uploaded.emplace_back(nameView);
    }

    switch (status)
    {
    case 0:
        return true;
    case -2:
        error = "The file is too large (the upload limit is " + std::to_string(MAX_UPLOAD_BYTES >> 20) + " MB)";
        return false;
    case -3:
        error = "The page does not support file uploads";
        return false;
    default:
        error = "Unable to write the uploaded file(s) into " + folder;
        return false;
    }
}
bool BrowserTerminal::DownloadToHost(const std::filesystem::path& file)
{
    std::error_code err;
    CHECK(std::filesystem::is_regular_file(file, err), false, "Not a file: %s", file.string().c_str());
    const std::string path = file.string();
    const std::string name = file.filename().string();
    return appcui_js_download(path.c_str(), name.c_str()) != 0;
}
void BrowserTerminal::OnSaveFileSelected(const std::filesystem::path& file)
{
    // snapshot: an existing file is exported only once the application has rewritten it
    std::error_code err;
    this->pendingSave        = file;
    this->pendingSaveExisted = std::filesystem::is_regular_file(file, err);
    this->pendingSaveSize    = this->pendingSaveExisted ? std::filesystem::file_size(file, err) : 0;
    this->pendingSaveTime =
          this->pendingSaveExisted ? std::filesystem::last_write_time(file, err) : std::filesystem::file_time_type();
    this->pendingSaveDeadlineMs = emscripten_get_now() + SAVE_EXPORT_TIMEOUT_MS;
}
void BrowserTerminal::CheckPendingSave()
{
    std::error_code err;
    if (std::filesystem::is_regular_file(this->pendingSave, err))
    {
        const auto size = std::filesystem::file_size(this->pendingSave, err);
        // keep the first error: a successful call resets `err`
        const auto time =
              err ? std::filesystem::file_time_type() : std::filesystem::last_write_time(this->pendingSave, err);
        const bool changed =
              (!this->pendingSaveExisted) || (size != this->pendingSaveSize) || (time != this->pendingSaveTime);
        if ((!err) && changed)
        {
            const std::filesystem::path file = std::move(this->pendingSave);
            this->pendingSave.clear();
            DownloadToHost(file);
            return;
        }
    }
    if (emscripten_get_now() >= this->pendingSaveDeadlineMs)
        this->pendingSave.clear(); // never written (failed / cancelled by the application)
}

// AppCUI::Frontend::Browser -> FrontendType::Browser, and FrontendType::Default (the only native frontend of a
// wasm32-emscripten build)
static const TerminalRegistrar browserTerminalRegistrar(
      Application::FrontendType::Browser, CreateTerminal<BrowserTerminal>, DefaultFrontendPriority::Native);
} // namespace AppCUI::Internal
