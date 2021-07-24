#include "Internal.hpp"

using namespace AppCUI::OS;

bool Clipboard::Clear()
{
    CHECK(OpenClipboard(nullptr), false, "Fail to open the clipboard object !");
    EmptyClipboard();
    CloseClipboard();
    return true;
}
bool Clipboard::SetText(const char *text, unsigned int textSize)
{
	CHECK(text != nullptr, false, "Text should be different than nullptr !");
    if (textSize == 0xFFFFFFFF)
        textSize = Utils::String::Len(text);
    textSize++; // last NULL character

    HANDLE hMem;
    CHECK((hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, textSize)), false, "Fail to allocate %d bytes in data memory to copy a string", textSize);
    char *temp = (char *)GlobalLock(hMem);
    if (temp)
    {
        memcpy(temp, text, textSize - 1);
        temp[textSize - 1] = 0;
    }
    GlobalUnlock(hMem);
    CHECK(temp, false, "Global Lock failed !");

    CHECK(OpenClipboard(nullptr), false, "Fail to open the clipboard object !");
    EmptyClipboard();
    while (true)
    {
        HANDLE h = SetClipboardData(CF_TEXT, temp);
        CHECK(h != nullptr, false, "Fail to copy text data into clipboard (Error: %d)", GetLastError());
        CloseClipboard();
        return true;
    }
    CloseClipboard();
    return false;
}
bool Clipboard::SetText(const AppCUI::Utils::String &text)
{
	return SetText(text.GetText(), text.Len());
}

bool Clipboard::GetText(AppCUI::Utils::String &text)
{	
    CHECK(OpenClipboard(nullptr), false, "Fail to open clipboard object !");    
    while (true)
    {
        const char *temp = (const char *)GetClipboardData(CF_TEXT);
        CHECKBK(temp, "Invalid object received from clipboard data (null)");
        CHECKBK(text.Set(temp), "Fail to copy text into String buffer !");
        CloseClipboard();
        return true;
    }
    CloseClipboard();
    return false;
}