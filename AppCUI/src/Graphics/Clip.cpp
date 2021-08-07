#include "AppCUI.hpp"

using namespace AppCUI::Graphics;

Clip::Clip()
{
    Reset();
}
Clip::Clip(const Clip& parent, int x, int y, int width, int height)
{
    Set(parent, x, y, width, height);
}
Clip::Clip(int x, int y, int width, int height)
{
    Set(x, y, width, height);
}
void Clip::Reset()
{
    ScreenPosition.X = ScreenPosition.Y = 0;
    ClipRect.Width = ClipRect.Height = ClipRect.X = ClipRect.Y = 0;
    Visible                                                    = false;
}
void Clip::Set(int x, int y, int width, int height)
{
    this->ScreenPosition.X = x;
    this->ScreenPosition.Y = y;
    if (x >= 0)
        this->ClipRect.X = x;
    else
    {
        this->ClipRect.X = 0;
        width += x;
    }
    if (y >= 0)
        this->ClipRect.Y = y;
    else
    {
        this->ClipRect.Y = 0;
        height += y;
    }
    if ((width > 0) && (height > 0))
    {
        this->ClipRect.Width  = width;
        this->ClipRect.Height = height;
        this->Visible         = true;
    }
    else
    {
        this->ClipRect.Width = this->ClipRect.Height = 0;
        this->Visible                                = false;
    }
}
void Clip::Set(const Clip& parent, int x, int y, int width, int height)
{
    this->ScreenPosition.X = parent.ScreenPosition.X + x;
    this->ScreenPosition.Y = parent.ScreenPosition.Y + y;
    if (parent.Visible == false)
    {
        Visible = false;
        return;
    }
    if (this->ScreenPosition.X >= parent.ClipRect.X)
    {
        this->ClipRect.X = this->ScreenPosition.X;
    }
    else
    {
        this->ClipRect.X = parent.ClipRect.X;
        width -= (parent.ClipRect.X - this->ScreenPosition.X);
    }
    if (this->ScreenPosition.Y >= parent.ClipRect.Y)
    {
        this->ClipRect.Y = this->ScreenPosition.Y;
    }
    else
    {
        this->ClipRect.Y = parent.ClipRect.Y;
        height -= (parent.ClipRect.Y - this->ScreenPosition.Y);
    }

    if ((width > 0) && (height > 0))
    {
        int tmp = parent.ClipRect.X + parent.ClipRect.Width;
        if ((this->ClipRect.X + width) > tmp)
            width = tmp - this->ClipRect.X;

        tmp = parent.ClipRect.Y + parent.ClipRect.Height;
        if ((this->ClipRect.Y + height) > tmp)
            height = tmp - this->ClipRect.Y;
        if ((width > 0) && (height > 0))
        {
            this->ClipRect.Width  = width;
            this->ClipRect.Height = height;
            this->Visible         = true;
            return;
        }
    }
    // invalid clip
    this->ClipRect.Width = this->ClipRect.Height = 0;
    this->Visible                                = false;
}
