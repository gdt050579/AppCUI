#include "AppCUI.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
namespace fs = std::filesystem;

int _special_characters_consolas_unicode_[(unsigned int) AppCUI::Graphics::SpecialChars::Count] = {
    0x2554, 0x2557, 0x255D, 0x255A, 0x2550, 0x2551, 0x256C,                         // double line box
    0x250C, 0x2510, 0x2518, 0x2514, 0x2500, 0x2502, 0x253C,                         // single line box
    0x2191, 0x2193, 0x2190, 0x2192, 0x2195, 0x2194,                                 // arrows
    32,     0x2591, 0x2592, 0x2593, 0x2588, 0x2580, 0x2584, 0x258C, 0x2590, 0x25A0, // blocks
    0x25B2, 0x25BC, 0x25C4, 0x25BA,                                                 // Trangles
    0x25CF, 0x25CB, 0x221A,                                                         // symbols
};

struct HexViewUserControl : public UserControl
{
  public:
    void Paint(Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });
        constexpr ColorPair textColor = ColorPair{ Color::White, Color::Transparent };
        const std::uint64_t height    = GetHeight();
        const std::uint64_t width     = GetWidth();

        for (int i = 0; i < 256; i++)
        {
            const int y = i / width;
            const int x = i % width;
            renderer.WriteCharacter(x, y, i, textColor);
        }

        const int nextLine = 256 / width + 2;
        for (int i = 0; i < (int) AppCUI::Graphics::SpecialChars::Count; i++)
        {
            const int y = i / width;
            const int x = i % width;
            renderer.WriteCharacter(
                  x,
                  nextLine + y,
                  _special_characters_consolas_unicode_[i],
                  ColorPair{ Color::White, Color::Transparent });
        }
        const auto russianText = u8"я текст на русском";
        renderer.WriteSingleLineText(0, height - 1, russianText, textColor);
        const auto romanianText = u8"vivat pălincă";
        renderer.WriteSingleLineText(0, height - 2, romanianText, textColor);
        const auto utf16text = u"ϟ i am decorated utf16 ϟ";
        renderer.WriteSingleLineText(0, height - 3, utf16text, textColor);
    }
};

class FontTest : public AppCUI::Controls::Window
{
  private:
    HexViewUserControl hexView;

  public:
    FontTest() : Window("FontTest", "x:0,y:0,w:100%,h:100%",WindowFlags::None)
    {
        
        hexView.Create(this, "x:0,y:0,w:100%,h:100%");
    }
};

int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<FontTest>());
    Application::Run();
    return 0;
}
