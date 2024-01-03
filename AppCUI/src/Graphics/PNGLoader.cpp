#include "ImageLoader.hpp"
#include <libpng16/png.h>

namespace AppCUI::Graphics
{
bool LoadPNGToImage(Image& img, const uint8* imageBuffer, uint32 size)
{
    /* Initialize the 'png_image' structure. */
    png_image image{ .version = PNG_IMAGE_VERSION }; /* The control structure used by libpng */

    /* The first argument is the file to read: */
    CHECK(png_image_begin_read_from_memory(&image, imageBuffer, size) != 0, false, "Failed to read PNG from memory!");

    std::unique_ptr<png_byte> buffer(new png_byte[PNG_IMAGE_SIZE(image)]);
    CHECK(png_image_finish_read(&image, NULL, buffer.get(), 0, NULL) != 0, false, "");

    CHECK(img.Create(image.width, image.height), false, "Fail to create a %ux%u size image", image.width, image.height);
    memcpy(img.GetPixelsBuffer(), buffer.get(), ((size_t) image.width * (size_t) image.height) * sizeof(Pixel));

    return true;
}
} // namespace AppCUI::Graphics
