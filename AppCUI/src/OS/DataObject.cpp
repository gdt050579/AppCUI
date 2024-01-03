#include "AppCUI.hpp"

namespace AppCUI::OS
{
using namespace OS;
using namespace Utils;

DataObject::~DataObject()
{
}
bool DataObject::ReadBuffer(void*, uint32, uint32&)
{
    NOT_IMPLEMENTED(false);
}
bool DataObject::WriteBuffer(const void*, uint32, uint32&)
{
    NOT_IMPLEMENTED(false);
}
uint64 DataObject::GetSize()
{
    NOT_IMPLEMENTED(0);
}
uint64 DataObject::GetCurrentPos()
{
    NOT_IMPLEMENTED(0);
}
bool DataObject::SetSize(uint64)
{
    NOT_IMPLEMENTED(false);
}
bool DataObject::SetCurrentPos(uint64)
{
    NOT_IMPLEMENTED(false);
}
void DataObject::Close()
{
}

//====================================[READ METHODS]==========================================
bool DataObject::Read(void* buffer, uint32 bufferSize)
{
    uint32 temp;
    CHECK(this->ReadBuffer(buffer, bufferSize, temp), false, "Fail to read %lld bytes", bufferSize);
    CHECK(temp == bufferSize,
          false,
          "Unable to read %lld bytes required (only %lld bytes available)",
          bufferSize,
          temp);
    return true;
}
bool DataObject::Read(uint64 offset, void* buffer, uint32 bufferSize, uint32& bytesRead)
{
    bytesRead = 0;
    CHECK(this->SetCurrentPos(offset), false, "Fail to move cursor to offset: %lld", offset);
    CHECK(this->ReadBuffer(buffer, bufferSize, bytesRead), false, "Fail to read %lld bytes", bufferSize);
    return true;
}
bool DataObject::Read(void* buffer, uint32 bufferSize, uint32& bytesRead)
{
    return this->ReadBuffer(buffer, bufferSize, bytesRead);
}
bool DataObject::Read(AppCUI::Utils::Buffer& buf, uint32 size)
{
    buf.Resize(size);
    return Read((void*) buf.GetData(), size);
}
bool DataObject::Read(uint64 offset, AppCUI::Utils::Buffer& buf, uint32 size)
{
    uint32 bytesRead;
    buf.Resize(size);

    if ((Read(offset, (void*) buf.GetData(), size, bytesRead) == false) || (bytesRead == size))
    {
        buf.Resize(0);
        return false;
    }
    return true;
}
//====================================[WRITE METHODS]=========================================
bool DataObject::Write(const void* buffer, uint32 bufferSize)
{
    uint32 temp;
    CHECK(this->WriteBuffer(buffer, bufferSize, temp), false, "Fail to read %lld bytes", bufferSize);
    CHECK(temp == bufferSize, false, "Unable to write %lld bytes required (only %lld were written)", bufferSize, temp);
    return true;
}
bool DataObject::Write(uint64 offset, const void* buffer, uint32 bufferSize, uint32& bytesWritten)
{
    bytesWritten = 0;
    CHECK(this->SetCurrentPos(offset), false, "Fail to move cursor to offset: %lld", offset);
    CHECK(this->WriteBuffer(buffer, bufferSize, bytesWritten), false, "Fail to read %lld bytes", bufferSize);
    return true;
}
bool DataObject::Write(const void* buffer, uint32 bufferSize, uint32& bytesWritten)
{
    return this->WriteBuffer(buffer, bufferSize, bytesWritten);
}
bool DataObject::Write(string_view text)
{
    return Write(reinterpret_cast<const void*>(text.data()), static_cast<uint32>(text.length()));
}
bool DataObject::Write(uint64 offset, string_view text, uint32& bytesWritten)
{
    return Write(offset, reinterpret_cast<const void*>(text.data()), static_cast<uint32>(text.length()), bytesWritten);
}
} // namespace AppCUI