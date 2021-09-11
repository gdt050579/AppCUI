#include "Internal.hpp"

using namespace AppCUI::OS;



void AppCUI::OS::GetSpecialFolders(std::vector<std::pair<std::string, std::filesystem::path>>& specialFolderLists, SpecialFoldersType, bool clearVector)
{
	if (clearVector)
		specialFolderLists.clear();
	// to be implemented
}
std::filesystem::path AppCUI::OS::GetCurrentApplicationPath()
{
	return std::filesystem::path();
}
