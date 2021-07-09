#include <AppCUI.h>
#include <string.h>

using namespace AppCUI::Utils;

#define ARRAY32_FLAG_STACK_BUFFER 0x80000000
#define VALIDATE_ALLOCATED_SPACE(requiredItems, returnValue) \
    if ((requiredItems) > (Allocated & 0x7FFFFFFF)) { \
        CHECK(Grow(requiredItems), returnValue, "Fail to allocate space for %d items", (requiredItems)); \
    }


bool Array32::Grow(unsigned int newSize)
{
    newSize = ((newSize | 0x7F) + 1) & 0x7FFFFFFF;
    if (newSize <= (Allocated & 0x7FFFFFFF))
        return true;
    unsigned int * temp = new unsigned int[newSize];
    CHECK(temp, false, "Failed to allocate: %d elements", newSize);
    if (Data)
    {
        memcpy(temp, Data, ((size_t)Count) << 2); /* multiply by 4 --> size of int/uint32*/
        if ((Allocated & ARRAY32_FLAG_STACK_BUFFER) == 0)
            delete Data;
    }
    Data = temp;
    Allocated = newSize;
    return true;
}

Array32::Array32()
{
    this->Data = nullptr;
    this->Count = 0;
    this->Allocated = 0;
}
Array32::~Array32()
{
    Destroy();
}
void Array32::Destroy()
{
    if ((Data) && ((Allocated & ARRAY32_FLAG_STACK_BUFFER) == 0))
        delete Data;
    this->Data = nullptr;
    this->Count = 0;
    this->Allocated = 0;
}

bool Array32::Create(unsigned int allocatedCount)
{
    VALIDATE_ALLOCATED_SPACE(allocatedCount, false);
    this->Count = 0;
    return true;
}
bool Array32::Create(unsigned int * vector, unsigned int vectorSize, unsigned int elementsCount)
{
    Destroy();
    CHECK(vector, false, "Expecting a valid (non-null) vector");
    CHECK(vectorSize > 0, false, "Expecting a valid (bigger than 0) vector size");
    this->Count = MINVALUE(elementsCount, vectorSize);
    this->Allocated = vectorSize | ARRAY32_FLAG_STACK_BUFFER;
    this->Data = vector;
    return true;    
}
bool Array32::Create(int * vector, unsigned int vectorSize, unsigned int elementsCount)
{
    return Create((unsigned int*)vector, vectorSize, elementsCount);
}
bool Array32::Reserve(unsigned int newSize)
{
    VALIDATE_ALLOCATED_SPACE(newSize, false);
    return true;
}
bool Array32::Resize(unsigned int newSize)
{
    VALIDATE_ALLOCATED_SPACE(newSize, false);
    this->Count = newSize;
    return true;
}
bool Array32::Push(unsigned int value)
{
    VALIDATE_ALLOCATED_SPACE(Count + 1, false);
    Data[Count++] = value;
    return true;
}
bool Array32::Push(int value)
{
    VALIDATE_ALLOCATED_SPACE(Count + 1, false);
    *(int*)(Data + Count) = value;
    Count++;
    return true;
}