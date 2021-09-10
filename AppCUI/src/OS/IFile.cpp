#include "AppCUI.hpp"

using namespace AppCUI::OS;

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
std::unique_ptr<char[]> IFile::ReadContentToBuffer(unsigned int& bufferSize)
{
    bufferSize = 0;
    CHECK(SetCurrentPos(0), nullptr, "Fail to position the current pointer to the start of the file");
    unsigned long long file_size = this->GetSize();
    CHECK(file_size > 0, nullptr, "Empty file !");
    CHECK(file_size < 0xFFFFFF, nullptr, "File size exceed 4G size");
    auto buf = std::make_unique<char[]>(file_size);
    CHECK(this->Read(buf.get(), (unsigned int) file_size),
          nullptr,
          "Fail to read %d bytes from the file",
          (unsigned int) file_size);
    bufferSize = (unsigned int) file_size;
    return buf;
}
bool IFile::Write(std::string_view text)
{
    return Write(reinterpret_cast<const void*>(text.data()), static_cast<unsigned int>(text.length()));
}
bool IFile::Write(unsigned long long offset, std::string_view text, unsigned int& bytesWritten)
{
    return Write(offset, reinterpret_cast<const void*>(text.data()), static_cast<unsigned int>(text.length()), bytesWritten);
}
