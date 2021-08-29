#include <AppCUI.hpp>

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;


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
    unsigned int r = Channel_To_Index(colorRGB & 0xFF);
    unsigned int g = Channel_To_Index((colorRGB >> 8) & 0xFF);
    unsigned int b = Channel_To_Index((colorRGB >> 16) & 0xFF);
    // normalize
    if ((r+g+b)==5) // one of the channels is "1", the rest are "2"
    {
        r = g = b = 2;
    }
    if ((r+g+b)==4)
    {
        if ((r == 1) || (g == 1) || (b==1)) // one of the channels is "2", the rest are "1"
        {
            r = g = b = 1;
        }
    }
    // after this step, r,g,b can be a combination of "0" with either "1" or "2"
    if ((r+g+b)<=3)
    {
        // darker colors
        return static_cast<Color>(b | (g << 1) | (r << 2));
    }
    else
    {
        // lighter colors
        return static_cast<Color>(((b >> 1) | (g) | (r << 1)) | 8);
    }
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