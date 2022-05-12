#include "AppCUI.hpp"


#define VALIDATE_ALLOCATED_SPACE(requiredItems, returnValue)                                                           \
    if ((requiredItems) > (Allocated & 0x7FFFFFFF))                                                                    \
    {                                                                                                                  \
        CHECK(Grow(requiredItems), returnValue, "Fail to allocate space for %d items", (requiredItems));               \
    }

namespace AppCUI::Utils
{
constexpr uint32 ARRAY32_FLAG_STACK_BUFFER = 0x80000000U;

template <typename T>
void __HeapSortContext(
      T* Data, int32 (*cmpFunc)(T elem1, T elem2, void* Context), int32 nrElements, bool ascendent, void* Context)
{
    T tempElement;
    uint32 n = nrElements, parent = nrElements / 2, index, child;

    if ((nrElements <= 0) || (Data == nullptr))
        return; // nothing to sort
    if (ascendent)
    {
        for (;;)
        {
            if (parent > 0)
            {
                tempElement = Data[(--parent)];
            }
            else
            {
                n--;
                if (n == 0)
                    return;
                tempElement = Data[n];
                Data[n]     = Data[0];
            }
            index = parent;
            child = index * 2 + 1;
            while (child < n)
            {
                if ((child + 1 < n) && (cmpFunc(Data[(child + 1)], Data[child], Context) > 0))
                    child++;

                if (cmpFunc(Data[child], tempElement, Context) > 0)
                {
                    Data[index] = Data[child];
                    index       = child;
                    child       = index * 2 + 1;
                }
                else
                {
                    break;
                }
            }
            Data[index] = tempElement;
        }
    }
    else
    {
        for (;;)
        {
            if (parent > 0)
            {
                tempElement = Data[(--parent)];
            }
            else
            {
                n--;
                if (n == 0)
                    return;
                tempElement = Data[n];
                Data[n]     = Data[0];
            }
            index = parent;
            child = index * 2 + 1;
            while (child < n)
            {
                if ((child + 1 < n) && (cmpFunc(Data[(child + 1)], Data[child], Context) < 0))
                    child++;

                if (cmpFunc(Data[child], tempElement, Context) < 0)
                {
                    Data[index] = Data[child];
                    index       = child;
                    child       = index * 2 + 1;
                }
                else
                {
                    break;
                }
            }
            Data[index] = tempElement;
        }
    }
}

bool Array32::Grow(uint32 newSize)
{
    newSize = ((newSize | 0x7F) + 1) & 0x7FFFFFFF;
    if (newSize <= (Allocated & 0x7FFFFFFF))
        return true;
    uint32* temp = new uint32[newSize];
    CHECK(temp, false, "Failed to allocate: %d elements", newSize);
    if (Data)
    {
        memcpy(temp, Data, ((size_t) Count) << 2); /* multiply by 4 --> size of int32/uint32*/
        if ((Allocated & ARRAY32_FLAG_STACK_BUFFER) == 0)
            delete []Data;
    }
    Data      = temp;
    Allocated = newSize;
    return true;
}

Array32::Array32()
{
    this->Data      = nullptr;
    this->Count     = 0;
    this->Allocated = 0;
}
Array32::~Array32()
{
    Destroy();
}
void Array32::Destroy()
{
    if ((Data) && ((Allocated & ARRAY32_FLAG_STACK_BUFFER) == 0))
        delete []Data;
    this->Data      = nullptr;
    this->Count     = 0;
    this->Allocated = 0;
}

bool Array32::Create(uint32 allocatedCount)
{
    VALIDATE_ALLOCATED_SPACE(allocatedCount, false);
    this->Count = 0;
    return true;
}
bool Array32::Create(uint32* vector, uint32 vectorSize, uint32 elementsCount)
{
    Destroy();
    CHECK(vector, false, "Expecting a valid (non-null) vector");
    CHECK(vectorSize > 0, false, "Expecting a valid (bigger than 0) vector size");
    this->Count     = std::min<>(elementsCount, vectorSize);
    this->Allocated = vectorSize | ARRAY32_FLAG_STACK_BUFFER;
    this->Data      = vector;
    return true;
}
bool Array32::Create(int32* vector, uint32 vectorSize, uint32 elementsCount)
{
    return Create((uint32*) vector, vectorSize, elementsCount);
}
bool Array32::Reserve(uint32 newSize)
{
    VALIDATE_ALLOCATED_SPACE(newSize, false);
    return true;
}
bool Array32::Resize(uint32 newSize)
{
    VALIDATE_ALLOCATED_SPACE(newSize, false);
    this->Count = newSize;
    return true;
}
bool Array32::Push(uint32 value)
{
    VALIDATE_ALLOCATED_SPACE(Count + 1, false);
    Data[Count++] = value;
    return true;
}
bool Array32::Push(int32 value)
{
    VALIDATE_ALLOCATED_SPACE(Count + 1, false);
    *(int32*) (Data + Count) = value;
    Count++;
    return true;
}
bool Array32::Get(uint32 index, uint32& value)
{
    CHECK(index < Count, false, "Invalid index - should be between [0 and %d)", Count);
    value = Data[index];
    return true;
}
bool Array32::Get(uint32 index, int32& value)
{
    CHECK(index < Count, false, "Invalid index - should be between [0 and %d)", Count);
    value = *(int32*) (Data + Count);
    return true;
}
bool Array32::Insert(uint32 index, uint32 value)
{
    CHECK(index <= Count, false, "Invalid index - should be between [0 and %u]", Count);
    VALIDATE_ALLOCATED_SPACE(Count + 1, false);
    if (index==Count)
    {
        Data[Count++] = value;
    }
    else
    {
        memmove(Data + index + 1, Data + index, ((size_t) (Count - index)) << 2);
        Data[index] = value;
        Count++;
    }
    return true;
}
bool Array32::Insert(uint32 index, int32 value)
{
    return Insert(index, *(uint32*) &value);
}
bool Array32::Delete(uint32 start, uint32 size)
{
    CHECK(start < Count, false, "Invalid start position (%u) - should be between [0 and %u)", start, Count);
    if (start + size>=Count)
    {
        this->Count = start; // truncate
    }
    else
    {
        memmove(Data + start, Data + start+size, ((size_t) (Count - (start + size))) << 2);
        this->Count -= size;
    }
    return true;
}

bool Array32::Sort(int32 (*compare)(int32 elem1, int32 elem2, void* Context), SortDirection direction, void* Context)
{
    CHECK(compare, false, "Expecting a valid (non-null) compare function !");
    __HeapSortContext<int32>((int32*) Data, compare, this->Count, direction == SortDirection::Ascendent, Context);
    return true;
}
bool Array32::Sort(int32 (*compare)(uint32 elem1, uint32 elem2, void* Context), SortDirection direction, void* Context)
{
    CHECK(compare, false, "Expecting a valid (non-null) compare function !");
    __HeapSortContext<uint32>(Data, compare, this->Count, direction == SortDirection::Ascendent, Context);
    return true;
}
} // namespace AppCUI::Utils