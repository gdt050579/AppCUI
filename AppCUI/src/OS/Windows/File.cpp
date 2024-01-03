#include "Internal.hpp"

namespace AppCUI
{
using namespace OS;
using namespace Utils;

#define VALIDATE_FILE_HANLDE(returnValue)                                                                              \
    CHECK(this->FileID.Handle != INVALID_HANDLE_VALUE, returnValue, "File has not been opened!");
#define F_HNDL ((HANDLE) this->FileID.Handle)

static const std::u16string longPathPrefix{ uR"(\\?\)" };

File::File()
{
    FileID.Handle = INVALID_HANDLE_VALUE;
}

File::~File()
{
    Close();
}

bool File::OpenWrite(const std::filesystem::path& filePath)
{
    Close();

    // sanitize -> https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=powershell
    auto path      = filePath.u16string();
    auto u16svPath = std::u16string_view{ path.data(), path.size() };

    LocalUnicodeStringBuilder<2048> longPath;
    if (u16svPath.size() >= MAX_PATH && u16svPath.starts_with(u"\\") == false && u16svPath.starts_with(u"\?") == false)
    {
        std::replace(path.begin(), path.end(), L'/', L'\\');

        CHECK(longPath.Add(longPathPrefix), false, "");
        CHECK(longPath.Add(path), false, "");
        CHECK(longPath.AddChar(u'\0'), false, "");
        u16svPath = longPath.ToStringView();
    }

    HANDLE hFile = CreateFileW(
          (LPCWSTR) u16svPath.data(),
          GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL,
          OPEN_EXISTING,
          0,
          NULL);
    CHECK(hFile != INVALID_HANDLE_VALUE,
          false,
          "Fail to create: %s ==> Error code: %d",
          filePath.string().c_str(),
          GetLastError());

    CHECK(SetFilePointer(hFile, 0, NULL, FILE_END) != INVALID_SET_FILE_POINTER,
          false,
          "Fail to set file pointer at the end of the file !");

    FileID.Handle = hFile;

    return true;
}

bool File::OpenRead(const std::filesystem::path& filePath)
{
    Close();

    // sanitize -> https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=powershell
    auto path      = filePath.u16string();
    auto u16svPath = std::u16string_view{ path.data(), path.size() };

    LocalUnicodeStringBuilder<2048> longPath;
    if (u16svPath.size() >= MAX_PATH && u16svPath.starts_with(u"\\") == false && u16svPath.starts_with(u"\?") == false)
    {
        std::replace(path.begin(), path.end(), L'/', L'\\');

        CHECK(longPath.Add(longPathPrefix), false, "");
        CHECK(longPath.Add(path), false, "");
        CHECK(longPath.AddChar(u'\0'), false, "");
        u16svPath = longPath.ToStringView();
    }

    HANDLE hFile = CreateFileW(
          (LPCWSTR) u16svPath.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    CHECK(hFile != INVALID_HANDLE_VALUE,
          false,
          "Fail to create: %s ==> Error code: %d",
          filePath.string().c_str(),
          GetLastError());

    FileID.Handle = hFile;

    return true;
}

bool File::Create(const std::filesystem::path& filePath, bool overwriteExisting)
{
    Close();

    // sanitize -> https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=powershell
    auto path      = filePath.u16string();
    auto u16svPath = std::u16string_view{ path.data(), path.size() };

    LocalUnicodeStringBuilder<2048> longPath;
    if (u16svPath.size() >= MAX_PATH && u16svPath.starts_with(u"\\") == false && u16svPath.starts_with(u"\?") == false)
    {
        std::replace(path.begin(), path.end(), L'/', L'\\');

        CHECK(longPath.Add(longPathPrefix), false, "");
        CHECK(longPath.Add(path), false, "");
        CHECK(longPath.AddChar(u'\0'), false, "");
        u16svPath = longPath.ToStringView();
    }

    HANDLE hFile = CreateFileW(
          (LPCWSTR) u16svPath.data(),
          GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL,
          overwriteExisting ? CREATE_ALWAYS : CREATE_NEW,
          0,
          NULL);
    CHECK(hFile != INVALID_HANDLE_VALUE,
          false,
          "Fail to create: %s ==> Error code: %d",
          filePath.string().c_str(),
          GetLastError());

    FileID.Handle = hFile;

    return true;
}

bool File::ReadBuffer(void* buffer, uint32 bufferSize, uint32& bytesRead)
{
    bytesRead = 0;
    VALIDATE_FILE_HANLDE(false);
    CHECK(buffer, false, "Expecting a valid (non-null) buffer !");
    CHECK(bufferSize, false, "Expecting a valid (bigger than 0) size for the buffer ");

    DWORD nrBytesRead = 0;
    CHECK(ReadFile(F_HNDL, buffer, bufferSize, &nrBytesRead, NULL),
          false,
          "Reading from file failed with code: %d",
          GetLastError());

    bytesRead = nrBytesRead;

    return true;
}

bool File::WriteBuffer(const void* buffer, uint32 bufferSize, uint32& bytesWritten)
{
    bytesWritten = 0;
    VALIDATE_FILE_HANLDE(false);
    CHECK(buffer, false, "Expecting a valid (non-null) buffer !");
    CHECK(bufferSize, false, "Expecting a valid (bigger than 0) size for the buffer ");

    DWORD nrBytesWrittem = 0;
    CHECK(WriteFile(F_HNDL, buffer, bufferSize, &nrBytesWrittem, NULL),
          false,
          "Reading from file failed with code: %d",
          GetLastError());

    bytesWritten = nrBytesWrittem;

    return true;
}

uint64 File::GetSize()
{
    VALIDATE_FILE_HANLDE(0);
    LARGE_INTEGER size;
    CHECK(GetFileSizeEx(F_HNDL, &size), 0, "GetFileSizeEx failed !");
    return (uint64) size.QuadPart;
}

uint64 File::GetCurrentPos()
{
    VALIDATE_FILE_HANLDE(0);
    LARGE_INTEGER pos;
    pos.QuadPart = 0;
    CHECK(SetFilePointerEx(F_HNDL, pos, &pos, FILE_CURRENT), 0, "SetFilePointerEx failed !");
    return (uint64) pos.QuadPart;
}

bool File::SetSize(uint64 newSize)
{
    VALIDATE_FILE_HANLDE(false);
    CHECK(SetCurrentPos(newSize), false, "Error setting current size pointer !");
    CHECK(SetEndOfFile(F_HNDL), false, "Error on SetEndOfFile !");
    return true;
}

bool File::SetCurrentPos(uint64 newPosition)
{
    VALIDATE_FILE_HANLDE(false);
    LARGE_INTEGER res;
    res.QuadPart = newPosition;
    CHECK(SetFilePointerEx(F_HNDL, res, &res, FILE_BEGIN), false, "SetFilePointerEx failed !");
    CHECK(res.QuadPart == newPosition, false, "SetFilePointerEx failed !");
    return true;
}

void File::Close()
{
    if (FileID.Handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(F_HNDL);
        FileID.Handle = INVALID_HANDLE_VALUE;
    }
}
} // namespace AppCUI

#undef VALIDATE_FILE_HANLDE
#undef F_HNDL