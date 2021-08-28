#include "Internal.hpp"

using namespace AppCUI::OS;



void AppCUI::OS::GetSpecialFolders(std::vector<std::pair<std::string, std::filesystem::path>>& specialFolderLists, SpecialFoldersType type, bool clearVector)
{
	if (clearVector)
		specialFolderLists.clear();
	// to be implemented
}
