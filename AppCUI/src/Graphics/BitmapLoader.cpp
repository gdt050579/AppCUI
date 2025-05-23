#include "ImageLoader.hpp"

namespace AppCUI::Graphics
{
#pragma pack(push, 1)
constexpr uint16 BITMAP_WINDOWS_MAGIC             = 0x4D42;
constexpr uint32 BITMAP_COMPRESSION_METHID_BI_RGB = 0;
struct BMP_Header
{
    uint16 magic;
    uint32 size;
    uint16 reserved_1;
    uint16 reserved_2;
    uint32 pixelOffset;
};
struct BMP_InfoHeader
{
    uint32 sizeOfHeader;
    int32 width;
    int32 height;
    uint16 colorPlanes;
    uint16 bitsPerPixel;
    uint32 comppresionMethod;
    uint32 imageSize;
    uint32 horizontalResolution;
    uint32 verticalResolution;
    uint32 numberOfColors;
    uint32 numberOfImportantColors;
};
#pragma pack(pop)

struct DIBPaintBuffer
{
    const uint8* px;
    const uint8* end;
    const uint32* colorTable;
    uint32 width;
    uint32 height;
    const BMP_InfoHeader* header;
};

bool Paint_32bits_DIB(Image& img, DIBPaintBuffer& d, bool bottomUp)
{
    uint32 x          = 0;
    uint32 y          = bottomUp ? d.height - 1 : 0;
    uint32 last_y     = bottomUp ? 0 : d.height - 1;
    uint32 rowPadding = 0;
    while (d.px + 4 <= d.end)
    {
        CHECK(img.SetPixel(x, y, Pixel(d.px[2], d.px[1], d.px[0])),
              false,
              "Fail to set pixel on %u,%u coordonates",
              x,
              y);
        d.px += 4;
        x++;
        if (x == d.width)
        {
            if (y == last_y)
                return true;
            d.px += rowPadding;
            x = 0;
            if (bottomUp)
                y--;
            else
                y++;
        }
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool Paint_24bits_DIB(Image& img, DIBPaintBuffer& d, bool bottomUp)
{
    uint32 x          = 0;
    uint32 y          = bottomUp ? d.height - 1 : 0;
    uint32 last_y     = bottomUp ? 0 : d.height - 1;
    uint32 rowPadding = (4 - ((d.width * 3) & 3)) & 3;
    while (d.px + 3 <= d.end)
    {
        CHECK(img.SetPixel(x, y, Pixel(d.px[2], d.px[1], d.px[0])),
              false,
              "Fail to set pixel on %u,%u coordonates",
              x,
              y);
        d.px += 3;
        x++;
        if (x == d.width)
        {
            if (y == last_y)
                return true;
            d.px += rowPadding;
            x = 0;
            if (bottomUp)
                y--;
            else
                y++;
        }
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool Paint_256color_DIB(Image& img, DIBPaintBuffer& d, bool bottomUp)
{
    uint32 x          = 0;
    uint32 y          = bottomUp ? d.height - 1 : 0;
    uint32 last_y     = bottomUp ? 0 : d.height - 1;
    uint32 rowPadding = (4 - (d.width & 3)) & 3;
    while (d.px <= d.end)
    {
        CHECK(img.SetPixel(x, y, Pixel(d.colorTable[*d.px])), false, "Fail to set pixel on %u,%u coordonates", x, y);
        d.px++;
        x++;
        if (x == d.width)
        {
            if (y == last_y)
                return true;
            d.px += rowPadding;
            x = 0;
            if (bottomUp)
                y--;
            else
                y++;
        }
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool Paint_16color_DIB(Image& img, DIBPaintBuffer& d, bool bottomUp)
{
    uint32 x          = 0;
    uint32 y          = bottomUp ? d.height - 1 : 0;
    uint32 last_y     = bottomUp ? 0 : d.height - 1;
    uint32 pxWidth    = (d.width & 1) + (d.width >> 1); // for 3 the result is 2, for 8 the result is 4
    uint32 rowPadding = (4 - (pxWidth & 3)) & 3;
    while (d.px <= d.end)
    {
        auto val = *d.px;

        for (uint32 cnt = 0; cnt < 2; cnt++) // two colors per byte
        {
            CHECK(img.SetPixel(x, y, Pixel(d.colorTable[(val & 0xF0) >> 4])),
                  false,
                  "Fail to set pixel on %u,%u coordonates",
                  x,
                  y);
            x++;
            if (x == d.width)
            {
                if (y == last_y)
                    return true;
                d.px += rowPadding;
                x = 0;
                if (bottomUp)
                    y--;
                else
                    y++;
                break;
            }
            val = val << 4;
        }
        d.px++;
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool Paint_monochrome_DIB(Image& img, DIBPaintBuffer& d, bool bottomUp)
{
    uint32 x          = 0;
    uint32 y          = bottomUp ? d.height - 1 : 0;
    uint32 last_y     = bottomUp ? 0 : d.height - 1;
    uint32 pxWidth    = (d.width & 7) == 0 ? (d.width >> 3) : (d.width >> 3) + 1;
    uint32 rowPadding = (4 - (pxWidth & 3)) & 3;
    Color c;

    while (d.px <= d.end)
    {
        auto val = *d.px;

        for (uint32 cnt = 0; cnt < 8; cnt++) // two colors per byte
        {
            if (val & 0x80)
                c = Color::White;
            else
                c = Color::Black;
            CHECK(img.SetPixel(x, y, c), false, "Fail to set pixel on %u,%u coordonates", x, y);
            x++;
            if (x == d.width)
            {
                if (y == last_y)
                    return true;
                d.px += rowPadding;
                x = 0;
                if (bottomUp)
                    y--;
                else
                    y++;
                break;
            }
            val = val << 1;
        }
        d.px++;
    }
    RETURNERROR(false, "Premature end of bitmap buffer !");
}
bool LoadDIBToImage(Image& img, const uint8* buffer, uint32 size, bool isIcon)
{
    CHECK(size > sizeof(BMP_InfoHeader),
          false,
          "At least %u bytes must be allocated to read a BItmap DIB header",
          (uint32) sizeof(BMP_InfoHeader));
    auto* h = reinterpret_cast<const BMP_InfoHeader*>(buffer);
    CHECK(h->sizeOfHeader == 40, false, "Invalid `sizeOfHeade` value for DIB header");
    CHECK(h->comppresionMethod == BITMAP_COMPRESSION_METHID_BI_RGB,
          false,
          "Only BI_RGB compression method is supported");
    CHECK(h->width > 0, false, "Width can not be a negative or 0 value value !");
    auto width  = (uint32) h->width;
    auto height = h->height >= 0 ? (uint32) h->height : (uint32) (-h->height);
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
        return Paint_monochrome_DIB(img, dpb, h->height >= 0);
    case 4:
        // set color table
        dpb.px += 64; // 16 entries x 4 (RGBA)
        dpb.colorTable = reinterpret_cast<const uint32*>(buffer + sizeof(BMP_InfoHeader));
        return Paint_16color_DIB(img, dpb, h->height >= 0);
    case 8:
        // set color table
        dpb.px += 1024; // 256 entries x 4 (RGBA)
        dpb.colorTable = reinterpret_cast<const uint32*>(buffer + sizeof(BMP_InfoHeader));
        return Paint_256color_DIB(img, dpb, h->height >= 0);
    case 24:
        return Paint_24bits_DIB(img, dpb, h->height >= 0);
    case 32:
        return Paint_32bits_DIB(img, dpb, h->height >= 0);
    }
    RETURNERROR(false, "Paint method for %d bits/pixels is not implemeted !", h->bitsPerPixel);
}
bool LoadBMPToImage(Image& img, const uint8* buffer, uint32 size)
{
    CHECK(buffer, false, "Expecting a valid (non-null) buffer");
    CHECK(size > sizeof(BMP_Header),
          false,
          "At least %u bytes must be allocated to read a BItmap header",
          (uint32) sizeof(BMP_Header));
    auto* h = reinterpret_cast<const BMP_Header*>(buffer);
    CHECK(h->magic == BITMAP_WINDOWS_MAGIC, false, "Invalid magic --> expecting 'BM'");
    CHECK(h->size <= size, false, "Size field in bitmap is invalid (%u), should be at least %u", h->size, size);
    // all good
    return LoadDIBToImage(img, buffer + sizeof(BMP_Header), size - (uint32) sizeof(BMP_Header), false);
}
} // namespace AppCUI::Graphics
