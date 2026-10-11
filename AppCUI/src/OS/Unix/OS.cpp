#include "../../Internal.hpp"
#include "PlatformFolders.hpp"
#include "Whereami.hpp"
#include <sys/mount.h>

namespace AppCUI::OS
{
void GetSpecialFolders([[maybe_unused]] SpecialFolderMap& specialFolders, RootsVector& roots)
{
#ifdef __EMSCRIPTEN__
    // The page has its own in-memory file system: the XDG folders (Desktop, Music, Videos, ...) do not exist there and
    // the executable lives at "/" -> only list real locations. Files of the user arrive through FileDialog's Upload.
    const char* home = std::getenv("HOME");
    roots.push_back({ "Home", (home && home[0]) ? home : "/home/web_user" });
    roots.push_back({ "Temp", "/tmp" });
    roots.push_back({ "/ (root)", "/" });
#else
    const auto desktop = sago::getDesktopFolder();
    if (!desktop.empty())
    {
        specialFolders[SpecialFolder::Desktop] = { "Desktop", desktop };
    }

    specialFolders[SpecialFolder::AppPath] = { "App Folder", GetCurrentApplicationPath().parent_path() };

    const auto documents = sago::getDocumentsFolder();
    if (!documents.empty())
    {
        specialFolders[SpecialFolder::Documents]      = { "Documents", documents };
        specialFolders[SpecialFolder::LocalDocuments] = { "Local Documents", documents };
    }

    auto downloadFolder = sago::getDownloadFolder();
    if (downloadFolder.empty())
    {
        downloadFolder = sago::getDownloadFolder1();
    }
    if (!downloadFolder.empty())
    {
        specialFolders[SpecialFolder::Downloads]      = { "Downloads", downloadFolder };
        specialFolders[SpecialFolder::LocalDownloads] = { "Local Downloads", downloadFolder };
    }

    const auto music = sago::getMusicFolder();
    if (!music.empty())
    {
        specialFolders[SpecialFolder::Music]      = { "Music", music };
        specialFolders[SpecialFolder::LocalMusic] = { "Local Music", music };
    }

    const auto pictures = sago::getPicturesFolder();
    if (!pictures.empty())
    {
        specialFolders[SpecialFolder::Pictures]      = { "Pictures", pictures };
        specialFolders[SpecialFolder::LocalPictures] = { "Local Pictures", pictures };
    }

    const auto video = sago::getVideoFolder();
    if (!video.empty())
    {
        specialFolders[SpecialFolder::Videos]      = { "Videos", video };
        specialFolders[SpecialFolder::LocalVideos] = { "Local Videos", video };
    }

    roots.push_back({ "/ (root)", "/" });
#endif
}

static std::filesystem::path current_application_path;

std::filesystem::path GetCurrentApplicationPath()
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
}