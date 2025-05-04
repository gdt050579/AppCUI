#include "Internal.hpp"
#include <dlfcn.h>

namespace AppCUI::OS
{
bool Library::Load(const std::filesystem::path& path, std::string& errorMessage)
{
    if (libraryHandle)
    {
        errorMessage = "Library already opened!";
        RETURNERROR(false, errorMessage.c_str());
    }
    libraryHandle = dlopen(path.native().c_str(), RTLD_LAZY);
    if (!libraryHandle)
    {
        const auto error = dlerror();
        errorMessage     = "Fail to load library: " + path.generic_string() + " " + "[dlerror=" + error + "]!";
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
    void* fnPtr       = dlsym(libraryHandle, name);
    const char* error = dlerror();
    CHECK(!error && fnPtr, nullptr, "Unable to find address of function: %s [dlerror=%s]", name, error);
    return fnPtr;
}
} // namespace AppCUI::OS
