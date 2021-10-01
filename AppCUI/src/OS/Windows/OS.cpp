#include "Internal.hpp"

using namespace AppCUI::OS;

void AddSpecialFolder(
      REFKNOWNFOLDERID specialFolerID, SpecialFolder specialType, const char* name, SpecialFolderMap& specialFolders)
{
    LPWSTR resultPath = nullptr;
    HRESULT hr;
    hr = SHGetKnownFolderPath(specialFolerID, KF_FLAG_CREATE, NULL, &resultPath);
    if (SUCCEEDED(hr))
    {
        specialFolders[specialType] = { name, resultPath };
    }
    if (resultPath)
        CoTaskMemFree(resultPath);
}

void AppCUI::OS::GetSpecialFolders(SpecialFolderMap& specialFolders, RootsVector& roots)
{
    char drivePath[4] = "_:\\";
    std::string name;
    for (char drv = 'A'; drv <= 'Z'; drv++)
    {
        drivePath[0] = drv;
        UINT type    = GetDriveTypeA(drivePath);
        name         = drivePath;
        switch (type)
        {
        case DRIVE_CDROM:
            name += " (CD-ROM)";
            break;
        case DRIVE_FIXED:
            name += " (Local)";
            break;
        case DRIVE_REMOVABLE:
            name += " (Removable)";
            break;
        case DRIVE_RAMDISK:
            name += " (RAM-Drive)";
            break;
        case DRIVE_REMOTE:
            name += " (Remote)";
            break;
        default:
            continue;
        }
        roots.push_back({ name, drivePath });
    }

    // special folers
    AddSpecialFolder(FOLDERID_Desktop, SpecialFolder::Desktop, "Desktop", specialFolders);
    AddSpecialFolder(FOLDERID_Documents, SpecialFolder::Documents, "Documents", specialFolders);
    AddSpecialFolder(FOLDERID_Downloads, SpecialFolder::Downloads, "Downloads", specialFolders);
    AddSpecialFolder(FOLDERID_Music, SpecialFolder::Music, "Music", specialFolders);
    AddSpecialFolder(FOLDERID_Pictures, SpecialFolder::Pictures, "Pictures", specialFolders);
    AddSpecialFolder(FOLDERID_Videos, SpecialFolder::Videos, "Videos", specialFolders);
}

std::filesystem::path AppCUI::OS::GetCurrentApplicationPath()
{
    WCHAR path[1024];
    DWORD nrChars = GetModuleFileNameW(NULL, path, (sizeof(path) / sizeof(WCHAR)) - 1);
    if (nrChars == 0)
        return std::filesystem::path(); // empty path
    if (nrChars <= ((sizeof(path) / sizeof(WCHAR)) - 1))
    {
        path[nrChars] = 0; // add the last NULL char
        return std::filesystem::path(path);
    }
    // path is larger than 1024 character
    try
    {
        WCHAR* tempPath = new WCHAR[(size_t) nrChars + 2];
        nrChars         = GetModuleFileNameW(NULL, tempPath, nrChars + 1);
        if (nrChars == 0)
        {
            delete[] tempPath;
            return std::filesystem::path(); // empty path
        }
        tempPath[nrChars] = 0;
        std::filesystem::path app_path(tempPath);
        delete[] tempPath;
        return app_path;
    }
    catch (...)
    {
        // exception -> return an empty path
        return std::filesystem::path();
    }
}