#include "../Internal.h"

using namespace AppCUI::OS;

#define INVALID_FILE_HANDLE -1

File::File() {
    this->FileID.fid = INVALID_FILE_HANDLE;
}

File::~File() {
    this->Close();
}

bool File::OpenWrite(const char * filePath) {
    CHECK(filePath != nullptr, false, "Invalid NULL file path.");
    Close();
    int fileId = open(filePath, O_RDWR | O_APPEND);
    CHECK(fileId >= 0, false, "ERROR: %s", strerror(errno));
    this->FileID.fid = fileId;
    return true;
}

bool File::OpenRead(const char * filePath) {
    CHECK(filePath != nullptr, false, "Invalid NULL file path.");
    Close();
    int fileId = open(filePath, O_RDONLY);
    CHECK(fileId >= 0, false, "ERROR: %s", strerror(errno));
    this->FileID.fid = fileId;
    return true;
}

bool File::Create(const char * filePath, bool overwriteExisting) {
    CHECK(filePath != nullptr, false, "Invalid NULL file path.");
    Close();
    int fileId = open(filePath, overwriteExisting? O_CREAT | O_RDWR | O_EXCL: O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    CHECK(fileId >= 0, false, "ERROR: %s", strerror(errno));
    this->FileID.fid = fileId;
    return true;
}

bool File::ReadBuffer(void* buffer, unsigned int bufferSize, unsigned int & bytesRead) {
    bytesRead = 0;
    CHECK(this->FileID.fid != INVALID_FILE_HANDLE, false, "Invalid file handle.");
    CHECK(buffer, false, "Invalid NULL buffer.");
    CHECK(bufferSize > 0, false, "Invalid bufferSize");
    ssize_t readSize = read(this->FileID.fid, buffer, bufferSize);
    CHECK(readSize > 0, false, "ERROR: %s", strerror(errno));
    bytesRead = readSize;
}

bool File::WriteBuffer(const void* buffer, unsigned int bufferSize, unsigned int & bytesWritten) {
    bytesWritten = 0;
    CHECK(this->FileID.fid != INVALID_FILE_HANDLE, false, "Invalid file handle.");
    CHECK(buffer, false, "Invalid NULL buffer.");
    CHECK(bufferSize > 0, false, "Invalid bufferSize");
    ssize_t writeSize = write(this->FileID.fid, buffer, bufferSize);
    CHECK(writeSize > 0, false, "ERROR: %s", strerror(errno));
    bytesWritten = writeSize;
}

unsigned long long File::GetSize() {
    struct stat st;
    CHECK(this->FileID.fid != INVALID_FILE_HANDLE, false, "Invalid file handle.");
    CHECK(fstat(this->FileID.fid, &st) == 0, 0, "ERROR: %s", strerror(errno));
    return st.st_size;
}

unsigned long long File::GetCurrentPos() {
    CHECK(this->FileID.fid != INVALID_FILE_HANDLE, false, "Invalid file handle.");
    off_t position = lseek(this->FileID.fid, 0, SEEK_CUR);
    CHECK(position >= 0, 0, "ERROR: %s", strerror(errno));
    return position;
}

bool File::SetSize(unsigned long long newSize) {
    CHECK(this->FileID.fid != INVALID_FILE_HANDLE, false, "Invalid file handle.")
    CHECK(SetCurrentPos(newSize), false, "ERROR: %s", strerror(errno));
    CHECK(ftruncate(this->FileID.fid, newSize) >= 0, false, "ERROR: %s", strerror(errno));
    return true;
}

bool File::SetCurrentPos(unsigned long long newPosition) {
    CHECK(this->FileID.fid != INVALID_FILE_HANDLE, false, "Invalid file handle.");
    off_t position = newPosition;
    off_t setPosition = lseek(this->FileID.fid, position, SEEK_SET);
    CHECK(setPosition >= 0, false, "ERROR: %s", strerror(errno));
    CHECK(setPosition == position, false, "ERROR: Set position differs %s", strerror(errno));
    return true;
}

void File::Close() {
    if (this->FileID.fid != INVALID_FILE_HANDLE) {
        close(FileID.fid);
        this->FileID.fid = INVALID_FILE_HANDLE;
    }
}
