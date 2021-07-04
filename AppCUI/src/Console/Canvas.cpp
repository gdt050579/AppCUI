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
    CHECK(width > 0, false, "Width must be greater than 0.");
    CHECK(height > 0, false, "Height must be greater than 0.");
    Character * tmp = new Character[width*height];
    CHECK(tmp, false, "Fail to allocate %d x %d characters", width, height);
    if (this->Characters)
    {
        // copy from Characters to tmp

        // destroy old Characters buffer
        Destroy();
    }
    this->Characters = tmp;
    this->Width = width;
    this->Height = height;
    CHECK(Clear(fillCharacter, color), false, "");

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
    NOT_IMPLEMENTED(false);
}