#include "Internal.hpp"
#include "SDL.h"
using namespace AppCUI::OS;

bool Clipboard::Clear()
{
    CHECK(SDL_SetClipboardText("") > 0, false, "Failed to clear clipboard: %s", SDL_GetError());
    return true;
}

bool Clipboard::SetText(const AppCUI::Utils::ConstString& text)
{
    AppCUI::Utils::ConstStringObject textObj(text);
    if (textObj.Type == StringViewType::Ascii)
    {
	// GDT: temporary fix - we can't guarantee that text is a NULL terminated string
    	CHECK(SDL_SetClipboardText((const char *)textObj.Data) > 0, false, "Failed to set clipboard text: %s", SDL_GetError());
    	return true;
    }
    NOT_IMPLEMENTED(false,"Support for UNICODE/UTF-8/Character is not implemented yet");
}

bool Clipboard::GetText(AppCUI::Utils::String& text)
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