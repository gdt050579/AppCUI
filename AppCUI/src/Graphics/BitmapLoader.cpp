#include "ImageLoader.hpp"

namespace AppCUI::Graphics
{
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
    const uint32_t* colorTable;
    uint32_t width;
    uint32_t height;
    const BMP_InfoHeader* header;
};

bool Paint_32bits_DIB(Image& img, DIBPaintBuffer& d)
{
    uint32_t x          = 0;
    uint32_t y          = d.height - 1;
    uint32_t rowPadding = 0;
    while (d.px + 4 <= d.end)
    {
        CHECK(img.SetPixel(x, y, Pixel(d.px[0], d.px[1], d.px[2])),
              false,
              "Fail to set pixel on %u,%u coordonates",
              x,
              y);
        d.px += 4;
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
bool Paint_256color_DIB(Image& img, DIBPaintBuffer& d)
{
    uint32_t x          = 0;
    uint32_t y          = d.height - 1;
    uint32_t rowPadding = (4 - (d.width & 3)) & 3;
    while (d.px <= d.end)
    {
        CHECK(img.SetPixel(x, y, Pixel(d.colorTable[*d.px])), false, "Fail to set pixel on %u,%u coordonates", x, y);
        d.px++;
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
bool Paint_16color_DIB(Image& img, DIBPaintBuffer& d)
{
    uint32_t x          = 0;
    uint32_t y          = d.height - 1;
    uint32_t pxWidth    = (d.width & 1) + (d.width >> 1); // for 3 the result is 2, for 8 the result is 4
    uint32_t rowPadding = (4 - (pxWidth & 3)) & 3;
    while (d.px <= d.end)
    {
        auto val = *d.px;

        for (unsigned int cnt = 0; cnt < 2; cnt++) // two colors per byte
        {
            CHECK(img.SetPixel(x, y, Pixel(d.colorTable[(val & 0xF0) >> 4])),
                  false,
                  "Fail to set pixel on %u,%u coordonates",
                  x,
                  y);
            x++;
            if (x == d.width)
            {
                if (y == 0)
                    return true;
                d.px += rowPadding;
                x = 0;
                y--;
                break;
            }
            val = val << 4;
        }
        d.px++;
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool Paint_monochrome_DIB(Image& img, DIBPaintBuffer& d)
{
    uint32_t x          = 0;
    uint32_t y          = d.height - 1;
    uint32_t pxWidth    = (d.width & 7) == 0 ? (d.width >> 8) : (d.width >> 8) + 1;
    uint32_t rowPadding = (4 - (pxWidth & 3)) & 3;
    Color c;
    while (d.px <= d.end)
    {
        auto val = *d.px;

        for (unsigned int cnt = 0; cnt < 8; cnt++) // two colors per byte
        {
            if (val & 0x80)
                c = Color::White;
            else
                c = Color::Black;
            CHECK(img.SetPixel(x, y, c), false, "Fail to set pixel on %u,%u coordonates", x, y);
            x++;
            if (x == d.width)
            {
                if (y == 0)
                    return true;
                d.px += rowPadding;
                x = 0;
                y--;
                break;
            }
            val = val << 1;
        }
        d.px++;
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool LoadDIBToImage(Image& img, const unsigned char* buffer, unsigned int size, bool isIcon)
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
    auto width = h->width;
    CHECK(width > 0, false, "Invalid width (should be bigger than 0)");
    auto height = h->height;
    if (isIcon)
    {
        CHECK((h->height & 1) == 0, false, "Height must be a multiple of 2 --> value is %u ", h->height);
        height = h->height >> 1;
        CHECK(height == width,
              false,
              "Expecting height and width to be equal but with = %u, height = %u",
              width,
              height);
    }
    CHECK(height > 0, false, "Invalid height (should be bigger than 0)");
    CHECK(img.Create(width, height), false, "Fail to create a %ux%u image", width, height);
    CHECK((h->bitsPerPixel == 1) || (h->bitsPerPixel == 4) || (h->bitsPerPixel == 8) || (h->bitsPerPixel == 16) ||
                (h->bitsPerPixel == 24) || (h->bitsPerPixel == 32),
          false,
          "Only 1,4,8,16,24,32 bits/pixels are supported !");
    DIBPaintBuffer dpb;
    dpb.px         = buffer + sizeof(BMP_InfoHeader);
    dpb.end        = buffer + size;
    dpb.header     = h;
    dpb.width      = width;
    dpb.height     = height;
    dpb.colorTable = nullptr;

    switch (h->bitsPerPixel)
    {
    case 1:
        return Paint_monochrome_DIB(img, dpb);
    case 4:
        // set color table
        dpb.px += 64; // 16 entries x 4 (RGBA)
        dpb.colorTable = reinterpret_cast<const uint32_t*>(buffer + sizeof(BMP_InfoHeader));
        return Paint_16color_DIB(img, dpb);
    case 8:
        // set color table
        dpb.px += 1024; // 256 entries x 4 (RGBA)
        dpb.colorTable = reinterpret_cast<const uint32_t*>(buffer + sizeof(BMP_InfoHeader));
        return Paint_256color_DIB(img, dpb);
    case 24:
        return Paint_24bits_DIB(img, dpb);
    case 32:
        return Paint_32bits_DIB(img, dpb);
    }
    RETURNERROR(false, "Paint method for %d bits/pixels is not implemeted !", h->bitsPerPixel);
}
bool LoadBMPToImage(Image& img, const unsigned char* buffer, unsigned int size)
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
    return LoadDIBToImage(img, buffer + sizeof(BMP_Header), size - (unsigned int) sizeof(BMP_Header), false);
}
} // namespace AppCUI::Graphics
