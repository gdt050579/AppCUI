#include "ImageLoader.hpp"
#include <turbojpeg.h>

namespace AppCUI::Graphics
{
bool LoadJPGToImage(Image& img, const uint8* imageBuffer, uint32 size)
{
    tjhandle tjInstance = tjInitDecompress();
    if (!tjInstance)
    {
        RETURNERROR(false, "Failed to initialize TurboJPEG decompressor");
    }

    int width, height, jpegSubsamp;
    if (tjDecompressHeader2(
              tjInstance,
              const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(imageBuffer)),
              size,
              &width,
              &height,
              &jpegSubsamp) != 0)
    {
        tjDestroy(tjInstance);
        RETURNERROR(false, "Failed to read JPEG header: %s", tjGetErrorStr());
    }

    CHECK(img.Create(width, height), false, "Fail to create a %ux%u size image", width, height);

    if (tjDecompress2(
              tjInstance,
              const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(imageBuffer)),
              size,
              reinterpret_cast<unsigned char*>(img.GetPixelsBuffer()),
              width,
              0 /* pitch */,
              height,
              TJPF_RGB,
              TJFLAG_FASTDCT) != 0)
    {
        tjDestroy(tjInstance);
        RETURNERROR(false, "Failed to decompress JPEG image: %s", tjGetErrorStr());
    }

    tjDestroy(tjInstance);
    return true;
}
} // namespace AppCUI::Graphics
