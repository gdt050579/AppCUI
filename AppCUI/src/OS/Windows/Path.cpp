#include <os.h>

using namespace AppCUI::OS::FileSystem;

bool Path::IsRootPath(const char *path)
{
    CHECK(path != nullptr, false, "");
    int count = 0;
    while ((*path) != 0)
    {
        if (((*path) == '\\') || ((*path) == '/'))
            count++;
        path++;
    }
    return (count <= 1);
}
bool Path::CopyDirectoryName(const char * path, AppCUI::Utils::String& result)
{
    unsigned int poz = -1;
    CHECK(path, false, "Expecting a valid (non-null) path !");
    for (unsigned int tr = 0; path[tr] != 0; tr++)
        if ((path[tr] == '\\') || (path[tr] == '/'))
            poz = tr;
    if (poz == -1)
    {
        CHECK(result.Set(""), false, "No folder present !");
    }
    else {
        CHECK(result.Set(path, poz + 1), false, "Fail to copy folder");
    }
    return true;
}
bool Path::Join(AppCUI::Utils::String& path, const char * name)
{
    if (!path.EndsWith("\\"))
    {
        CHECK(path.AddChar('\\'), false, "");
    }
    CHECK(path.Add(name), false, "");
    return true;
}