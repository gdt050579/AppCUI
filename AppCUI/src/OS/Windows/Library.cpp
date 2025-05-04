#include "Internal.hpp"
#include <system_error>

namespace AppCUI::OS
{
bool Library::Load(const std::filesystem::path& path, std::string& errorMessage)
{
    if (libraryHandle)
    {
        errorMessage = "Library already opened!";
        RETURNERROR(false, errorMessage.c_str());
    }
    libraryHandle = LoadLibraryW(path.native().c_str());
    if (!libraryHandle)
    {
        const auto code   = GetLastError();
        std::string sCode = std::to_string(code);
        errorMessage      = "(" + sCode + ") " + std::system_category().message(code);
        RETURNERROR(false, errorMessage.c_str());
    }

    return true;
}

void* Library::GetFunction(const char* name) const
{
    CHECK(libraryHandle, nullptr, "Library was not loaded --> have you call Load(...) first ?");
    CHECK(name, nullptr, "Expecting a valid (non-null) function name !");
    CHECK(*name, nullptr, "Expecting a valid (non-empty) function name !");

    // all good
    void* fnPtr = GetProcAddress((HMODULE) libraryHandle, name);
    CHECK(fnPtr, nullptr, "Unable to find address of function: %s", name);
    return fnPtr;
}
} // namespace AppCUI::OS
