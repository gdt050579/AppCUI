#include "AppCUI.hpp"

namespace AppCUI
{
using namespace OS;
using namespace Utils;

IFile::~IFile()
{
}
bool IFile::ReadBuffer(void*, uint32, uint32&)
{
    NOT_IMPLEMENTED(false);
}
bool IFile::WriteBuffer(const void*, uint32, uint32&)
{
    NOT_IMPLEMENTED(false);
}
uint64 IFile::GetSize()
{
    NOT_IMPLEMENTED(0);
}
uint64 IFile::GetCurrentPos()
{
    NOT_IMPLEMENTED(0);
}
bool IFile::SetSize(uint64)
{
    NOT_IMPLEMENTED(false);
}
bool IFile::SetCurrentPos(uint64)
{
    NOT_IMPLEMENTED(false);
}
void IFile::Close()
{
}

bool IFile::Read(void* buffer, uint32 bufferSize)
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
bool IFile::Write(const void* buffer, uint32 bufferSize)
{
    uint32 temp;
    CHECK(this->WriteBuffer(buffer, bufferSize, temp), false, "Fail to read %lld bytes", bufferSize);
    CHECK(temp == bufferSize, false, "Unable to write %lld bytes required (only %lld were written)", bufferSize, temp);
    return true;
}
bool IFile::Read(uint64 offset, void* buffer, uint32 bufferSize, uint32& bytesRead)
{
    bytesRead = 0;
    CHECK(this->SetCurrentPos(offset), false, "Fail to move cursor to offset: %lld", offset);
    CHECK(this->ReadBuffer(buffer, bufferSize, bytesRead), false, "Fail to read %lld bytes", bufferSize);
    return true;
}
bool IFile::Write(uint64 offset, const void* buffer, uint32 bufferSize, uint32& bytesWritten)
{
    bytesWritten = 0;
    CHECK(this->SetCurrentPos(offset), false, "Fail to move cursor to offset: %lld", offset);
    CHECK(this->WriteBuffer(buffer, bufferSize, bytesWritten), false, "Fail to read %lld bytes", bufferSize);
    return true;
}
bool IFile::Read(void* buffer, uint32 bufferSize, uint32& bytesRead)
{
    return this->ReadBuffer(buffer, bufferSize, bytesRead);
}
bool IFile::Write(const void* buffer, uint32 bufferSize, uint32& bytesWritten)
{
    return this->WriteBuffer(buffer, bufferSize, bytesWritten);
}

bool IFile::Write(string_view text)
{
    return Write(reinterpret_cast<const void*>(text.data()), static_cast<uint32>(text.length()));
}
bool IFile::Write(uint64 offset, string_view text, uint32& bytesWritten)
{
    return Write(offset, reinterpret_cast<const void*>(text.data()), static_cast<uint32>(text.length()), bytesWritten);
}

//======================================================================================[Static methods from File]===
Buffer File::ReadContent(const std::filesystem::path& path)
{
    File f;
    CHECK(f.OpenRead(path), Buffer(), "Fail to open: %s", path.string().c_str());
    CHECK(f.SetCurrentPos(0),
          Buffer(),
          "Fail to position the current pointer to the start of the file: %s",
          path.string().c_str());
    auto file_size = f.GetSize();
    CHECK(file_size > 0, Buffer(), "Empty file (%s)!", path.string().c_str());
    CHECK(file_size < 0xFFFFFFF, Buffer(), "File size exceed 0xFFFFF bytes (%s)", path.string().c_str());
    Buffer buf(file_size);
    CHECK(f.Read(buf.GetData(), (uint32) file_size),
          Buffer(),
          "Fail to read %u bytes from the file %s",
          (uint32) file_size,
          path.string().c_str());
    f.Close();
    return buf;
}
bool File::WriteContent(const std::filesystem::path& path, BufferView buf)
{
    File f;
    CHECK(buf.GetLength() < 0xFFFFFFF,
          false,
          "Buffer size exceed 0xFFFFF bytes --> cannot create (%s)",
          path.string().c_str());
    CHECK(f.Create(path, true), false, "Fail to create: %s", path.string().c_str());
    if (buf.Empty())
    {
        f.Close(); // empty file
        return true;
    }
    CHECK(f.Write(buf.GetData(), (uint32) buf.GetLength()),
          false,
          "Fail to write %u bytes into %s",
          (uint32) buf.GetLength(),
          path.string().c_str());
    f.Close();
    return true;
}
} // namespace AppCUI