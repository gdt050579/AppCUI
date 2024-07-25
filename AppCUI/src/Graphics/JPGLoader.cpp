#include "ImageLoader.hpp"
#include <turbojpeg.h>

namespace AppCUI::Graphics
{
bool LoadJPGToImage(Image& img, const uint8* imageBuffer, uint32 size)
{
    std::unique_ptr<void, void (*)(void*)> handle(
          reinterpret_cast<void*>(tjInitDecompress()), [](void* p) { tjDestroy(reinterpret_cast<tjhandle>(p)); });
    CHECK(handle != nullptr, false, "Failed to initialize TurboJPEG decompressor!");

    int width, height, jpegSubsamp;
    CHECK(tjDecompressHeader2(
                handle.get(),
                const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(imageBuffer)),
                size,
                &width,
                &height,
                &jpegSubsamp) == 0,
          false,
          "Failed to read JPEG header: %s",
          tjGetErrorStr());

    CHECK(img.Create(width, height), false, "Fail to create a %ux%u size image", width, height);

    CHECK(tjDecompress2(
                handle.get(),
                const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(imageBuffer)),
                size,
                reinterpret_cast<unsigned char*>(img.GetPixelsBuffer()),
                width,
                0 /* pitch */,
                height,
                TJPF_RGB,
                TJFLAG_FASTDCT) == 0,
          false,
          "Failed to decompress JPEG image: %s",
          tjGetErrorStr());

    return true;
}
} // namespace AppCUI::Graphics
