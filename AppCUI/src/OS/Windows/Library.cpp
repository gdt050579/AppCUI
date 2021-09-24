#include "Internal.hpp"

using namespace AppCUI::OS;

Library::Library()
{
    this->libraryHandle = nullptr;
}
bool Library::Load(const std::filesystem::path & path)
{
    CHECK(this->libraryHandle == nullptr, false, "Library already opened !");
    this->libraryHandle = LoadLibraryW(path.native().c_str());
    CHECK(this->libraryHandle, false, "Fail to load library: %s", path.generic_string().c_str());
    return true;
}
void* Library::GetFunction(const char * functionName) const
{
    CHECK(this->libraryHandle, nullptr, "Library was not loaded --> have you call Load(...) first ?");
    CHECK(functionName, nullptr, "Expecting a valid (non-null) function name !");
    CHECK(*functionName, nullptr, "Expecting a valid (non-empty) function name !");
    // all good 
    void* fnPtr = GetProcAddress((HMODULE) this->libraryHandle, functionName);
    CHECK(fnPtr, nullptr, "Unable to find address of function: %s", functionName);
    return fnPtr;
}