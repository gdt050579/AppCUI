#include "Internal.hpp"

using namespace AppCUI::OS;

// remove some Windows defined macros
#ifdef DeleteFile
#   undef DeleteFile
#endif
#ifdef CopyFile
#   undef CopyFile
#endif


bool __Get_Special_Folder__(FileSystem::SpecialFolder id)
{
	NOT_IMPLEMENTED(false);
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

bool        FileSystem::DeleteEmptyFoler(const char * fullPath)
{
    CHECK(::SetFileAttributesA(fullPath, FILE_ATTRIBUTE_NORMAL), false, "Failed to clear attributes for '%s' -> with error: %08X", fullPath, GetLastError());
    CHECK(::RemoveDirectoryA(fullPath), false, "Failed to delete folder for '%s' -> with error: %08X", fullPath, GetLastError());
    return true;
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

bool         FileSystem::GetSpecialFolderName(SpecialFolder id, AppCUI::Utils::String& name)
{
	UINT type;
	switch (id)
	{
		case SpecialFolder::DriveA:
		case SpecialFolder::DriveB:
		case SpecialFolder::DriveC:
		case SpecialFolder::DriveD:
		case SpecialFolder::DriveE:
		case SpecialFolder::DriveF:
		case SpecialFolder::DriveG:
		case SpecialFolder::DriveH:
		case SpecialFolder::DriveI:
		case SpecialFolder::DriveJ:
		case SpecialFolder::DriveK:
		case SpecialFolder::DriveL:
		case SpecialFolder::DriveM:
		case SpecialFolder::DriveN:
		case SpecialFolder::DriveO:
		case SpecialFolder::DriveP:
		case SpecialFolder::DriveQ:
		case SpecialFolder::DriveR:
		case SpecialFolder::DriveS:
		case SpecialFolder::DriveT:
		case SpecialFolder::DriveU:
		case SpecialFolder::DriveV:
		case SpecialFolder::DriveW:
		case SpecialFolder::DriveX:
		case SpecialFolder::DriveY:
		case SpecialFolder::DriveZ:
			CHECK(name.SetFormat("%c:\\", (((char)id)-(char)SpecialFolder::DriveA) + 'A'), false, "Fail to create special folder name !");
			type = GetDriveTypeA(name.GetText());
			switch (type)
			{
				case DRIVE_CDROM:
					CHECK(name.Add(" (CD-ROM)"), false, "Fail to add type to special folder name !");
					return true;
				case DRIVE_FIXED:
					CHECK(name.Add(" (Local)"), false, "Fail to add type to special folder name !");
					return true;
				case DRIVE_REMOVABLE:
					CHECK(name.Add(" (Removable)"), false, "Fail to add type to special folder name !");
					return true;
				case DRIVE_RAMDISK:
					CHECK(name.Add(" (Ram-Drive)"), false, "Fail to add type to special folder name !");
					return true;
				case DRIVE_REMOTE:
					CHECK(name.Add(" (Remote)"), false, "Fail to add type to special folder name !");
					return true;
			}
			return false; // drive is unknown
		case SpecialFolder::Desktop:
			CHECK(name.Set("Desktop"), false, "Fail to create Desktop string !");
			return true;
		case SpecialFolder::Home:
			CHECK(name.Set("Home"), false, "Fail to create Ho,e string !");
			return true;
	}
	RETURNERROR(false, "Unknwon special folder ID: %d", (unsigned int)id);
}