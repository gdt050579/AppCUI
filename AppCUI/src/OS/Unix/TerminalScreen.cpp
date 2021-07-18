#include <string>
#include <filesystem>
#include <fstream>

#include "os.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "cmrc/cmrc.hpp"

CMRC_DECLARE(font);

using namespace AppCUI::Internal;
using namespace AppCUI::Input;
namespace fs = std::filesystem;

constexpr size_t NR_COLORS = 16;
constexpr SDL_Color COLOR_BLACK = SDL_Color{0, 0, 0};
constexpr SDL_Color COLOR_DARKBLUE = SDL_Color{0, 0, 128};
constexpr SDL_Color COLOR_DARKGREEN = SDL_Color{0, 128, 0};
constexpr SDL_Color COLOR_TEAL = SDL_Color{0, 128, 128};
constexpr SDL_Color COLOR_DARKRED = SDL_Color{128, 0, 0};
constexpr SDL_Color COLOR_MAGENTA = SDL_Color{128, 0, 128};
constexpr SDL_Color COLOR_OLIVE = SDL_Color{128, 128, 0};
constexpr SDL_Color COLOR_SILVER = SDL_Color{128, 128, 128};
constexpr SDL_Color COLOR_GRAY = SDL_Color{128, 128, 128};
constexpr SDL_Color COLOR_BLUE = SDL_Color{0, 0, 255};
constexpr SDL_Color COLOR_GREEN = SDL_Color{0, 255, 0};
constexpr SDL_Color COLOR_AQUA = SDL_Color{0, 255, 255};
constexpr SDL_Color COLOR_RED = SDL_Color{255, 0, 0};
constexpr SDL_Color COLOR_PINK = SDL_Color{255, 0, 255};
constexpr SDL_Color COLOR_YELLOW = SDL_Color{255, 255, 0};
constexpr SDL_Color COLOR_WHITE = SDL_Color{255, 255, 255};

constexpr static std::array<SDL_Color, NR_COLORS> appcuiColorToSDLColor = {
    /* Black */     COLOR_BLACK,
    /* DarkBlue */  COLOR_DARKBLUE,
    /* DarkGreen */ COLOR_DARKGREEN,
    /* Teal */      COLOR_TEAL,
    /* DarkRed */   COLOR_DARKRED,
    /* Magenta */   COLOR_MAGENTA,
    /* Olive */     COLOR_OLIVE,
    /* Silver */    COLOR_SILVER,

    /* GRAY */      COLOR_GRAY,
    /* Blue */      COLOR_BLUE,
    /* Green */     COLOR_GREEN,
    /* Aqua */      COLOR_AQUA,
    /* Red */       COLOR_RED,
    /* Pink */      COLOR_PINK,
    /* Yellow */    COLOR_YELLOW,
    /* White */     COLOR_WHITE,
};

bool Terminal::initScreen()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    TTF_Init();
    auto fs = cmrc::font::get_filesystem();
    const std::string fontName = "CourierNew.ttf";
    auto fontResource = fs.open("resources/" + fontName);

    const std::string fontNameFS = std::string("AppCUI_") + fontName;
    fs::path tempFolderPath = fs::temp_directory_path();
    fs::path fontFilePath = tempFolderPath / fontNameFS;
    std::ofstream fontFile(fontFilePath, std::ios::binary);
    std::copy(fontResource.begin(), fontResource.end(), std::ostream_iterator<uint8_t>(fontFile));
    font = TTF_OpenFont(fontFilePath.c_str(), 16);

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    const size_t width = DM.w / 2;
    const size_t height = DM.h / 2;

    window = SDL_CreateWindow(
        "AppCUI",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    size_t consoleWidth = width / charWidth;
    size_t consoleHeight = height / charHeight;
    CHECK(ScreenCanvas.Create(consoleWidth, consoleHeight), false, 
        "Fail to create an internal canvas of %d x %d size", consoleWidth, consoleHeight);
    CHECK(OriginalScreenCanvas.Create(consoleWidth, consoleHeight), false, 
        "Fail to create the original screen canvas of %d x %d size", consoleWidth, consoleHeight);
    return true;
}

// A very basic flush to screen
// It will draw each character as a separate texture
// Optimizations would be welcome, like drawing an entire string of text with the same colors
void Terminal::OnFlushToScreen()
{
    SDL_RenderClear(renderer);

    AppCUI::Console::Character *charsBuffer = this->ScreenCanvas.GetCharactersBuffer();
    const size_t width = ScreenCanvas.GetWidth();
    const size_t height = ScreenCanvas.GetHeight();

    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            AppCUI::Console::Character ch = charsBuffer[y * width + x];
            const Uint16 text[] = {ch.Code, 0};

            const int cuiFG = static_cast<int>(ch.Color.Forenground);
            const int cuiBG = static_cast<int>(ch.Color.Background);
            const SDL_Color& fg = appcuiColorToSDLColor[cuiFG];
            const SDL_Color& bg = appcuiColorToSDLColor[cuiBG];
            SDL_Surface *surfaceMessage = TTF_RenderUNICODE_Shaded(font, text, fg, bg);
            SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

            SDL_Rect Message_rect;
            Message_rect.x = charWidth * x;
            Message_rect.y = charHeight * y;
            Message_rect.w = charWidth;
            Message_rect.h = charHeight;
            SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
            SDL_FreeSurface(surfaceMessage);
            SDL_DestroyTexture(Message);
        }
    }

    SDL_RenderPresent(renderer);
}

bool Terminal::OnUpdateCursor()
{
    // Currently no cursor for sdl
    return true;
}

void Terminal::uninitScreen()
{
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}