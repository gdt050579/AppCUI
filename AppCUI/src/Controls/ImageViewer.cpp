#include <AppCUI.hpp>

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

Color _color_map_[] = {
    /* 0*/ Color::Black,     // (0, 0, 0)
    /* 1*/ Color::DarkBlue,  // (0, 0, 1)
    /* 2*/ Color::Blue,      // (0, 0, 2)
    /* 3*/ Color::DarkGreen, // (0, 1, 0)
    /* 4*/ Color::Teal,      // (0, 1, 1)
    /* 5*/ Color::Teal,      // (0, 1, 2) [Aprox]
    /* 6*/ Color::Green,     // (0, 2, 0)
    /* 7*/ Color::Teal,      // (0, 2, 1) [Aprox]
    /* 8*/ Color::Aqua,      // (0, 2, 2)
    /* 9*/ Color::DarkRed,   // (1, 0, 0)
    /*10*/ Color::Magenta,   // (1, 0, 1)
    /*11*/ Color::Magenta,   // (1, 0, 2) [Aprox]
    /*12*/ Color::Olive,     // (1, 1, 0)
    /*13*/ Color::Gray,      // (1, 1, 1)
    /*14*/ Color::Gray,      // (1, 1, 2) [Aprox]
    /*15*/ Color::Olive,     // (1, 2, 0) [Aprox]
    /*16*/ Color::Gray,      // (1, 2, 1) [Aprox]
    /*17*/ Color::Silver,    // (1, 2, 2) [Aprox]
    /*18*/ Color::Red,       // (2, 0, 0)
    /*19*/ Color::Magenta,   // (2, 0, 1) [Aprox]
    /*20*/ Color::Pink,      // (2, 0, 2)
    /*21*/ Color::Olive,     // (2, 1, 0) [Aprox]
    /*22*/ Color::Gray,      // (2, 1, 1) [Aprox]
    /*23*/ Color::Silver,    // (2, 1, 2) [Aprox]
    /*24*/ Color::Yellow,    // (2, 2, 0)
    /*25*/ Color::Silver,    // (2, 2, 1) [Aprox]
    /*26*/ Color::White,     // (2, 2, 2)
};
inline unsigned int Channel_To_Index(unsigned int rgbChannelValue)
{
    if (rgbChannelValue <= 64)
        return 0;
    else if (rgbChannelValue < 192)
        return 1;
    return 2;
}
Color RGB_to_Color(unsigned int colorRGB)
{
    unsigned int b = Channel_To_Index(colorRGB & 0xFF);  // blue channel
    unsigned int g = Channel_To_Index((colorRGB >> 8) & 0xFF); // green channel
    unsigned int r = Channel_To_Index((colorRGB >> 16) & 0xFF); // red channel
    return _color_map_[r * 9 + g * 3 + b];
}
bool Paint_SmallBoxes(Canvas& c, const AppCUI::Graphics::Image& img)
{
    CHECK(c.Resize(img.GetWidth(), ((img.GetHeight() | 1 + 1) / 2)), false, "Fail to resize !");
    unsigned int py = 0;
    for (unsigned int y = 0; y < img.GetHeight();y+=2,py++)
    {
        for (unsigned int x = 0; x < img.GetWidth(); x++)
        {
            ColorPair cp = {
                RGB_to_Color(img.GetPixel(x, y)),
                RGB_to_Color(img.GetPixel(x, y + 1)),
            };
            c.WriteSpecialCharacter(x, py, SpecialChars::BlockUpperHalf, cp);
        }
    }
    return true;
}
// ===================================================================================================
bool ImageViewer::Create(Control* parent, const std::string_view& layout, ViewerFlags flags)
{
    return CanvasViewer::Create(parent, layout, 16, 16, flags);
}
bool ImageViewer::Create(
      Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout, ViewerFlags flags)
{
    return CanvasViewer::Create(parent, caption, layout, 16, 16, flags);
}
bool ImageViewer::SetImage(const AppCUI::Graphics::Image& img, ImageRendererMode mode)
{
    auto* c = GetCanvas();
    CHECK(c, false, "");
    switch (mode)
    {
    case AppCUI::Controls::ImageRendererMode::SmallBoxes:
        return Paint_SmallBoxes(*c, img);
    case AppCUI::Controls::ImageRendererMode::LargeBoxes:
        NOT_IMPLEMENTED(false);
    case AppCUI::Controls::ImageRendererMode::AsciiArt:
        NOT_IMPLEMENTED(false);
    default:
        NOT_IMPLEMENTED(false);
    }
}