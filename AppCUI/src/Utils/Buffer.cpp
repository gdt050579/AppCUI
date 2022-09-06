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
Buffer::Buffer(size_t size)
{
    if (size > 0)
    {
        data      = new uint8[ALIGN_SIZE(size)];
        length    = size;
        allocated = ALIGN_SIZE(size);
    }
    else
    {
        data      = nullptr;
        length    = 0;
        allocated = 0;
    }
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
Buffer& Buffer::operator=(const Buffer& buf)
{
    if (buf.length > this->allocated)
    {
        if (data)
            delete[] data;
        data      = new uint8[ALIGN_SIZE(buf.length)];
        length    = 0;
        allocated = ALIGN_SIZE(buf.length);
    }
    if ((buf.data) && (buf.length))
        memcpy(data, buf.data, buf.length);
    length = buf.length;
    return *this;
}
void Buffer::Resize(size_t newSize)
{
    if (newSize > this->allocated)
    {
        auto* temp = new uint8[ALIGN_SIZE(newSize)];
        if (this->data)
        {
            memcpy(temp, this->data, this->length);
            delete[] data;
        }
        data            = temp;
        this->allocated = ALIGN_SIZE(newSize);
    }
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
size_t Buffer::Add(const void* p, size_t size)
{
    if ((size == 0) || (p == nullptr))
        return this->length;
    Resize(this->length + size);
    memcpy(this->data + this->length, p, size);
    auto result = this->length;
    this->length += size;
    return result;
}
} // namespace AppCUI::Utils
