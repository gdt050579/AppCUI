#include "AppCUI.h"
#include <string.h>

using namespace AppCUI::Console;

#define CHECK_CANVAS_INITIALIZE CHECK(this->Characters,false,"Canvas object is not initialized. Have you called Cretae/Resize methods");

Canvas::Canvas()
{
    Characters = nullptr;
    Width = Height = 0;
}
Canvas::~Canvas()
{
    Destroy();
}
void Canvas::Destroy()
{
    if (Characters)
        delete[]Characters;
    Characters = nullptr;
    Width = Height = 0;
}
bool Canvas::Create(unsigned int width, unsigned int height, int fillCharacter, unsigned int color)
{
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    if ((width == this->Width) && (height = this->Height))
    {
        // no re-allocation required
        CHECK(Clear(fillCharacter, color), false, "");
        return true;
    }
    Character * tmp = new Character[width*height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters",width,height);
    Destroy();
    this->Characters = tmp;
    this->Width = width;
    this->Height = height;
    CHECK(Clear(fillCharacter, color), false, "");

    return true;
}
bool Canvas::Resize(unsigned int width, unsigned int height, int fillCharacter, unsigned int color)
{
    if (this->Characters == nullptr)
        return Create(width, height, fillCharacter, color);
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    if ((width == this->Width) && (height = this->Height))
        return true; // nothing to resize
    Character * tmp = new Character[width*height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters", width, height);

    unsigned short col = color & 0xFF; // no transparency allowed
    unsigned short chr = ' ';

    if ((fillCharacter >= 0) && (fillCharacter <= 0xFFFF))
        chr = (unsigned short)(fillCharacter & 0xFFFF);
    // copy from Characters to tmp
    unsigned int min_w = MINVALUE(this->Width, width);
    unsigned int min_h = MINVALUE(this->Height, height);
    for (unsigned int y = 0; y < min_h; y++)
    {
        Character * p_temp = tmp + (y*width);
        Character * p_current = this->Characters + (y*this->Width);
        for (unsigned int x = 0; x < min_w; x++,p_temp++,p_current++)
        {
            *(p_temp) = *(p_current);
        }
    }
    // fill the rest
    if (min_w < width) {
        for (unsigned int y = 0; y < height; y++)
        {
            Character * p_temp = tmp + (y*width);
            for (unsigned int x = min_w; x < width; x++,p_temp++)
            {
                p_temp->Code = chr;
                p_temp->Color = col;                    
            }
        }
    }
    if (min_h < height)
    {
        for (unsigned int y = min_h; y < height; y++)
        {
            Character * p_temp = tmp + (y*width);
            for (unsigned int x = 0; x < min_w; x++, p_temp++)
            {
                p_temp->Code = chr;
                p_temp->Color = col;
            }
        }
    }
    // destroy old Characters buffer
    Destroy();
    this->Characters = tmp;
    this->Width = width;
    this->Height = height;

    return true;
}
bool Canvas::Clear(unsigned int color)
{
    return Clear(' ', color);
}
bool Canvas::Clear(int character, unsigned int color) 
{
    CHECK_CANVAS_INITIALIZE;
    Character * s = this->Characters;
    Character * e = s + (this->Width* this->Height);
    unsigned short col = color & 0xFF; // no transparency allowed
    unsigned short chr = ' ';

    if ((character >= 0) && (character <= 0xFFFF))
        chr = (unsigned short)(character & 0xFFFF);
    while (s < e) {
        s->Code = chr;
        s->Color = col;
        s++;
    }
    return true;
}
bool Canvas::Set(int x, int y, int character, unsigned int color)
{
    CHECK_CANVAS_INITIALIZE;
    CHECK(((unsigned int)x) < this->Width, false, "Invalid 'X' coordonate (%d) --> shold be smaller than %d", x, this->Width);
    CHECK(((unsigned int)y) < this->Height, false, "Invalid 'Y' coordonate (%d) --> shold be smaller than %d", y, this->Height);
    Character * c = this->Characters + (x + y * this->Width);
    if (character >= 0)
        c->Code = ((unsigned short)character);
    if (color < 256)
    {
        c->Color = color;
    } else {
        if (color != AppCUI::Console::Color::NoColor) 
        {
            unsigned int temp_color = color;
            if (color & 256) temp_color = c->Color & 0x0F | (temp_color & 0xFFFFF0);
            if (color & (256<<4)) temp_color = c->Color & 0xF0 | (temp_color & 0xFFFF0F);
            c->Color = temp_color;
        }
    }
    return true;
}
bool Canvas::FillRect(int left, int top, int right, int bottom, int fillCharacter, unsigned int color)
{
    left = MAXVALUE(left, 0);
    top = MAXVALUE(top, 0);
    if (right>=(int)this->Width)
        right = ((int)(this->Width)) - 1;
    if (bottom >= (int)this->Height)
        bottom = ((int)(this->Height)) - 1;
    while (top <= bottom)
    {
        Character * c = this->Characters + top * this->Width;
        for (int x = left; x <= right; x++, c++)
        {
            if (fillCharacter >= 0)
                c->Code = ((unsigned short)fillCharacter);
            if (color < 256)
            {
                c->Color = color;
            }
            else {
                if (color != AppCUI::Console::Color::NoColor)
                {
                    unsigned int temp_color = color;
                    if (color & 256) temp_color = c->Color & 0x0F | (temp_color & 0xFFFFF0);
                    if (color & (256 << 4)) temp_color = c->Color & 0xF0 | (temp_color & 0xFFFF0F);
                    c->Color = temp_color;
                }
            }
        }
    }
    return true;
}
bool Canvas::FillRectWithSize(int x, int y, unsigned int width, unsigned int height, int fillCharacter, unsigned int color)
{
    CHECK(width > 0, false, "Width must be bigger than 0");
    CHECK(height > 0, false, "Width must be bigger than 0");
    return FillRect(x, y, x + ((int)width) - 1, y + ((int)height) - 1, fillCharacter, color);
}

