#include "Internal.hpp"

namespace AppCUI::OS
{
using namespace Utils;

bool CopyTextBufferToClipboard(const void* buf, size_t characterSize, size_t length)
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
            ((unsigned char*) temp)[length] = 0;
        else if (characterSize == 2)
            ((unsigned short*) temp)[length] = 0;
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

bool Clipboard::Clear()
{
    CHECK(OpenClipboard(nullptr), false, "Fail to open the clipboard object !");
    EmptyClipboard();
    CloseClipboard();
    return true;
}
bool Clipboard::HasText()
{
    return ((IsClipboardFormatAvailable(CF_TEXT)) || (IsClipboardFormatAvailable(CF_UNICODETEXT)));
}
bool Clipboard::SetText(const ConstString& text)
{
    ConstStringObject textObj(text);
    CHECK(textObj.Data != nullptr, false, "Text should be different than nullptr !");

    Utils::LocalUnicodeStringBuilder<1024> unicode;
    if (textObj.Encoding == StringEncoding::Ascii)
        return CopyTextBufferToClipboard(textObj.Data, sizeof(char), textObj.Length);
    CHECK(unicode.Set(text), false, "Fail to convert ConstString into unicode buffer !");
    return CopyTextBufferToClipboard(unicode.GetString(), sizeof(char16_t), unicode.Len());
}

bool Clipboard::GetText(Utils::UnicodeStringBuilder& text)
{
    CHECK(OpenClipboard(nullptr), false, "Fail to open clipboard object !");
    while (true)
    {
        const char16_t* temp = (const char16_t*) GetClipboardData(CF_UNICODETEXT);
        CHECKBK(temp, "Invalid object received from clipboard data (null)");
        CHECKBK(text.Set(temp), "Fail to copy text into String buffer !");
        CloseClipboard();
        return true;
    }
    CloseClipboard();
    return false;
}
} // namespace AppCUI::OS