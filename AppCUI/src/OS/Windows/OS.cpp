#include "Internal.hpp"

using namespace AppCUI::OS;

using SpecialFolderList = std::vector<std::pair<std::string, std::filesystem::path>>;

void AddSpecialFolder(REFKNOWNFOLDERID specialFolerID, const char * name, SpecialFolderList& specialFolderList)
{
    LPWSTR resultPath = nullptr;
    HRESULT hr;
    hr = SHGetKnownFolderPath(specialFolerID, KF_FLAG_CREATE, NULL, &resultPath);
    if (SUCCEEDED(hr))
    {
        specialFolderList.push_back({ name, resultPath });
    }
    if (resultPath)
        CoTaskMemFree(resultPath);
}

void AppCUI::OS::GetSpecialFolders(SpecialFolderList& specialFolderList, SpecialFoldersType type, bool clearVector)
{
    if (clearVector)
        specialFolderList.clear();
    if ((type == SpecialFoldersType::All) || (type == SpecialFoldersType::Drives))
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
            specialFolderList.push_back({ name, drivePath });
        }
    }
    if ((type == SpecialFoldersType::All) || (type == SpecialFoldersType::SpecialLocations))
    {
        // special folers
        AddSpecialFolder(FOLDERID_Desktop, "Desktop", specialFolderList);
        AddSpecialFolder(FOLDERID_Documents, "Documents", specialFolderList);
        AddSpecialFolder(FOLDERID_LocalDocuments, "Local Documents", specialFolderList);
        AddSpecialFolder(FOLDERID_Downloads, "Downloads", specialFolderList);
        AddSpecialFolder(FOLDERID_LocalDownloads, "Local Downloads", specialFolderList);
        AddSpecialFolder(FOLDERID_Music, "Music", specialFolderList);
        AddSpecialFolder(FOLDERID_LocalMusic, "Local Music", specialFolderList);
        AddSpecialFolder(FOLDERID_Pictures, "Pictures", specialFolderList);
        AddSpecialFolder(FOLDERID_LocalPictures, "Local Pictures", specialFolderList);
        AddSpecialFolder(FOLDERID_Videos, "Videos", specialFolderList);
        AddSpecialFolder(FOLDERID_LocalVideos, "Local Videos", specialFolderList);
    }
}
