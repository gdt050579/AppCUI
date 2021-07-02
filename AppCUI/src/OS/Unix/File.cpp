#include "../Internal.h"

using namespace AppCUI::OS;


File::File()
{
    // instantiate file handle / or file ID to an invalid value
}
File::~File()
{
    this->Close();
}
bool                File::OpenWrite(const char * filePath)
{
    NOT_IMPLEMENTED(false);
}
bool                File::OpenRead(const char * filePath)
{
    NOT_IMPLEMENTED(false);
}
bool                File::Create(const char * filePath, bool overwriteExisting)
{
    NOT_IMPLEMENTED(false);
}
bool                File::Read(void* buffer, unsigned int bufferSize, unsigned int & bytesRead)
{
    NOT_IMPLEMENTED(false);
}
bool                File::Write(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten)
{
    NOT_IMPLEMENTED(false);
}
unsigned long long  File::GetSize()
{
    NOT_IMPLEMENTED(0);
}
unsigned long long  File::GetCurrentPos()
{
    NOT_IMPLEMENTED(0);
}
bool                File::SetSize(unsigned long long newSize)
{
    NOT_IMPLEMENTED(false);
}
bool                File::SetCurrentPos(unsigned long long newPosition)
{
    NOT_IMPLEMENTED(false);
}
void                File::Close()
{

}