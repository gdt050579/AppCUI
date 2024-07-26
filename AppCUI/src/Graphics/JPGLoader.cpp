#include "ImageLoader.hpp"
#include <turbojpeg.h>

namespace AppCUI::Graphics
{
bool LoadJPGToImage(Image& img, const uint8* imageBuffer, uint32 size)
{
    std::unique_ptr<void, void (*)(void*)> handle(
          reinterpret_cast<void*>(tjInitDecompress()), [](void* p) { tjDestroy(reinterpret_cast<tjhandle>(p)); });
    CHECK(handle != nullptr, false, "Failed to initialize TurboJPEG decompressor!");

    int width, height, jpegSubsamp, colorspace;
    CHECK(tjDecompressHeader3(
                handle.get(),
                const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(imageBuffer)),
                size,
                &width,
                &height,
                &jpegSubsamp,
                &colorspace) == 0,
          false,
          "Failed to read JPEG header: %s",
          tjGetErrorStr());

    TJPF pixelFormat = TJPF_RGB;

    if (colorspace == TJCS_GRAY) {
        pixelFormat = TJPF_RGBX;
    }
    else if (colorspace == TJCS_RGB) {
        pixelFormat = TJPF_RGB;
    }
    else if (colorspace == TJCS_YCbCr || colorspace == TJCS_CMYK || colorspace == TJCS_YCCK) {
        pixelFormat = TJPF_BGRA;
    }

    CHECK(img.Create(width, height), false, "Fail to create a %ux%u size image", width, height);

    uint8* pixelBuffer = reinterpret_cast<uint8*>(img.GetPixelsBuffer());
    CHECK(pixelBuffer != nullptr, false, "Failed to get the image pixel buffer!");

    CHECK(tjDecompress2(
                handle.get(),
                const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(imageBuffer)),
                size,
                pixelBuffer,
                width,
                0,
                height,
                pixelFormat,
                TJFLAG_FASTDCT) == 0,
          false,
          "Failed to decompress JPEG image: %s",
          tjGetErrorStr());

    return true;
}
} // namespace AppCUI::Graphics
