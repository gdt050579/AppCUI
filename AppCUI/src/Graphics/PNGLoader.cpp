#include "ImageLoader.hpp"

#include <lodepng.h>

namespace AppCUI::Graphics
{
bool LoadPNGToImage(Image& img, const uint8* imageBuffer, uint32 size)
{
    uint32 resultedWidth  = 0;
    uint32 resultedHeight = 0;
    uint8* temp           = nullptr;

    if (lodepng_decode_memory(
              &temp, &resultedWidth, &resultedHeight, imageBuffer, size, LodePNGColorType::LCT_RGBA, 8) == 0)
    {
        if (temp != nullptr)
        {
            CHECK(img.Create(resultedWidth, resultedHeight),
                  false,
                  "Fail to create a %ux%u size image",
                  resultedWidth,
                  resultedHeight);

            // data is allocated with malloc --> so for the moment we need to copy it into a buffer allocated with
            auto* p = img.GetPixelsBuffer();
            auto e  = temp + ((size_t) resultedWidth * (size_t) resultedHeight) * sizeof(Pixel);
            auto* c = temp;
            while (c < e)
            {
                p->Red   = *c++;
                p->Green = *c++;
                p->Blue  = *c++;
                p->Alpha = *c++;
                p++;
            }
            free(temp);
            return true;
        }
        RETURNERROR(false, "No bytes were allocated when decoding PNG !");
    }
    else
    {
        if (temp)
            free(temp);
        RETURNERROR(false, "Fail to decode PNG buffer !");
    }
}
} // namespace AppCUI::Graphics
