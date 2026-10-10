#include "Internal.hpp"

namespace AppCUI::OS
{
using namespace Utils;

#ifdef _WIN32
static bool CopyTextBufferToClipboard(const void* buf, size_t characterSize, size_t length)
{
    HANDLE hMem;
    CHECK((hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, (length + 1) * characterSize)),
          false,
          "Fail to allocate %z bytes in data memory to copy a string",
          length + 1);
    void* temp = (void*) GlobalLock(hMem);
    if (temp)
    {
        memcpy(temp, buf, length * characterSize);
        if (characterSize == 1)
            ((uint8*) temp)[length] = 0;
        else if (characterSize == 2)
            ((uint16*) temp)[length] = 0;
    }
    GlobalUnlock(hMem);
    CHECK(temp, false, "Global Lock failed !");

    CHECK(OpenClipboard(nullptr), false, "Fail to open the clipboard object !");
    EmptyClipboard();

    // copy to clipboard --> two formats (ascii and unicode)
    HANDLE h = nullptr;
    if (characterSize == 1)
        h = SetClipboardData(CF_TEXT, hMem);
    else if (characterSize == 2)
        h = SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    CHECK(h != nullptr,
          false,
          "Fail to copy text data into clipboard (Error: %d) or invalid character size (expecting either 1 or 2)",
          GetLastError());

    return true;
}
#endif

// The clipboard belongs to the frontend: the terminal of the running application decides which clipboard is used
// (the OS one, the one of SDL, a process-private one for a custom frontend, none for the test terminal, ...).
static Internal::AbstractTerminal* GetClipboardTerminal()
{
    auto app = Application::GetApplication();
    CHECK(app != nullptr, nullptr, "AppCUI is not initialized (there is no clipboard without a frontend) !");
    CHECK(app->terminal != nullptr, nullptr, "AppCUI has no terminal (there is no clipboard without a frontend) !");
    return app->terminal.get();
}

bool Clipboard::Clear()
{
    auto terminal = GetClipboardTerminal();
    return terminal ? terminal->ClipboardClear() : false;
}

bool Clipboard::HasText()
{
    auto terminal = GetClipboardTerminal();
    return terminal ? terminal->ClipboardHasText() : false;
}

bool Clipboard::SetText(const ConstString& text)
{
    auto terminal = GetClipboardTerminal();
    return terminal ? terminal->ClipboardSetText(text) : false;
}

bool Clipboard::GetText(Utils::UnicodeStringBuilder& text)
{
    auto terminal = GetClipboardTerminal();
    return terminal ? terminal->ClipboardGetText(text) : false;
}
} // namespace AppCUI::OS

namespace AppCUI::Internal
{
using namespace Utils;

// Default implementation: the clipboard of the OS (Win32). Other platforms have no OS clipboard support (yet).
bool AbstractTerminal::ClipboardClear()
{
#ifdef _WIN32
    CHECK(OpenClipboard(nullptr), false, "Fail to open the clipboard object !");
    EmptyClipboard();
    CloseClipboard();
#endif
    return true;
}

bool AbstractTerminal::ClipboardHasText()
{
#ifdef _WIN32
    return ((IsClipboardFormatAvailable(CF_TEXT)) || (IsClipboardFormatAvailable(CF_UNICODETEXT)));
#else
    return false;
#endif
}

bool AbstractTerminal::ClipboardSetText([[maybe_unused]] const ConstString& text)
{
#ifdef _WIN32
    ConstStringObject textObj(text);
    CHECK(textObj.Data != nullptr, false, "Text should be different than nullptr !");

    Utils::LocalUnicodeStringBuilder<1024> unicode;
    if (textObj.Encoding == StringEncoding::Ascii)
        return OS::CopyTextBufferToClipboard(textObj.Data, sizeof(char), textObj.Length);
    CHECK(unicode.Set(text), false, "Fail to convert ConstString into unicode buffer !");
    return OS::CopyTextBufferToClipboard(unicode.GetString(), sizeof(char16), unicode.Len());
#else
    return false;
#endif
}

bool AbstractTerminal::ClipboardGetText([[maybe_unused]] Utils::UnicodeStringBuilder& text)
{
#ifdef _WIN32
    CHECK(OpenClipboard(nullptr), false, "Fail to open clipboard object !");
    while (true)
    {
        const char16* temp = (const char16*) GetClipboardData(CF_UNICODETEXT);
        CHECKBK(temp, "Invalid object received from clipboard data (null)");
        CHECKBK(text.Set(temp), "Fail to copy text into String buffer !");
        CloseClipboard();
        return true;
    }
    CloseClipboard();
    return false;
#else
    return false;
#endif
}
} // namespace AppCUI::Internal
