#include "ImageLoader.hpp"

using namespace AppCUI::Graphics;

#pragma pack(push, 1)
constexpr uint16_t BITMAP_WINDOWS_MAGIC             = 0x4D42;
constexpr uint32_t BITMAP_COMPRESSION_METHID_BI_RGB = 0;
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

struct DIBPaintBuffer
{
    const unsigned char* px;
    const unsigned char* end;
    uint32_t width;
    uint32_t height;
    const BMP_InfoHeader* header;
};

bool Paint_24bits_DIB(Image& img, DIBPaintBuffer& d)
{
    uint32_t x          = 0;
    uint32_t y          = d.height - 1;
    uint32_t rowPadding = (4 - ((d.width * 3) & 3)) & 3;
    while (d.px + 3 <= d.end)
    {
        CHECK(img.SetPixel(x, y, Pixel(d.px[0], d.px[1], d.px[2])),
              false,
              "Fail to set pixel on %u,%u coordonates",
              x,
              y);
        d.px += 3;
        x++;
        if (x == d.width)
        {
            if (y == 0)
                return true;
            d.px += rowPadding;
            x = 0;
            y--;
        }
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool AppCUI::Graphics::LoadDIBToImage(Image& img, const unsigned char* buffer, unsigned int size)
{
    CHECK(size > sizeof(BMP_InfoHeader),
          false,
          "At least %u bytes must be allocated to read a BItmap DIB header",
          (unsigned int) sizeof(BMP_InfoHeader));
    auto* h = reinterpret_cast<const BMP_InfoHeader*>(buffer);
    CHECK(h->sizeOfHeader == 40, false, "Invalid `sizeOfHeade` value for DIB header");
    CHECK(h->comppresionMethod == BITMAP_COMPRESSION_METHID_BI_RGB,
          false,
          "Only BI_RGB compression method is supported");
    CHECK(h->width > 0, false, "Invalid width (should be bigger than 0)");
    CHECK(h->height > 0, false, "Invalid height (should be bigger than 0)");
    CHECK(img.Create(h->width, h->height), false, "Fail to create a %ux%u image", h->width, h->height);
    CHECK((h->bitsPerPixel == 1) || (h->bitsPerPixel == 4) || (h->bitsPerPixel == 8) || (h->bitsPerPixel == 16) ||
                (h->bitsPerPixel == 24) || (h->bitsPerPixel == 32),
          false,
          "Only 1,4,8,16,24,32 bits/pixels are supported !");
    DIBPaintBuffer dpb;
    dpb.px     = buffer + sizeof(BMP_InfoHeader);
    dpb.end    = buffer + size;
    dpb.header = h;
    dpb.width  = h->width;
    dpb.height = h->height;

    switch (h->bitsPerPixel)
    {
    case 24:
        return Paint_24bits_DIB(img, dpb);
    }
    RETURNERROR(false, "Paint method for %d bits/pixels is not implemeted !", h->bitsPerPixel);
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
    return LoadDIBToImage(img, buffer + sizeof(BMP_Header), size - (unsigned int) sizeof(BMP_Header));
}