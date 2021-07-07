#include <os.h>

using namespace AppCUI::OS;

// Enumerate files & folders
bool		FileSystem::EnumerateFiles(AppCUI::Utils::String &path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles)
{
    NOT_IMPLEMENTED(false);
}
bool		FileSystem::EnumerateFiles(const char * path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles)
{
    	AppCUI::Utils::LocalString<2048> tmp;
	CHECK(tmp.Set(path), false, "Fail to copy %s to string variable", path);
	return EnumerateFiles(tmp, recursive, callback, Context, callFolderCallbackAfterProcessingAllOfItsFiles);
}

// File & folders existance
bool		FileSystem::FileExists(const char *path)
{
    NOT_IMPLEMENTED(false);
}
bool		FileSystem::DirectoryExists(const char *path)
{
    NOT_IMPLEMENTED(false);
}

bool		FileSystem::GetCurrentDir(AppCUI::Utils::String &path)
{
    NOT_IMPLEMENTED(false);
}

bool		FileSystem::CreateFolder(const char *name)
{
    NOT_IMPLEMENTED(false);
}

bool		FileSystem::DeleteFile(const char *name, bool failIfFileIsMissing)
{
    NOT_IMPLEMENTED(false);
}

bool		FileSystem::DeleteFolder(AppCUI::Utils::String &name, bool failIfFileIsMissing, DeleteFileCallback callback, void *Context)
{
	return DeleteFolder(name.GetText(), failIfFileIsMissing, callback, Context);
}
bool		FileSystem::DeleteFolder(const char *name, bool failIfFileIsMissing, DeleteFileCallback callback, void *Context)
{
    NOT_IMPLEMENTED(false);
}


bool		FileSystem::CopyFile(const char* source, const char* destination, CopyFileCallback callback, void* Context)
{
	CHECK(source != nullptr, false, "");
	CHECK(destination != nullptr, false, "");

	unsigned char       tempBuffer[8192];
	unsigned long long  readSoFar, sourceSize, toRead;
	File fs, fd;
    
	bool continue_copy = true;
	while (true)
	{
		CHECKBK(fs.OpenRead(source), "Fail to open: %s",source);
		CHECKBK(fd.Create(destination), "Fail to create: %s",destination);
        sourceSize = fs.GetSize();
		readSoFar = 0;
		while ((readSoFar < sourceSize) && (continue_copy))
		{
			toRead = sourceSize - readSoFar;
			if (toRead>sizeof(tempBuffer))
				toRead = sizeof(tempBuffer);

			CHECKBK(fs.Read(tempBuffer, (unsigned int)toRead), "Fail to read %d bytes from %s", (unsigned int)toRead,source);
			CHECKBK(fd.Write(tempBuffer, (unsigned int)toRead), "Fail to write %d bytes to %s", (unsigned int)toRead,destination);
			readSoFar += toRead;
			if ((callback != nullptr) && (readSoFar < sourceSize))
				continue_copy = callback(source, destination, readSoFar, sourceSize, Context);
		}
		CHECKBK(readSoFar == sourceSize, "");
		if (callback != nullptr)
			callback(source, destination, readSoFar, sourceSize, Context);
		fs.Close();
		fd.Close();
		return true;
	}
	fs.Close();
	fd.Close();
	return false;
}

