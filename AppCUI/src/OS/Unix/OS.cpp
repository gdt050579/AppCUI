#include "Internal.hpp"
#include "PlatformFolders.hpp"
#include "Whereami.hpp"
#include <sys/mount.h>

using namespace AppCUI::OS;

void AppCUI::OS::GetSpecialFolders(SpecialFolderMap& specialFolders, RootsVector& roots)
{
    const auto desktop = sago::getDesktopFolder();
    if (!desktop.empty())
    {
        specialFolders[SpecialFolder::Desktop] = { "Desktop", desktop };
    }

    const auto documents = sago::getDocumentsFolder();
    if (!desktop.empty())
    {
        specialFolders[SpecialFolder::Documents] = { "Documents", documents };
    }

    auto downloadFolder = sago::getDownloadFolder();
    if (downloadFolder.empty())
    {
        downloadFolder = sago::getDownloadFolder1();
    }
    if (!downloadFolder.empty())
    {
        specialFolders[SpecialFolder::Downloads] = { "Downloads", downloadFolder };
    }

    const auto music = sago::getMusicFolder();
    if (!music.empty())
    {
        specialFolders[SpecialFolder::Music] = { "Music", music };
    }

    const auto pictures = sago::getPicturesFolder();
    if (!pictures.empty())
    {
        specialFolders[SpecialFolder::Pictures] = { "Pictures", pictures };
    }

    const auto video = sago::getVideoFolder();
    if (!video.empty())
    {
        specialFolders[SpecialFolder::Videos] = { "Video", video };
    }

    specialFolders[SpecialFolder::AppPath] = { "App Folder", GetCurrentApplicationPath().parent_path() };

    roots.push_back({ "/ (root)", "/" });
}

static std::filesystem::path current_application_path;

std::filesystem::path AppCUI::OS::GetCurrentApplicationPath()
{
    if (!current_application_path.empty())
    {
        return current_application_path;
    }

    const int actual_length = wai_getExecutablePath(nullptr, 0, nullptr);
    char* output_buffer     = new char[actual_length + 1]();
    if (!output_buffer)
    {
        return std::filesystem::path();
    }

    wai_getExecutablePath(output_buffer, actual_length, nullptr);
    output_buffer[actual_length] = 0;

    current_application_path = output_buffer;
    delete[] output_buffer;

    return current_application_path;
}
