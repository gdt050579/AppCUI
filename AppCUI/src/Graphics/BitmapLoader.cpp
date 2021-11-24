#include "BitmapLoader.hpp"

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
#pragma pack(pop)

bool AppCUI::Graphics::LoadDIBToImage(Image& img, const unsigned char* buffer, unsigned int size)
{
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