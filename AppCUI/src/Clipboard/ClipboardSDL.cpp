#include "Internal.h"
#include "SDL.h"
using namespace AppCUI::OS;

bool Clipboard::Clear()
{
	CHECK(SDL_SetClipboardText("") > 0, false, "Failed to clear clipboard: %s", SDL_GetError());
	return true;
}

bool Clipboard::SetText(const char *text, unsigned int textSize)
{
	CHECK(SDL_SetClipboardText(text) > 0, false, "Failed to set clipboard text: %s", SDL_GetError());
	return true;
}

bool Clipboard::SetText(const AppCUI::Utils::String &text)
{
	CHECK(SDL_SetClipboardText(text.GetText()) > 0, false, "Failed to set clipboard text: %s", SDL_GetError());
	return true;
}

bool Clipboard::GetText(AppCUI::Utils::String &text)
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