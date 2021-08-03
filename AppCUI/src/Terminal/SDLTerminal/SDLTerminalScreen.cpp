#include "SDL.h"
#include "SDL_ttf.h"
#include "Terminal/SDLTerminal/SDLTerminal.hpp"
#include "cmrc/cmrc.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <string>

CMRC_DECLARE(font);

using namespace AppCUI::Internal;
using namespace AppCUI::Input;
namespace fs = std::filesystem;

constexpr size_t NR_COLORS = 16;
// https://devblogs.microsoft.com/commandline/updating-the-windows-console-colors/
// something from the old scheme, something from the new scheme
constexpr SDL_Color COLOR_BLACK        = SDL_Color{ 0, 0, 0 };
constexpr SDL_Color COLOR_DARKBLUE     = SDL_Color{ 0, 0, 128 };
constexpr SDL_Color COLOR_DARKGREEN    = SDL_Color{ 19, 161, 14 };
constexpr SDL_Color COLOR_DARKCYAN     = SDL_Color{ 58, 150, 221 };
constexpr SDL_Color COLOR_DARKRED      = SDL_Color{ 197, 15, 31 };
constexpr SDL_Color COLOR_MAGENTA      = SDL_Color{ 136, 23, 152 };
constexpr SDL_Color COLOR_DARKYELLOW   = SDL_Color{ 193, 156, 0 };
constexpr SDL_Color COLOR_DARKWHITE    = SDL_Color{ 204, 204, 204 };
constexpr SDL_Color COLOR_BRIGHTBLACK  = SDL_Color{ 118, 118, 118 };
constexpr SDL_Color COLOR_BRIGHTBLUE   = SDL_Color{ 59, 120, 255 };
constexpr SDL_Color COLOR_BRIGHTGREEN  = SDL_Color{ 22, 198, 12 };
constexpr SDL_Color COLOR_BRIGHTCYAN   = SDL_Color{ 97, 214, 214 };
constexpr SDL_Color COLOR_BRIGHTRED    = SDL_Color{ 231, 72, 86 };
constexpr SDL_Color COLOR_BRIGHTMAGENT = SDL_Color{ 180, 0, 158 };
constexpr SDL_Color COLOR_BRIGHTYELLOW = SDL_Color{ 249, 241, 165 };
constexpr SDL_Color COLOR_WHITE        = SDL_Color{ 242, 242, 242 };

constexpr static std::array<SDL_Color, NR_COLORS> appcuiColorToSDLColor = {
    /* Black */ COLOR_BLACK,
    /* DarkBlue */ COLOR_DARKBLUE,
    /* DarkGreen */ COLOR_DARKGREEN,
    /* Teal */ COLOR_DARKCYAN,
    /* DarkRed */ COLOR_DARKRED,
    /* Magenta */ COLOR_MAGENTA,
    /* Olive */ COLOR_DARKYELLOW,
    /* Silver */ COLOR_DARKWHITE,

    /* GRAY */ COLOR_BRIGHTBLACK,
    /* Blue */ COLOR_BRIGHTBLUE,
    /* Green */ COLOR_BRIGHTGREEN,
    /* Aqua */ COLOR_BRIGHTCYAN,
    /* Red */ COLOR_BRIGHTRED,
    /* Pink */ COLOR_BRIGHTMAGENT,
    /* Yellow */ COLOR_BRIGHTYELLOW,
    /* White */ COLOR_WHITE,
};

/*
    AppCUI window is a 2D table of characters
    So the width and the height in pixels of the window
    are not the same as the appcui window height and width
*/

// We have an embedded compiled resource that we inserted during build
// Let's extract this resource, drop it into some temporary location and load it as a font
//
// After the font is successfully loaded, let's see how wide and high are the characters,
// that will be our cell width and cell height.
bool SDLTerminal::initFont(const InitializationData& initData)
{
    size_t fontSize = 0;
    switch (initData.CharSize)
    {
    case CharacterSize::Tiny:
        fontSize = 12;
        break;
    case CharacterSize::Small:
        fontSize = 16;
        break;
    case CharacterSize::Default:
    case CharacterSize::Normal:
        fontSize = 18;
        break;
    case CharacterSize::Large:
        fontSize = 20;
        break;
    case CharacterSize::Huge:
        fontSize = 23;
        break;
    default:
        break;
    }
    TTF_Init();

    // Load font resource
    auto fs           = cmrc::font::get_filesystem();
    auto fontResource = fs.open("resources/" + fontName);

    // Drop font to disk
    const std::string fontNameFS = std::string("AppCUI_") + fontName;
    const fs::path fontFilePath  = fs::temp_directory_path() / fontNameFS;
    std::ofstream fontFile(fontFilePath, std::ios::binary | std::ios::trunc);
    std::copy(fontResource.begin(), fontResource.end(), std::ostream_iterator<uint8_t>(fontFile));

    // Load font file as TTF
    this->font = TTF_OpenFont(fontFilePath.string().c_str(), fontSize);
    CHECK(font, false, "Failed to init font");

    int fontCharWidth  = 0;
    int fontCharHeight = 0;
    // Hopefully we're using a fixed size font so all the chars are the same
    TTF_SizeText(font, "A", &fontCharWidth, &fontCharHeight);
    this->charWidth  = static_cast<size_t>(fontCharWidth);
    this->charHeight = static_cast<size_t>(fontCharHeight);
    return true;
}

bool SDLTerminal::initScreen(const InitializationData& initData)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    CHECK(initFont(initData), false, "Unable to init font");

    // Default size is half the screen
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    size_t pixelWidth  = DM.w / 2;
    size_t pixelHeight = DM.h / 2;

    Uint32 windowFlags = 0;
    if (!initData.FixedSize)
    {
        windowFlags |= SDL_WindowFlags::SDL_WINDOW_RESIZABLE;
    }
    switch (initData.TermSize)
    {
    case TerminalSize::FullScreen:
        windowFlags |= SDL_WindowFlags::SDL_WINDOW_FULLSCREEN;
        pixelWidth  = DM.w;
        pixelHeight = DM.h;
        break;
    case TerminalSize::Maximized:
        windowFlags |= SDL_WindowFlags::SDL_WINDOW_MAXIMIZED;
        pixelWidth  = DM.w;
        pixelHeight = DM.h;
        break;
    case TerminalSize::CustomSize:
        pixelWidth  = charWidth * initData.Width;
        pixelHeight = charWidth * initData.Height;
        break;
    default:
        break;
    }

    window = SDL_CreateWindow(
          "AppCUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pixelWidth, pixelHeight, windowFlags);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    const size_t widthInChars  = pixelWidth / charWidth;
    const size_t heightInChars = pixelHeight / charHeight;
    CHECK(ScreenCanvas.Create(widthInChars, heightInChars),
          false,
          "Fail to create an internal canvas of %d x %d size",
          widthInChars,
          heightInChars);
    CHECK(OriginalScreenCanvas.Create(widthInChars, heightInChars),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          widthInChars,
          heightInChars);
    return true;
}

// A very basic flush to screen
// It will draw each character as a separate texture
// Optimizations would be welcome, like drawing an entire string of text with the same colors
void SDLTerminal::OnFlushToScreen()
{
    SDL_RenderClear(renderer);

    AppCUI::Console::Character* charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const size_t width                      = ScreenCanvas.GetWidth();
    const size_t height                     = ScreenCanvas.GetHeight();

    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            AppCUI::Console::Character ch = charsBuffer[y * width + x];
            const Uint16 text[]           = { ch.Code, 0 };

            const int cuiFG             = static_cast<int>(ch.Color.Forenground);
            const int cuiBG             = static_cast<int>(ch.Color.Background);
            const SDL_Color& fg         = appcuiColorToSDLColor[cuiFG];
            const SDL_Color& bg         = appcuiColorToSDLColor[cuiBG];
            SDL_Surface* surfaceMessage = TTF_RenderUNICODE_Shaded(font, text, fg, bg);
            SDL_Texture* Message        = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

            SDL_Rect Message_rect;
            Message_rect.x = charWidth * x;
            Message_rect.y = charHeight * y;
            Message_rect.w = charWidth;
            Message_rect.h = charHeight;
            SDL_RenderCopy(renderer, Message, nullptr, &Message_rect);
            SDL_DestroyTexture(Message);
            SDL_FreeSurface(surfaceMessage);
        }
    }

    SDL_RenderPresent(renderer);
}
void SDLTerminal::RestoreOriginalConsoleSettings()
{
}

bool SDLTerminal::OnUpdateCursor()
{
    // Currently no cursor for sdl
    return true;
}

void SDLTerminal::uninitScreen()
{
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}