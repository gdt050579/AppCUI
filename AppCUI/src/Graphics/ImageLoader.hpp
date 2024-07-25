#pragma once

#include "AppCUI.hpp"

namespace AppCUI
{
namespace Graphics
{
    bool LoadDIBToImage(Image& img, const uint8* buffer, uint32 size, bool isIcon);
    bool LoadBMPToImage(Image& img, const uint8* buffer, uint32 size);
    bool LoadPNGToImage(Image& img, const uint8* buffer, uint32 size);
    bool LoadJPGToImage(Image& img, const uint8* buffer, uint32 size);
} // namespace Graphics
} // namespace AppCUI
