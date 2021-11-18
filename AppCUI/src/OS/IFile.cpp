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

bool IFile::Write(std::string_view text)
{
    return Write(reinterpret_cast<const void*>(text.data()), static_cast<unsigned int>(text.length()));
}
bool IFile::Write(unsigned long long offset, std::string_view text, unsigned int& bytesWritten)
{
    return Write(offset, reinterpret_cast<const void*>(text.data()), static_cast<unsigned int>(text.length()), bytesWritten);
}
