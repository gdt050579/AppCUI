#include "Internal.hpp"
#include "Whereami.hpp"
using namespace AppCUI::OS;

void AppCUI::OS::GetSpecialFolders(
      std::vector<std::pair<std::string, std::filesystem::path>>& specialFolderLists,
      SpecialFoldersType,
      bool clearVector)
{
    if (clearVector)
        specialFolderLists.clear();
    // to be implemented
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
