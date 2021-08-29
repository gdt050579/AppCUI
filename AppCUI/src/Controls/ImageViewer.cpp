#include <AppCUI.hpp>

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;


Color RGB_to_Color(unsigned int colorRGB)
{

    return Color::Black;
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