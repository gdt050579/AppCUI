#include "Internal.hpp"
#include "SDL.h"

namespace AppCUI::OS
{
using namespace Utils;

bool CopyTextBufferToClipboard(const void* buf, size_t characterSize, size_t length)
{
#ifdef _WIN32
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
#endif

    return false;
}

bool Clipboard::Clear()
{
    const auto& frontend = Application::GetApplication()->GetFrontendType();
#ifdef _WIN32
    if (frontend == AppCUI::Application::FrontendType::Default ||
        frontend == AppCUI::Application::FrontendType::Terminal ||
        frontend == AppCUI::Application::FrontendType::WindowsConsole)
    {
        CHECK(OpenClipboard(nullptr), false, "Fail to open the clipboard object !");
        EmptyClipboard();
        CloseClipboard();
    }
    else if (frontend == AppCUI::Application::FrontendType::SDL)
    {
        CHECK(SDL_SetClipboardText("") > 0, false, "Failed to clear clipboard: %s", SDL_GetError());
    }
#else

#endif

    return true;
}

bool Clipboard::HasText()
{
    const auto& frontend = Application::GetApplication()->GetFrontendType();
#ifdef _WIN32

    if (frontend == AppCUI::Application::FrontendType::Default ||
        frontend == AppCUI::Application::FrontendType::Terminal ||
        frontend == AppCUI::Application::FrontendType::WindowsConsole)
    {
        return ((IsClipboardFormatAvailable(CF_TEXT)) || (IsClipboardFormatAvailable(CF_UNICODETEXT)));
    }
    else if (frontend == AppCUI::Application::FrontendType::SDL)
    {
        return SDL_HasClipboardText();
    }
#else
    return false;
#endif

    return false;
}

bool Clipboard::SetText(const ConstString& text)
{
    const auto& frontend = Application::GetApplication()->GetFrontendType();
#ifdef _WIN32
    ConstStringObject textObj(text);
    if (frontend == AppCUI::Application::FrontendType::Default ||
        frontend == AppCUI::Application::FrontendType::Terminal ||
        frontend == AppCUI::Application::FrontendType::WindowsConsole)
    {
        CHECK(textObj.Data != nullptr, false, "Text should be different than nullptr !");

        Utils::LocalUnicodeStringBuilder<1024> unicode;
        if (textObj.Encoding == StringEncoding::Ascii)
            return CopyTextBufferToClipboard(textObj.Data, sizeof(char), textObj.Length);
        CHECK(unicode.Set(text), false, "Fail to convert ConstString into unicode buffer !");
        return CopyTextBufferToClipboard(unicode.GetString(), sizeof(char16), unicode.Len());
    }
    else if (frontend == AppCUI::Application::FrontendType::SDL)
    {
        if (textObj.Encoding == StringEncoding::Ascii)
        {
            // GDT: temporary fix - we can't guarantee that text is a NULL terminated string
            CHECK(SDL_SetClipboardText((const char*) textObj.Data) > 0,
                  false,
                  "Failed to set clipboard text: %s",
                  SDL_GetError());
            return true;
        }
        NOT_IMPLEMENTED(false, "Support for UNICODE/UTF-8/Character is not implemented yet");
    }
#else
    return false;
#endif

    return false;
}

bool Clipboard::GetText(Utils::UnicodeStringBuilder& text)
{
    const auto& frontend = Application::GetApplication()->GetFrontendType();
#ifdef _WIN32
    if (frontend == AppCUI::Application::FrontendType::Default ||
        frontend == AppCUI::Application::FrontendType::Terminal ||
        frontend == AppCUI::Application::FrontendType::WindowsConsole)
    {
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
    }
    else if (frontend == AppCUI::Application::FrontendType::SDL)
    {
        if (SDL_HasClipboardText())
        {
            char* cliptext = SDL_GetClipboardText();
            CHECK(cliptext != nullptr, false, "Failed to extract text from the clipboard: %s", SDL_GetError());
            text.Set(cliptext);
            SDL_free(cliptext);
        }
        return true;
    }
#else
    return false;
#endif

    return false;
}
} // namespace AppCUI::OS