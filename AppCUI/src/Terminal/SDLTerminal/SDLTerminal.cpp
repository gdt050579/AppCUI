#include "SDLTerminal.hpp"
#include "../TerminalFactory.hpp"

namespace AppCUI::Internal
{
bool SDLTerminal::OnInit(const Application::InitializationData& initData)
{
    CHECK(InitScreen(initData), false, "");
    CHECK(InitInput(initData), false, "");
    return true;
}

void SDLTerminal::OnUnInit()
{
    UnInitInput();
    UnInitScreen();
}

#ifdef _WIN32
bool SDLTerminal::ClipboardClear()
{
    CHECK(SDL_SetClipboardText("") > 0, false, "Failed to clear clipboard: %s", SDL_GetError());
    return true;
}

bool SDLTerminal::ClipboardHasText()
{
    return SDL_HasClipboardText();
}

bool SDLTerminal::ClipboardSetText(const ConstString& text)
{
    Utils::ConstStringObject textObj(text);
    if (textObj.Encoding == Utils::StringEncoding::Ascii)
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

bool SDLTerminal::ClipboardGetText(Utils::UnicodeStringBuilder& text)
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
#endif

// AppCUI::Frontend::SDL -> FrontendType::SDL (and FrontendType::Default when no native frontend is linked)
static const TerminalRegistrar sdlTerminalRegistrar(
      Application::FrontendType::SDL, CreateTerminal<SDLTerminal>, DefaultFrontendPriority::Fallback);
} // namespace AppCUI::Internal
