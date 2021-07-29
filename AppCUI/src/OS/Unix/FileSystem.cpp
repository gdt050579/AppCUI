#include "Internal.hpp"

using namespace AppCUI::OS;

// Enumerate files & folders
bool		FileSystem::EnumerateFiles(AppCUI::Utils::String &path, bool recursive, EnumerateFilesCallback callback, void *Context, bool callFolderCallbackAfterProcessingAllOfItsFiles)
{
	NOT_IMPLEMENTED(false);
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

bool        FileSystem::DeleteEmptyFoler(const char * fullPath)
{
	NOT_IMPLEMENTED(false);
}
bool		FileSystem::DeleteFile(const char *name, bool failIfFileIsMissing)
{
	NOT_IMPLEMENTED(false);
}
bool         FileSystem::GetSpecialFolderName(SpecialFolder id, AppCUI::Utils::String& name)
{
	NOT_IMPLEMENTED(false);
}

