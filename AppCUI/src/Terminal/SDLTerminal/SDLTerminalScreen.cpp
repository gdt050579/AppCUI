#include "SDL.h"
#include "SDL_ttf.h"
#include "Terminal/SDLTerminal/SDLTerminal.hpp"
#include "cmrc/cmrc.hpp"
#include <algorithm>
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
constexpr SDL_Color COLOR_BLACK        = SDL_Color{ 0, 0, 0, 255 };
constexpr SDL_Color COLOR_DARKBLUE     = SDL_Color{ 0, 0, 128, 255 };
constexpr SDL_Color COLOR_DARKGREEN    = SDL_Color{ 19, 161, 14, 255 };
constexpr SDL_Color COLOR_DARKCYAN     = SDL_Color{ 58, 150, 221, 255 };
constexpr SDL_Color COLOR_DARKRED      = SDL_Color{ 197, 15, 31, 255 };
constexpr SDL_Color COLOR_MAGENTA      = SDL_Color{ 136, 23, 152, 255 };
constexpr SDL_Color COLOR_DARKYELLOW   = SDL_Color{ 193, 156, 0, 255 };
constexpr SDL_Color COLOR_DARKWHITE    = SDL_Color{ 204, 204, 204, 255 };
constexpr SDL_Color COLOR_BRIGHTBLACK  = SDL_Color{ 118, 118, 118, 255 };
constexpr SDL_Color COLOR_BRIGHTBLUE   = SDL_Color{ 59, 120, 255, 255 };
constexpr SDL_Color COLOR_BRIGHTGREEN  = SDL_Color{ 22, 198, 12, 255 };
constexpr SDL_Color COLOR_BRIGHTCYAN   = SDL_Color{ 97, 214, 214, 255 };
constexpr SDL_Color COLOR_BRIGHTRED    = SDL_Color{ 231, 72, 86, 255 };
constexpr SDL_Color COLOR_BRIGHTMAGENT = SDL_Color{ 180, 0, 158, 255 };
constexpr SDL_Color COLOR_BRIGHTYELLOW = SDL_Color{ 249, 241, 165, 255 };
constexpr SDL_Color COLOR_WHITE        = SDL_Color{ 242, 242, 242, 255 };

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
        fontSize = 10;
        break;
    case CharacterSize::Small:
        fontSize = 12;
        break;
    case CharacterSize::Default:
    case CharacterSize::Normal:
        fontSize = 18;
        break;
    case CharacterSize::Large:
        fontSize = 20;
        break;
    case CharacterSize::Huge:
        fontSize = 25;
        break;
    default:
        break;
    }
    TTF_Init();

    // Load font resource
    auto fs           = cmrc::font::get_filesystem();
    auto fontResource = fs.open(FONT_PATH);

    // Drop font to disk
    const fs::path fontFilePath = fs::temp_directory_path() / "AppCUI_Font.ttf";
    std::ofstream fontFile(fontFilePath, std::ios::binary | std::ios::trunc);
    std::copy(fontResource.begin(), fontResource.end(), std::ostream_iterator<uint8_t>(fontFile));

    // Load font file as TTF
    this->font = TTF_OpenFont(fontFilePath.string().c_str(), fontSize);
    CHECK(font, false, "Failed to init font");

    int fontCharWidth  = 0;
    int fontCharHeight = 0;
    // Hopefully we're using a fixed size font so all the char widths are the same
    TTF_SizeText(font, "A", &fontCharWidth, &fontCharHeight);
    this->charWidth  = static_cast<size_t>(fontCharWidth);
    this->charHeight = static_cast<size_t>(fontCharHeight);

    // Check if it has the box drawing characters
    CHECK(TTF_GlyphIsProvided(font, 0x2550), false, "The provided font doesn't support box chars!");

    // int minx, maxx, miny, maxy, advance;
    // TTF_GlyphMetrics(font, 0x2550, &minx, &maxx, &miny, &maxy, &advance);
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
    CHECK(window, false, "Failed to initialize SDL Window: %s", SDL_GetError());
    windowID = SDL_GetWindowID(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    CHECK(renderer, false, "Failed to initialize SDL Renderer: %s", SDL_GetError());
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
    // Log::ToFile("log.txt");

    SDL_RenderClear(renderer);
    AppCUI::Graphics::Character* charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const std::size_t width                  = ScreenCanvas.GetWidth();
    const std::size_t height                 = ScreenCanvas.GetHeight();

    for (std::size_t y = 0; y < height; y++)
    {
        for (std::size_t x = 0; x < width; x++)
        {
            AppCUI::Graphics::Character ch = charsBuffer[y * width + x];
            const int cuiFG                = static_cast<int>(ch.Color.Foreground);
            const int cuiBG                = static_cast<int>(ch.Color.Background);
            const SDL_Color& fg            = appcuiColorToSDLColor[cuiFG];
            const SDL_Color& bg            = appcuiColorToSDLColor[cuiBG];
            SDL_Surface* glyphSurface      = TTF_RenderGlyph_Shaded(font, ch.Code, fg, bg);
            SDL_Texture* glyphTexture      = SDL_CreateTextureFromSurface(renderer, glyphSurface);

            int iFontCharWidth  = 0;
            int iFontCharHeight = 0;
            const Uint16 text[] = { ch.Code, 0 };
            TTF_SizeUNICODE(font, text, &iFontCharWidth, &iFontCharHeight);
            const std::size_t fontCharWidth  = static_cast<std::size_t>(iFontCharWidth);
            const std::size_t fontCharHeight = static_cast<std::size_t>(iFontCharHeight);

            SDL_Rect WindowRect;
            WindowRect.x = charWidth * x;
            WindowRect.y = charHeight * y;
            WindowRect.w = charWidth;
            WindowRect.h = charHeight;

            /*
            Log::Report(
                  Log::Severity::Information,
                  "log.txt",
                  "",
                  "",
                  0,
                  "char: %x, size: %dx%d",
                  ch.Code,
                  fontCharWidth,
                  fontCharHeight);
            */

            // If we need a character that is 10 x 18, but this one
            // seems to have bigger height (for example 10 x 21), then we
            // don't scale, just crop the lower part of the character, leaving
            // out the upper part
            // as if they're all on the same bottom line
            SDL_Rect CropRect;
            CropRect.x = 0;
            CropRect.y = 0;
            CropRect.w = charWidth;
            CropRect.h = charHeight;
            if (fontCharHeight > charHeight)
            {
                CropRect.y = fontCharHeight - charHeight;
            }

            SDL_RenderCopy(renderer, glyphTexture, &CropRect, &WindowRect);
            SDL_DestroyTexture(glyphTexture);
            SDL_FreeSurface(glyphSurface);
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