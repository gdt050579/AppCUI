#include "AppCUI.hpp"
#include <string.h>

using namespace AppCUI::Utils;

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
        data   = new unsigned char[size];
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
        data = new unsigned char[buf.length];
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
        data = new unsigned char[buf.length];
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