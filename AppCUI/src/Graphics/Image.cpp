#include "AppCUI.hpp"

using namespace AppCUI::Graphics;

static const Pixel Image_ConsoleColors[16] = {
    Pixel(0, 0, 0),       // Black
    Pixel(0, 0, 128),     // DarkBlue
    Pixel(0, 128, 0),     // DarkGreen
    Pixel(0, 128, 128),   // Teal
    Pixel(128, 0, 0),     // DarkRed
    Pixel(128, 0, 128),   // Purple
    Pixel(128, 128, 0),   // Brown
    Pixel(192, 192, 192), // LightGray
    Pixel(128, 128, 128), // DarkGray
    Pixel(0, 0, 255),     // Blue
    Pixel(0, 255, 0),     // Green
    Pixel(0, 255, 255),   // Aqua
    Pixel(255, 0, 0),     // Red
    Pixel(255, 0, 255),   // Pink
    Pixel(255, 255, 0),   // Yellow
    Pixel(255, 255, 255), // White
};

#define CHECK_INDEX(errorValue)                                                                                        \
    Pixel* pixel;                                                                                               \
    CHECK(Pixels != nullptr, errorValue, "Image was not instantiated yet (have you called Create methods ?)");         \
    CHECK(x < Width,                                                                                                   \
          errorValue,                                                                                                  \
          "Invalid X (%d) value. It should be less than %d (the width of the image)",                                  \
          x,                                                                                                           \
          Width);                                                                                                      \
    CHECK(y < Height,                                                                                                  \
          errorValue,                                                                                                  \
          "Invalid Y (%d) value. It should be less than %d (the height of the image)",                                 \
          y,                                                                                                           \
          Height);                                                                                                     \
    pixel = &Pixels[y * Width + x];

#define VALIDATE_CONSOLE_INDEX                                                                                         \
    CHECK(((unsigned int) color) < 16, false, "Invalid console color index (should be between 0 and 15)");

unsigned char Image_CharToIndex[256] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0,    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0,    0xFF, 0,    1,    2,    3,    4,    5,    6,    7,    8,
    9,    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 11,   1,    0xFF, 0xFF, 0xFF, 0xFF, 2,    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 5,    0xFF, 0xFF, 13,   0xFF, 4,    7,    3,    0xFF, 0xFF, 15,   0xFF, 14,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 11,   9,    0xFF, 0xFF, 0xFF, 0xFF, 10,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 5,    0xFF, 0xFF, 13,   0xFF,
    12,   0xFF, 11,   0xFF, 0xFF, 15,   0xFF, 14,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


Image::Image()
{
    Width = Height = 0;
    Pixels         = nullptr;
}
Image::~Image()
{
    if (Pixels)
        delete Pixels;
    Width = Height = 0;
    Pixels         = nullptr;
}
bool Image::Create(unsigned int width, unsigned int height)
{
    CHECK(width > 0, false, "Invalid 'width' parameter (should be bigger than 0)");
    CHECK(height > 0, false, "Invalid 'height' parameter (should be bigger than 0)");
    if (Pixels)
    {
        delete Pixels;
        Pixels = nullptr;
        Width = Height = 0;
    }
    Pixels = new Pixel[width * height];
    CHECK(Pixels != nullptr, false, "Unable to alocate space for an %d x %d image", width, height);
    auto* s = Pixels;
    auto* e = s + width * height;
    while (s < e)
    {
        s->ColorValue = 0;
        s++;
    }
    Width  = width;
    Height = height;
    return true;
}
bool Image::Create(unsigned int width, unsigned int height, std::string_view image)
{
    CHECK(Create(width, height), false, "");
    auto s = image.data();
    auto e = s + std::min<>(image.size(), (size_t) width * (size_t) height);
    auto p = Pixels;
    while (s<e)
    {
        auto val = Image_CharToIndex[*s];
        if (val != 0xFF)
            *p = Image_ConsoleColors[val];
        p++;
        s++;
    }
    return true;
}
bool Image::SetPixel(unsigned int x, unsigned int y, Pixel color)
{
    CHECK_INDEX(false);
    *pixel = color;
    return true;
}

bool Image::SetPixel(unsigned int x, unsigned int y, const Color color)
{
    CHECK_INDEX(false);
    if (((unsigned int) color) < 16)
        *pixel = Image_ConsoleColors[(unsigned int) color];
    return true;
}

bool Image::Clear(Pixel color)
{
    CHECK(Pixels != nullptr, false, "Image was not instantiated yet (have you called Create methods ?)");
    auto* s = Pixels;
    auto* e = s + (size_t)Width * (size_t)Height;
    while (s < e)
    {
        (*s) = color;
        s++;
    }
    return true;
}
bool Image::Clear(const Color color)
{
    VALIDATE_CONSOLE_INDEX;
    return Clear(Image_ConsoleColors[(unsigned int) color]);
}
Pixel Image::GetPixel(unsigned int x, unsigned int y, Pixel invalidIndexValue) const
{
    CHECK_INDEX(invalidIndexValue);
    return *pixel;
}
bool Image::GetPixel(unsigned int x, unsigned int y, Pixel& color) const
{
    CHECK_INDEX(false);
    color = (*pixel);
    return true;
}
Pixel Image::ComputeSquareAverageColor(unsigned int x, unsigned int y, unsigned int sz) const
{
    if ((x >= this->Width) || (y >= this->Height) || (sz==0))
        return Pixel(0U); // nothing to compute
    unsigned int e_x = x + sz;
    unsigned int e_y = y + sz;
    if (e_x >= this->Width)
        e_x = this->Width;
    if (e_y >= this->Height)
        e_y = this->Height;
    auto xSize         = e_x - x;
    auto ySize         = e_y - y;
    auto sPtr          = Pixels + (size_t) Width * (size_t) y + (size_t) x;
    unsigned int sum_r = 0;
    unsigned int sum_g = 0;
    unsigned int sum_b = 0;
    if ((xSize == 0) || (ySize == 0))
        return Pixel(0U); // nothing to compute (sanity check)

    while (y<e_y)
    {
        auto* p = sPtr;
        auto* e = p + (size_t)xSize;
        while (p<e)
        {
            sum_r += p->Red;
            sum_g += p->Green;
            sum_b += p->Blue;
            p++;
        }
        sPtr += Width; // move to next line
        y++;
    }
    const unsigned int totalPixesl = xSize * ySize;
    const auto result_r            = sum_r / totalPixesl;
    const auto result_g            = sum_g / totalPixesl;
    const auto result_b            = sum_b / totalPixesl;
    return Pixel(result_r, result_g, result_b);
}
