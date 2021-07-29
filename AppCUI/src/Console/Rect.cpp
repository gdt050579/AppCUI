#include "AppCUI.hpp"

using namespace AppCUI::Console;

bool Rect::Create(int x, int y, int width, int height, Alignament align)
{
    CHECK((width >= 0) && (height >= 0), false, "Invalid width and height (negative values)");
    switch (align)
    {
    case Alignament::TopLeft:
        this->X      = x;
        this->Y      = y;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::Top:
        this->X      = x - width / 2;
        this->Y      = y;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::TopRight:
        this->X      = x + 1 - width;
        this->Y      = y;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::Right:
        this->X      = x + 1 - width;
        this->Y      = y - height / 2;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::BottomRight:
        this->X      = x + 1 - width;
        this->Y      = y + 1 - height;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::Bottom:
        this->X      = x - width / 2;
        this->Y      = y + 1 - height;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::BottomLeft:
        this->X      = x;
        this->Y      = y + 1 - height;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::Left:
        this->X      = x;
        this->Y      = y - height / 2;
        this->Width  = width;
        this->Height = height;
        return true;
    case Alignament::Center:
        this->X      = x - width / 2;
        this->Y      = y - height / 2;
        this->Width  = width;
        this->Height = height;
        return true;
    }
    RETURNERROR(false, "Unknwon alignament type: %d", align);
}
void Rect::Create(int left, int top, int right, int bottom)
{
    if (left < right)
    {
        this->X     = left;
        this->Width = (right - left) + 1;
    }
    else
    {
        this->X     = right;
        this->Width = (left - right) + 1;
    }
    if (top < bottom)
    {
        this->Y      = top;
        this->Height = (bottom - top) + 1;
    }
    else
    {
        this->Y      = bottom;
        this->Height = (top - bottom) + 1;
    }
}
