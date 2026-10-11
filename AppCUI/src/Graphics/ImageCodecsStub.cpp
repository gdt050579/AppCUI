#include "ImageLoader.hpp"

// Built instead of PNGLoader.cpp / JPGLoader.cpp when APPCUI_IMAGE_CODECS is OFF (wasm32-emscripten builds): AppCUI
// keeps its BMP / DIB support and reports PNG / JPG images as unsupported.
namespace AppCUI::Graphics
{
bool LoadPNGToImage(Image&, const uint8*, uint32)
{
    RETURNERROR(false, "PNG images are not supported by this build of AppCUI (APPCUI_IMAGE_CODECS=OFF)");
}
bool LoadJPGToImage(Image&, const uint8*, uint32)
{
    RETURNERROR(false, "JPG images are not supported by this build of AppCUI (APPCUI_IMAGE_CODECS=OFF)");
}
} // namespace AppCUI::Graphics
