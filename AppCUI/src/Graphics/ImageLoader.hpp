#pragma once

#include "AppCUI.hpp"

namespace AppCUI
{
namespace Graphics
{
    bool LoadDIBToImage(Image& img, const uint8* buffer, unsigned int size, bool isIcon);
    bool LoadBMPToImage(Image& img, const uint8* buffer, unsigned int size);
    bool LoadPNGToImage(Image& img, const uint8* buffer, unsigned int size);
} // namespace Graphics
} // namespace AppCUI