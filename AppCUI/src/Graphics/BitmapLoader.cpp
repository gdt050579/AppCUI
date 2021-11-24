#include "ImageLoader.hpp"

using namespace AppCUI::Graphics;

#pragma pack(push,1)
constexpr uint16_t BITMAP_WINDOWS_MAGIC = 0x4D42;
struct BMP_Header
{
    uint16_t magic;
    uint32_t size;
    uint16_t reserved_1;
    uint16_t reserved_2;
    uint32_t pixelOffset;
};
struct BMP_InfoHeader
{
    uint32_t sizeOfHeader;
    uint32_t width;
    uint32_t height;
    uint16_t colorPlanes;
    uint16_t bitsPerPixel;
    uint32_t comppresionMethod;
    uint32_t imageSize;
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t numberOfColors;
    uint32_t numberOfImportantColors;
};
#pragma pack(pop)

bool AppCUI::Graphics::LoadDIBToImage(Image& img, const unsigned char* buffer, unsigned int size)
{
    CHECK(size > sizeof(BMP_InfoHeader),
          false,
          "At least %u bytes must be allocated to read a BItmap DIB header",
          (unsigned int) sizeof(BMP_InfoHeader));
    auto* h = reinterpret_cast<const BMP_InfoHeader*>(buffer);
    CHECK(h->sizeOfHeader == 40, false, "Invalid `sizeOfHeade` value for DIB header");
    NOT_IMPLEMENTED(false);
}
bool AppCUI::Graphics::LoadBMPToImage(Image& img, const unsigned char* buffer, unsigned int size)
{
    CHECK(buffer, false, "Expecting a valid (non-null) buffer");
    CHECK(size > sizeof(BMP_Header),
          false,
          "At least %u bytes must be allocated to read a BItmap header",
          (unsigned int) sizeof(BMP_Header));
    auto* h = reinterpret_cast<const BMP_Header*>(buffer);
    CHECK(h->magic == BITMAP_WINDOWS_MAGIC, false, "Invalid magic --> expecting 'BM'");
    CHECK(h->size <= size, false, "Size field in bitmap is invalid (%u), should be at least %u", h->size, size);
    // all good
    return LoadDIBToImage(img, buffer + sizeof(BMP_Header), size - (unsigned int)sizeof(BMP_Header));

}