#include "Internal.hpp"

using namespace AppCUI::OS;



void AppCUI::OS::GetSpecialFolders(std::vector<std::pair<std::string, std::filesystem::path>>& specialFolderLists)
{
    specialFolderLists.clear();
    char drivePath[4] = "_:\\";
    std::string name;
    for (char drv = 'A'; drv <='Z';drv++)
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
        specialFolderLists.push_back({ name, drivePath });
    }
}
