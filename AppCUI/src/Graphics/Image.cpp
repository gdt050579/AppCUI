#include "AppCUI.hpp"
#include "ImageLoader.hpp"
#include "string.h"

using namespace AppCUI::Graphics;

constexpr unsigned int IMAGE_PNG_MAGIC = 0x474E5089;
constexpr uint16_t IMAGE_BMP_MAGIC     = 0x4D42;

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
    Pixel* pixel;                                                                                                      \
    CHECK(this->pixels != nullptr, errorValue, "Image was not instantiated yet (have you called Create methods ?)");   \
    CHECK(x < this->width,                                                                                             \
          errorValue,                                                                                                  \
          "Invalid X (%d) value. It should be less than %d (the imageWidth of the image)",                             \
          x,                                                                                                           \
          this->width);                                                                                                \
    CHECK(y < this->height,                                                                                            \
          errorValue,                                                                                                  \
          "Invalid Y (%d) value. It should be less than %d (the imageHeight of the image)",                            \
          y,                                                                                                           \
          this->height);                                                                                               \
    pixel = &this->pixels[y * this->width + x];

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
    this->width  = 0;
    this->height = 0;
    this->pixels = nullptr;
}
Image::~Image()
{
    if (this->pixels)
        delete[] this->pixels;
    this->width  = 0;
    this->height = 0;
    this->pixels = nullptr;
}
Image::Image(const Image& img)
{
    if (img.pixels)
    {
        this->width  = img.width;
        this->height = img.height;
        this->pixels = new Pixel[(size_t) img.width * (size_t) img.height];
        memcpy(this->pixels, img.pixels, (size_t) img.width * (size_t) img.height * sizeof(Pixel));
    }
    else
    {
        this->width = this->height = 0;
        this->pixels               = nullptr;
    }
}

Image& Image::operator=(const Image& img)
{
    if (this->pixels)
        delete[] this->pixels;
    this->width  = 0;
    this->height = 0;
    this->pixels = nullptr;
    if (img.pixels)
    {
        this->width  = img.width;
        this->height = img.height;
        this->pixels = new Pixel[(size_t) img.width * (size_t) img.height];
        memcpy(this->pixels, img.pixels, (size_t) img.width * (size_t) img.height * sizeof(Pixel));
    }
    return *this;
}

bool Image::Create(unsigned int imageWidth, unsigned int imageHeight)
{
    CHECK(imageWidth > 0, false, "Invalid 'imageWidth' parameter (should be bigger than 0)");
    CHECK(imageHeight > 0, false, "Invalid 'imageHeight' parameter (should be bigger than 0)");
    if (this->pixels)
    {
        delete[] this->pixels;
        this->pixels = nullptr;
        this->width = this->height = 0;
    }
    this->pixels = new Pixel[imageWidth * imageHeight];
    CHECK(this->pixels != nullptr, false, "Unable to alocate space for an %d x %d image", imageWidth, imageHeight);
    auto* s = this->pixels;
    auto* e = s + imageWidth * imageHeight;
    while (s < e)
    {
        s->ColorValue = 0;
        s++;
    }
    this->width  = imageWidth;
    this->height = imageHeight;
    return true;
}
bool Image::Create(unsigned int imageWidth, unsigned int imageHeight, std::string_view image)
{
    CHECK(Create(imageWidth, imageHeight), false, "");
    auto s = image.data();
    auto e = s + std::min<>(image.size(), (size_t) imageWidth * (size_t) imageHeight);
    auto p = this->pixels;
    while (s < e)
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
    CHECK(this->pixels != nullptr, false, "Image was not instantiated yet (have you called Create methods ?)");
    auto* s = this->pixels;
    auto* e = s + (size_t) this->width * (size_t) this->height;
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
    if ((x >= this->width) || (y >= this->height) || (sz == 0))
        return Pixel(0U); // nothing to compute
    unsigned int e_x = x + sz;
    unsigned int e_y = y + sz;
    if (e_x >= this->width)
        e_x = this->width;
    if (e_y >= this->height)
        e_y = this->height;
    auto xSize         = e_x - x;
    auto ySize         = e_y - y;
    auto sPtr          = this->pixels + (size_t) this->width * (size_t) y + (size_t) x;
    unsigned int sum_r = 0;
    unsigned int sum_g = 0;
    unsigned int sum_b = 0;
    if ((xSize == 0) || (ySize == 0))
        return Pixel(0U); // nothing to compute (sanity check)

    while (y < e_y)
    {
        auto* p = sPtr;
        auto* e = p + (size_t) xSize;
        while (p < e)
        {
            sum_r += p->Red;
            sum_g += p->Green;
            sum_b += p->Blue;
            p++;
        }
        sPtr += this->width; // move to next line
        y++;
    }
    const unsigned int totalPixesl = xSize * ySize;
    const auto result_r            = sum_r / totalPixesl;
    const auto result_g            = sum_g / totalPixesl;
    const auto result_b            = sum_b / totalPixesl;
    return Pixel(result_r, result_g, result_b);
}
bool Image::Load(const std::filesystem::path& path)
{
    auto buf = AppCUI::OS::File::ReadContent(path);
    return Create((const unsigned char*) buf.GetData(), (unsigned int) buf.GetLength());
}
bool Image::CreateFromDIB(const unsigned char* imageBuffer, unsigned int size, bool isIcon)
{
    CHECK(size > 4, false, "Invalid size (expecting at least 4 bytes)");
    CHECK(imageBuffer, false, "Expecting a valid (non-null) buffer !");
    return LoadDIBToImage(*this, imageBuffer, size, isIcon);
}
bool Image::Create(const unsigned char* imageBuffer, unsigned int size)
{
    CHECK(size > 4, false, "Invalid size (expecting at least 4 bytes)");
    CHECK(imageBuffer, false, "Expecting a valid (non-null) buffer !");
    unsigned int magic32        = *(const unsigned int*) imageBuffer;
    uint16_t magic16            = *(const uint16_t*) imageBuffer;


    if (magic32 == IMAGE_PNG_MAGIC)
    {
        return LoadPNGToImage(*this, imageBuffer, size);
    }
    if (magic16 == IMAGE_BMP_MAGIC)
    {
        return LoadBMPToImage(*this, imageBuffer, size);
    }
    
    // unknwon type
    RETURNERROR(false, "Unknwon image type --> unable to identify magic ! (0x%08X)", magic32);
}