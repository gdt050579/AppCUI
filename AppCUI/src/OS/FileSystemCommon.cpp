#include "Internal.h"

using namespace AppCUI::OS;

// remove some Windows defined macros
#ifdef DeleteFile
#   undef DeleteFile
#endif
#ifdef CopyFile
#   undef CopyFile
#endif

struct __internal_delete_folder_struct__
{
	DeleteFileCallback	callback;
	void							*Context;
};
bool delete_folder_callback(const char *fullPath, const char* Name, unsigned long long size, bool Folder, void *Context)
{
	__internal_delete_folder_struct__ *idfs = (__internal_delete_folder_struct__*)Context;
	if ((idfs != nullptr) && (idfs->callback != nullptr) && (Folder==false))
	{
		CHECK(idfs->callback(fullPath, idfs->Context), false, "Delete interupted by user for: %s", fullPath);
	}
	// all is good 
	if (Folder == false)
	{
		CHECK(FileSystem::DeleteFile(fullPath), false, "");
		return true;
	}
	// altfel am un folder - sterg diferit
    CHECK(FileSystem::DeleteEmptyFoler(fullPath), false, "Fail to delete empty path: %s", fullPath);
    return true;
}


bool FileSystem::EnumerateFiles(const char * path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles)
{
    AppCUI::Utils::LocalString<2048> tmp;
	CHECK(tmp.Set(path), false, "Fail to copy %s to string variable", path);
	return EnumerateFiles(tmp, recursive, callback, Context, callFolderCallbackAfterProcessingAllOfItsFiles);
}

bool FileSystem::DeleteFolder(AppCUI::Utils::String &name, bool failIfFileIsMissing, DeleteFileCallback callback, void *Context)
{
	return DeleteFolder(name.GetText(), failIfFileIsMissing, callback, Context);
}
bool FileSystem::DeleteFolder(const char *name, bool failIfFileIsMissing, DeleteFileCallback callback, void *Context)
{
	CHECK(name != nullptr, false, "Expecting a non-null name for directory name !");
	if (FileSystem::DirectoryExists(name) == false)
		return failIfFileIsMissing;
	__internal_delete_folder_struct__ tmp;
	tmp.callback = callback;
	tmp.Context = Context;
	return EnumerateFiles(name, true, delete_folder_callback, &tmp, true);

}

bool FileSystem::CopyFile(const char* source, const char* destination, CopyFileCallback callback, void* Context)
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

