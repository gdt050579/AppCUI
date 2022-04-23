#include "AppCUI.hpp"
#include <string.h>

namespace AppCUI::Utils
{
Buffer::~Buffer()
{
    if (data)
        delete[] data;
    data   = nullptr;
    length = 0;
}
Buffer::Buffer(size_t size)
{
    if (size > 0)
    {
        data   = new uint8[size];
        length = size;
    }
    else
    {
        data   = nullptr;
        length = 0;
    }
}
Buffer::Buffer(const Buffer& buf)
{
    if ((buf.data) && (buf.length))
    {
        data = new uint8[buf.length];
        memcpy(data, buf.data, buf.length);
        length = buf.length;
    }
    else
    {
        data   = nullptr;
        length = 0;
    }
}
Buffer& Buffer::operator=(const Buffer& buf)
{
    if (data)
        delete[] data;
    data   = nullptr;
    length = 0;
    if ((buf.data) && (buf.length))
    {
        data = new uint8[buf.length];
        memcpy(data, buf.data, buf.length);
        length = buf.length;
    }
    else
    {
        data   = nullptr;
        length = 0;
    }
    return *this;
}
void Buffer::Resize(size_t newSize)
{
    if (newSize == 0)
    {
        // clean all
        if (data)
            delete[] data;
        data   = nullptr;
        length = 0;
        return;
    }
    if (newSize <= length)
    {
        // just set a different size
        this->length = newSize;
        return;
    }
    // for bigger sizes
    auto tmp = new uint8[newSize];
    if ((data) && (length))
    {
        memcpy(tmp, data, length);
        delete[] data;
        data = tmp;
        tmp += length;
        auto e = data + newSize;
        while (tmp < e)
        {
            *tmp = 0;
            tmp++;
        }
    }
    else
    {
        data = tmp;
        memset(data, 0, newSize);
    }
    this->length = newSize;
}
} // namespace AppCUI::Utils
