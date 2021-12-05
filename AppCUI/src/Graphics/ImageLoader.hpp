#pragma once

#include "AppCUI.hpp"

namespace AppCUI
{
namespace Graphics
{
    bool LoadDIBToImage(Image& img, const unsigned char* buffer, unsigned int size, bool isIcon);
    bool LoadBMPToImage(Image& img, const unsigned char* buffer, unsigned int size);
    bool LoadPNGToImage(Image& img, const unsigned char* buffer, unsigned int size);
} // namespace Graphics
} // namespace AppCUI