#include "AppCUI.hpp"

using namespace AppCUI::OS;
using namespace AppCUI::Utils;

IFile::~IFile()
{
}
bool IFile::ReadBuffer(void*, unsigned int, unsigned int&)
{
    NOT_IMPLEMENTED(false);
}
bool IFile::WriteBuffer(const void*, unsigned int, unsigned int&)
{
    NOT_IMPLEMENTED(false);
}
unsigned long long IFile::GetSize()
{
    NOT_IMPLEMENTED(0);
}
unsigned long long IFile::GetCurrentPos()
{
    NOT_IMPLEMENTED(0);
}
bool IFile::SetSize(unsigned long long)
{
    NOT_IMPLEMENTED(false);
}
bool IFile::SetCurrentPos(unsigned long long)
{
    NOT_IMPLEMENTED(false);
}
void IFile::Close()
{
}

bool IFile::Read(void* buffer, unsigned int bufferSize)
{
    unsigned int temp;
    CHECK(this->ReadBuffer(buffer, bufferSize, temp), false, "Fail to read %lld bytes", bufferSize);
    CHECK(temp == bufferSize,
          false,
          "Unable to read %lld bytes required (only %lld bytes available)",
          bufferSize,
          temp);
    return true;
}
bool IFile::Write(const void* buffer, unsigned int bufferSize)
{
    unsigned int temp;
    CHECK(this->WriteBuffer(buffer, bufferSize, temp), false, "Fail to read %lld bytes", bufferSize);
    CHECK(temp == bufferSize, false, "Unable to write %lld bytes required (only %lld were written)", bufferSize, temp);
    return true;
}
bool IFile::Read(unsigned long long offset, void* buffer, unsigned int bufferSize, unsigned int& bytesRead)
{
    bytesRead = 0;
    CHECK(this->SetCurrentPos(offset), false, "Fail to move cursor to offset: %lld", offset);
    CHECK(this->ReadBuffer(buffer, bufferSize, bytesRead), false, "Fail to read %lld bytes", bufferSize);
    return true;
}
bool IFile::Write(unsigned long long offset, const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten)
{
    bytesWritten = 0;
    CHECK(this->SetCurrentPos(offset), false, "Fail to move cursor to offset: %lld", offset);
    CHECK(this->WriteBuffer(buffer, bufferSize, bytesWritten), false, "Fail to read %lld bytes", bufferSize);
    return true;
}
bool IFile::Read(void* buffer, unsigned int bufferSize, unsigned int& bytesRead)
{
    return this->ReadBuffer(buffer, bufferSize, bytesRead);
}
bool IFile::Write(const void* buffer, unsigned int bufferSize, unsigned int& bytesWritten)
{
    return this->WriteBuffer(buffer, bufferSize, bytesWritten);
}

bool IFile::Write(std::string_view text)
{
    return Write(reinterpret_cast<const void*>(text.data()), static_cast<unsigned int>(text.length()));
}
bool IFile::Write(unsigned long long offset, std::string_view text, unsigned int& bytesWritten)
{
    return Write(
          offset, reinterpret_cast<const void*>(text.data()), static_cast<unsigned int>(text.length()), bytesWritten);
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
    CHECK(f.Read(buf.GetData(), (unsigned int) file_size),
          Buffer(),
          "Fail to read %u bytes from the file %s",
          (unsigned int) file_size,
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
    CHECK(f.Write(buf.GetData(), (unsigned int) buf.GetLength()),
          false,
          "Fail to write %u bytes into %s",
          (unsigned int) buf.GetLength(),
          path.string().c_str());
    f.Close();
    return true;
}
bool File::WriteContent(const std::filesystem::path& path, std::string_view text)
{
    return WriteContent(path, BufferView(text));
}