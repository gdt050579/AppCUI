#include <os.h>

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
	CHECK(::SetFileAttributesA(fullPath, FILE_ATTRIBUTE_NORMAL), false, "Failed to clear attributes for '%s' -> with error: %08X", fullPath, GetLastError());
	CHECK(::RemoveDirectoryA(fullPath), false, "Failed to delete folder for '%s' -> with error: %08X", fullPath, GetLastError());
	return true;
}

// Enumerate files & folders
bool		FileSystem::EnumerateFiles(AppCUI::Utils::String &path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles)
{
	CHECK(callback != nullptr, false, "EnumerateFiles with a nullptr callback has no use !");
	unsigned int len = path.Len();

	WIN32_FIND_DATAA	dt;
	HANDLE				hFindFile;

	CHECK(Path::Join(path,"*.*"), false, "Unable to add mask (*.*) to path variable !");
	hFindFile = FindFirstFileA(path.GetText(), &dt);
	CHECK(path.Truncate(len), false, "Unable to truncate path variable !");
	CHECK(hFindFile != INVALID_HANDLE_VALUE, false, "Invalid path (%s) - ErrorCode: %08X", path.GetText(), GetLastError());
	do {
		CHECK(path.Truncate(len), false, "Unable to truncate path variable !");
		if ((dt.dwFileAttributes & 16) == 0)
		{
			CHECK(Path::Join(path, dt.cFileName), false, "Unable to add file name (%s) to path variable !", dt.cFileName);
			CHECK(callback(path.GetText(),dt.cFileName, (((unsigned long long)dt.nFileSizeHigh) << 32) | ((unsigned long long)dt.nFileSizeLow),false, Context), false, "Callback stop by user !");
			CHECK(path.Truncate(len), false, "Unable to truncate path variable !");
		}
		else {
			if (((Utils::String::Equals(dt.cFileName, ".") == false) && (Utils::String::Equals(dt.cFileName, "..") == false)))
			{
				CHECK(Path::Join(path, dt.cFileName), false, "Unable to add directory name (%s) to path variable !", dt.cFileName);
				if (callFolderCallbackAfterProcessingAllOfItsFiles == false) {
					CHECK(callback(path.GetText(), dt.cFileName, 0, true, Context), false, "Callback stop by user !");
				}
				if (recursive) {
					CHECK(EnumerateFiles(path, recursive, callback, Context, callFolderCallbackAfterProcessingAllOfItsFiles), false, "Error on reading files from folder: %s", path.GetText());
				}
				CHECK(path.Truncate(len), false, "Unable to truncate path variable !");
			}
		}
	} while (FindNextFileA(hFindFile, &dt));
	FindClose(hFindFile);
	if (callFolderCallbackAfterProcessingAllOfItsFiles==true)
	{
		CHECK(path.Truncate(len), false, "Unable to truncate path variable !");
		const char *s = path.GetText();
		const char *f = s;
		while ((*s) != 0)
		{
			if (((*s)=='\\') || ((*s)=='/'))
				f = s+1;
			s++;
		}
		CHECK(callback(path.GetText(), f , 0, true, Context), false, "Callback stop by user !");
	}
	return true;
	
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
	WIN32_FIND_DATAA	FindFileData;
	HANDLE				hFindFile;

	hFindFile = FindFirstFileA(path, &FindFileData);
	if (hFindFile == INVALID_HANDLE_VALUE) 
        return false;
	FindClose(hFindFile);
	return ((FindFileData.dwFileAttributes & 16) == 0); // not a directory
}
bool		FileSystem::DirectoryExists(const char *path)
{
	WIN32_FIND_DATAA	FindFileData;
	HANDLE				hFindFile;

	hFindFile = FindFirstFileA(path, &FindFileData);
	if (hFindFile == INVALID_HANDLE_VALUE) 
        return false;
	FindClose(hFindFile);
	return ((FindFileData.dwFileAttributes & 16) != 0); // not a file	
}


bool		FileSystem::GetCurrentDir(AppCUI::Utils::String &path)
{
    char temp[4096];
	DWORD size = GetCurrentDirectoryA(sizeof(temp)-1, temp);
	CHECK(size>0, false, "Unable to read current directory - ErrorCode: %08X", GetLastError());
    CHECK(size < sizeof(temp), false, "Invalid number of characters written: %d", size);
    temp[size] = 0;
    CHECK(path.Set(temp), false, "Fail to copy '%s' to path variable !",temp);
	if (path.EndsWith("\\") == false)
	{
		CHECK(path.AddChar('\\'),false,"");
	}
	return true;
}

bool		FileSystem::CreateFolder(const char *name)
{
	CHECK(name != nullptr, false, "Expecting a non-null name for directory !");
	if (::CreateDirectoryA(name, nullptr))
		return true;
	DWORD err = GetLastError();
	if (ERROR_ALREADY_EXISTS == err)
		return true;
	RETURNERROR(false, "Unable to create forlder '%s' - error code: %d", name, err);
}

bool		FileSystem::DeleteFile(const char *name, bool failIfFileIsMissing)
{
	CHECK(name != nullptr, false, "Expecting a non-null name for file name !");
	if (FileSystem::FileExists(name) == false)
	{
		return failIfFileIsMissing;
	}
	CHECK(::SetFileAttributesA(name, FILE_ATTRIBUTE_NORMAL), false, "Failed to clear attributes for '%s' -> with error: %08X", name, GetLastError());
	CHECK(::DeleteFileA(name), false, "Failed to delete file for '%s' -> with error: %08X", name, GetLastError());
	return true;

}

bool		FileSystem::DeleteFolder(AppCUI::Utils::String &name, bool failIfFileIsMissing, DeleteFileCallback callback, void *Context)
{
	return DeleteFolder(name.GetText(), failIfFileIsMissing, callback, Context);
}
bool		FileSystem::DeleteFolder(const char *name, bool failIfFileIsMissing, DeleteFileCallback callback, void *Context)
{
	CHECK(name != nullptr, false, "Expecting a non-null name for directory name !");
	if (FileSystem::DirectoryExists(name) == false)
		return failIfFileIsMissing;
	__internal_delete_folder_struct__ tmp;
	tmp.callback = callback;
	tmp.Context = Context;
	return EnumerateFiles(name, true, delete_folder_callback, &tmp, true);

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

