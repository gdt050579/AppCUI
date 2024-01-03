#include "AppCUI.hpp"

namespace AppCUI::Utils
{
#define ALIGN_SIZE(sz) ((static_cast<size_t>(sz) | static_cast<size_t>(0xFF)) + static_cast<size_t>(1))
Buffer::~Buffer()
{
    if (data)
        delete[] data;
    data      = nullptr;
    length    = 0;
    allocated = 0;
}

Buffer::Buffer(const Buffer& buf)
{
    if ((buf.data) && (buf.length))
    {
        data = new uint8[ALIGN_SIZE(buf.length)];
        memcpy(data, buf.data, buf.length);
        length    = buf.length;
        allocated = ALIGN_SIZE(buf.length);
    }
    else
    {
        data      = nullptr;
        length    = 0;
        allocated = 0;
    }
}

void Buffer::Reserve(size_t newSize)
{
    if (newSize <= this->allocated)
        return;
    newSize    = ALIGN_SIZE(newSize);
    auto* temp = new uint8[ALIGN_SIZE(newSize)];
    if (this->data)
    {
        memcpy(temp, this->data, this->length);
        delete[] data;
    }
    data            = temp;
    this->allocated = newSize;
}

void Buffer::Resize(size_t newSize)
{
    Reserve(newSize);
    if (newSize <= this->allocated)
    {
        this->length = newSize;
        return;
    }
    else
    {
        auto* temp = data + length;
        auto e     = data + newSize;
        while (temp < e)
        {
            *temp = 0;
            temp++;
        }
        this->length = newSize;
        return;
    }
}
size_t Buffer::Add(const void* p, size_t size, uint32 times)
{
    if ((size == 0) || (p == nullptr) || (times == 0))
        return this->length;
    auto result = this->length;
    Reserve(this->length + size * times);
    auto dest = this->data + this->length;
    while (times)
    {
        memcpy(dest, p, size);
        dest += size;
        this->length += size;
        times--;
    }
    return result;
}
} // namespace AppCUI::Utils
