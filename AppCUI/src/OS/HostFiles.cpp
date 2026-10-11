#include "Internal.hpp"

namespace AppCUI::OS
{
// Host file transfer belongs to the frontend (same policy as the clipboard): only a frontend whose file system is
// not the one of the user (BrowserTerminal) implements it.
static Internal::AbstractTerminal* GetHostFilesTerminal()
{
    auto app = Application::GetApplication();
    if ((app == nullptr) || (app->terminal == nullptr))
        return nullptr;
    return app->terminal.get();
}

bool IsHostFileTransferAvailable()
{
    auto terminal = GetHostFilesTerminal();
    return terminal ? terminal->HostFileTransferSupported() : false;
}

bool DownloadFile(const std::filesystem::path& file)
{
    auto terminal = GetHostFilesTerminal();
    CHECK(terminal != nullptr, false, "AppCUI is not initialized (there is no frontend to download a file) !");
    return terminal->DownloadToHost(file);
}
} // namespace AppCUI::OS
