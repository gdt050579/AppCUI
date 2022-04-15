#include "AppCUI.hpp"

namespace AppCUI
{
using namespace OS;
bool MemoryFile::ReadBuffer(void* buffer, uint32 bufferSize, uint32& bytesRead)
{
    bytesRead = 0;
    CHECK(buffer, false, "Expecting a valid (non-null) buffer");
    if ((this->pos >= this->size) || (bufferSize == 0))
        return true;
    auto sizeToRead = std::min<>(this->size - this->pos, (uint64) bufferSize);
    memcpy(buffer, this->buffer + this->pos, sizeToRead);
    bytesRead = (uint32) sizeToRead;
    this->pos += sizeToRead;
    return true;
}
bool MemoryFile::WriteBuffer(const void* buffer, uint32 bufferSize, uint32& bytesWritten)
{
    bytesWritten = 0;
    CHECK(buffer, false, "Expecting a valid (non-null) buffer");
    auto newPos = (uint64) bufferSize + this->pos;
    if (newPos > this->size)
    {
        CHECK(SetSize(newPos), false, "Fail to resize buffer !");
    }
    memcpy(this->buffer + this->pos, buffer, bufferSize);
    this->pos    = newPos;
    bytesWritten = bufferSize;
    return true;
}
MemoryFile::MemoryFile()
{
    this->buffer    = nullptr;
    this->pos       = 0;
    this->size      = 0;
    this->allocated = 0;
}
MemoryFile::~MemoryFile()
{
    Close();
}

bool MemoryFile::Create(uint64 allocatedMemory)
{
    Close();
    CHECK(allocatedMemory < 0x7FFFFFFFFFFFFFFFULL, false, "size is too high");
    auto toAlloc = (allocatedMemory | 0x1F) + 1; // align to 32 bytes
    try
    {
        this->buffer    = new uint8[toAlloc];
        this->allocated = toAlloc;
        this->size      = 0;
        this->pos       = 0;
        memset(this->buffer, 0, toAlloc);
        return true;
    }
    catch (...)
    {
        RETURNERROR(false, "Fail to allocate memory for internal buffer");
    }
}
bool MemoryFile::Create(const void* _buffer, uint64 _size)
{
    CHECK(Create(_size), false, "Fail to create buffers !");
    if (_buffer)
    {
        memcpy(this->buffer, _buffer, _size);
        this->pos  = _size;
        this->size = _size;
    }
    return true;
}

uint64 MemoryFile::GetSize()
{
    CHECK(this->buffer, 0, "Memory file not instantiated. Have you call Create method ?");
    return size;
}
uint64 MemoryFile::GetCurrentPos()
{
    CHECK(this->buffer, 0, "Memory file not instantiated. Have you call Create method ?");
    return pos;
}
bool MemoryFile::SetSize(uint64 newSize)
{
    CHECK(this->buffer, false, "Memory file not instantiated. Have you call Create method ?");
    if (newSize < size)
    {
        this->size = newSize;
        this->pos  = std::min<>(this->pos, newSize);
        return true;
    }
    if ((newSize > size) && (newSize <= allocated))
    {
        memset(this->buffer + size, 0, newSize - size);
        this->size = newSize;
        return true;
    }
    if (newSize > allocated)
    {
        // need to resize first
        CHECK(newSize < 0x7FFFFFFFFFFFFFFFULL, false, "size is too high");
        auto toAlloc = (newSize | 0x1F) + 1; // align to 32 bytes
        try
        {
            auto newBuf = new uint8[toAlloc];
            memcpy(newBuf, this->buffer, this->size);
            memset(newBuf + this->size, 0, toAlloc - this->size);
            this->size      = newSize;
            this->allocated = toAlloc;
            delete[] this->buffer;
            this->buffer = newBuf;
            return true;
        }
        catch (...)
        {
            RETURNERROR(false, "Fail to increase the size of the internal buffer");
        }
    }
    RETURNERROR(false, "Fail to increase the size of the internal buffer");
}
bool MemoryFile::SetCurrentPos(uint64 newPosition)
{
    CHECK(newPosition < 0x7FFFFFFFFFFFFFFFULL, false, "size is too high");
    this->pos = newPosition;
    return true;
}
void MemoryFile::Close()
{
    if (this->buffer)
        delete[] this->buffer;
    this->buffer    = nullptr;
    this->pos       = 0;
    this->size      = 0;
    this->allocated = 0;
}
} // namespace AppCUI