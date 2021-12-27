#include "AppCUI.hpp"
#include <string.h>

namespace AppCUI::Graphics
{
Canvas::Canvas()
{
}
Canvas::~Canvas()
{
}
bool Canvas::Create(uint32 width, uint32 height, int fillCharacter, ColorPair color)
{
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    if ((width == this->Width) && (height = this->Height))
    {
        // no re-allocation required
        CHECK(Clear(fillCharacter, color), false, "");
        return true;
    }
    Character* tmp = new Character[width * height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters", width, height);
    // all good --> associate the rows to the screen
    Character** ofs_tmp = new Character*[height];
    CHECK(ofs_tmp, false, "Fail to allocate offset row vector of %d elements", height);
    Character* p = tmp;
    for (uint32 tr = 0; tr < height; tr++, p += width)
        ofs_tmp[tr] = p;
    _Destroy();
    this->Characters = tmp;
    this->OffsetRows = ofs_tmp;
    this->Width      = width;
    this->Height     = height;
    this->Reset();

    CHECK(Clear(fillCharacter, color), false, "");

    return true;
}
bool Canvas::Resize(uint32 width, uint32 height, int fillCharacter, ColorPair color)
{
    if (this->Characters == nullptr)
        return Create(width, height, fillCharacter, color);
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    if ((width == this->Width) && (height == this->Height))
        return true; // nothing to resize
    Character* tmp = new Character[width * height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters", width, height);
    Character** ofs_tmp = new Character*[height];
    CHECK(ofs_tmp, false, "Fail to allocate offset row vector of %d elements", height);
    Character* p = tmp;
    for (uint32 tr = 0; tr < height; tr++, p += width)
        ofs_tmp[tr] = p;

    uint16 chr = ' ';

    if ((fillCharacter >= 0) && (fillCharacter <= 0xFFFF))
        chr = (uint16) (fillCharacter & 0xFFFF);
    // copy from Characters to tmp
    uint32 min_w = std::min<>(this->Width, width);
    uint32 min_h = std::min<>(this->Height, height);
    for (uint32 y = 0; y < min_h; y++)
    {
        Character* p_temp    = tmp + (y * width);
        Character* p_current = this->Characters + (y * this->Width);
        for (uint32 x = 0; x < min_w; x++, p_temp++, p_current++)
        {
            *(p_temp) = *(p_current);
        }
    }
    // fill the rest
    if (min_w < width)
    {
        for (uint32 y = 0; y < height; y++)
        {
            Character* p_temp = tmp + (y * width);
            for (uint32 x = min_w; x < width; x++, p_temp++)
            {
                p_temp->Code  = chr;
                p_temp->Color = color;
            }
        }
    }
    if (min_h < height)
    {
        for (uint32 y = min_h; y < height; y++)
        {
            Character* p_temp = tmp + (y * width);
            for (uint32 x = 0; x < min_w; x++, p_temp++)
            {
                p_temp->Code  = chr;
                p_temp->Color = color;
            }
        }
    }
    // destroy old Characters buffer
    _Destroy();
    this->Characters = tmp;
    this->OffsetRows = ofs_tmp;
    this->Width      = width;
    this->Height     = height;
    // LOG_INFO("Resize screen canvas to %dx%d", this->Width, this->Height);
    this->Reset();
    return true;
}
void Canvas::Reset()
{
    this->TranslateX = this->TranslateY = 0;
    this->Clip.Left = this->Clip.Top = 0;
    this->Clip.Right                 = this->Width - 1;
    this->Clip.Bottom                = this->Height - 1;
    this->Clip.Visible               = true;
    this->ClipCopy.Bottom = this->ClipCopy.Top = this->ClipCopy.Left = this->ClipCopy.Right = -1;
    this->ClipCopy.Visible                                                                  = false;
    this->ClipHasBeenCopied                                                                 = false;
    this->HideCursor();
}
void Canvas::SetAbsoluteClip(const Graphics::Clip& clip)
{
    if (clip.Visible)
    {
        // make sure that clipping coordonates are within screen coordonates
        this->Clip.Left   = std::max<>(clip.ClipRect.X, 0);
        this->Clip.Top    = std::max<>(clip.ClipRect.Y, 0);
        this->Clip.Right  = clip.ClipRect.X + clip.ClipRect.Width - 1;
        this->Clip.Bottom = clip.ClipRect.Y + clip.ClipRect.Height - 1;
        if (this->Clip.Right >= (int) this->Width)
            this->Clip.Right = (int) this->Width - 1;
        if (this->Clip.Bottom >= (int) this->Height)
            this->Clip.Bottom = (int) this->Height - 1;
        this->Clip.Visible = (Clip.Left <= Clip.Right) && (Clip.Top <= Clip.Bottom);
    }
    else
    {
        this->Clip.Visible = false;
    }
    this->ClipHasBeenCopied = false;
}
void Graphics::Canvas::ExtendAbsoluteClipInAllDirections(int size)
{
    if (Clip.Visible)
    {
        Clip.Left   = std::max<>(0, Clip.Left - size);
        Clip.Top    = std::max<>(0, Clip.Top - size);
        Clip.Right  = std::min<>(Clip.Right + size, static_cast<int>(this->Width) - size);
        Clip.Bottom = std::min<>(Clip.Bottom + size, static_cast<int>(this->Height) - size);
    }
}
void Canvas::ExtendAbsoluteClipToRightBottomCorner()
{
    if (Clip.Visible)
    {
        if ((Clip.Right + 1) < (int) this->Width)
            Clip.Right++;
        if ((Clip.Bottom + 1) < (int) this->Height)
            Clip.Bottom++;
    }
}
void Canvas::ClearClip()
{
    this->Clip.Left         = 0;
    this->Clip.Top          = 0;
    this->Clip.Right        = this->Width - 1;
    this->Clip.Bottom       = this->Height - 1;
    this->Clip.Visible      = true;
    this->ClipHasBeenCopied = false;
}
void Canvas::SetTranslate(int offX, int offY)
{
    this->TranslateX = offX;
    this->TranslateY = offY;
}
void Canvas::DarkenScreen()
{
    Character* start = this->Characters;
    Character* end   = this->Characters + (this->Width * this->Height);
    while (start < end)
    {
        start->Color = ColorPair{ Color::Gray, Color::Black };
        start++;
    }
}
bool Canvas::ClearEntireSurface(int character, ColorPair color)
{
    return _ClearEntireSurface(character, color);
}
} // namespace AppCUI::Graphics