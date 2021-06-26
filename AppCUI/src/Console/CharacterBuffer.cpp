#include "AppCUI.h"
#include <string.h>

using namespace AppCUI::Console;

#define VALIDATE_ALLOCATED_SPACE(requiredSpace, returnValue) \
    if ((requiredSpace) > (Allocated & 0x7FFFFFFF)) { \
        CHECK(Grow(requiredSpace), returnValue, "Fail to allocate space for %d bytes", (requiredSpace)); \
    }

#define COMPUTE_TEXT_SIZE(text,textSize) if (textSize==0xFFFFFFFF) { textSize = AppCUI::Utils::String::Len(text);}


CharacterBuffer::CharacterBuffer()
{
    Allocated = 0;
    Count = 0;
    Buffer = nullptr;
}
CharacterBuffer::~CharacterBuffer(void)
{
    Destroy();
}
void CharacterBuffer::Destroy()
{
    if (Buffer)
        delete Buffer;
    Buffer = nullptr;
    Count = Allocated = 0;
}
bool CharacterBuffer::Grow(unsigned int newSize)
{
    newSize = ((newSize | 15) + 1) & 0x7FFFFFFF;
    if (newSize <= (Allocated & 0x7FFFFFFF))
        return true;
    Character * temp = new Character[newSize];
    CHECK(temp, false, "Failed to allocate: %d characters", newSize);
    if (Buffer)
    {
        memcpy(temp, Buffer, sizeof(Character)*this->Count);
        delete Buffer;
    }
    Buffer = temp;
    Allocated = newSize;
    return true;
}
bool CharacterBuffer::Add(const char * text, unsigned int color, unsigned int textSize)
{
    CHECK(text, false, "Expecting a valid (non-null) text");
    COMPUTE_TEXT_SIZE(text, textSize);
    VALIDATE_ALLOCATED_SPACE(textSize+this->Count, false);
    Character* ch = this->Buffer + this->Count;
    const unsigned char * p = (const unsigned char *)text;
    this->Count += textSize;
    while (textSize > 0)
    {
        ch->Color = color;
        ch->Code = *p;
        ch++;
        p++;
        textSize--;
    }
    return true;
}
bool CharacterBuffer::Set(const char * text, unsigned int color, unsigned int textSize)
{
    this->Count = 0;
    return Add(text, color, textSize);
}
bool CharacterBuffer::SetWithHotKey(const char * text, unsigned int & hotKeyCharacterPosition, unsigned int color, unsigned int textSize)
{
    hotKeyCharacterPosition = 0xFFFFFFFF;
    CHECK(text, false, "Expecting a valid (non-null) text");
    COMPUTE_TEXT_SIZE(text, textSize);
    VALIDATE_ALLOCATED_SPACE(textSize, false);
    Character* ch = this->Buffer;
    const unsigned char * p = (const unsigned char *)text;
    while (textSize > 0)
    {
        if ((hotKeyCharacterPosition == 0xFFFFFFFF) && ((*p) == '&') && (textSize>1 /* not the last character*/))
        {
            char tmp = p[1] | 0x20;
            if (((tmp >= 'a') && (tmp <= 'z')) || ((tmp >= '0') && (tmp <= '9')))
            {
                hotKeyCharacterPosition = (unsigned int)(p - (const unsigned char *)text);
                p++; textSize--;
                continue;
            }
        }
        ch->Color = color;
        ch->Code = *p;
        ch++;
        p++;
        textSize--;
    }
    this->Count = (unsigned int)(ch - this->Buffer);
    return true;
}
void CharacterBuffer::Clear()
{
    this->Count = 0;
}