#include "BrowserTerminal.hpp"
#include <emscripten/emscripten.h>

// JS -> C++ entry points used by appcui-host.js. Every argument is a number that went through JS ToInt32 /
// ToUint32 (truncated, wrapped modulo 2^32, NaN -> 0), so EVERY value is validated here, never in JS.
// Return value of the input exports: 1 = accepted, 0 = rejected (invalid value, no terminal or queue full).

namespace AppCUI::Internal
{
// the key table of appcui-host.js is pinned to these values (a change here must bump BROWSER_ABI_VERSION)
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
static_assert(static_cast<uint32>(Input::MouseButton::Left) == 0x01);
static_assert(static_cast<uint32>(Input::MouseButton::Center) == 0x02);
static_assert(static_cast<uint32>(Input::MouseButton::Right) == 0x04);
static_assert(static_cast<uint32>(Input::MouseButton::DoubleClicked) == 0x08);
static_assert(static_cast<uint32>(Input::MouseWheel::Up) == 1);
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
    // size is in [1, BROWSER_MAX_*] while a terminal is alive -> (size - 1) always fits in an int32
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
    if ((term == nullptr) || (!IsValidKey(key)) || (!IsValidBmpCharacter(unicodeChar)))
        return 0;
    if (((key & KEY_CODE_MASK) == 0) && (unicodeChar == 0))
        return 0; // nothing to report (pure modifier changes go through appcui_browser_shift_state)
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
    constexpr SystemEventType MOUSE_EVENT_TYPES[3] = { SystemEventType::MouseDown,
                                                       SystemEventType::MouseUp,
                                                       SystemEventType::MouseMove };
    auto* term = GetActiveBrowserTerminal();
    if ((term == nullptr) || (type > 2u) || ((buttons & ~MOUSE_BUTTONS_MASK) != 0) ||
        ((modifiers & ~KEY_MODIFIERS_MASK) != 0))
        return 0;
    auto e   = MakeEvent(MOUSE_EVENT_TYPES[type]);
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
